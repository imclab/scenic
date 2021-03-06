/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstring>
#include <cmath>
#include <gst/gst.h>
#include "util/log_writer.h"
#include "gutil/gutil.h"
#include "audio_level.h"
#include "vumeter.h"
#include "pipeline.h"

/* Sets message interval to one second by default */
AudioLevel::AudioLevel(Pipeline &pipeline, int numChannels, GdkNativeWindow socketID, const std::string &title) :
    BusMsgHandler(&pipeline),
    level_(pipeline.makeElement("level", NULL)),
    hasDisplay_(gutil::has_display())
{
    if (hasDisplay_)
    {
        static const int SPACING = 1;
        static const int ROWS = 2;
        GtkWidget *table = gtk_table_new(ROWS, numChannels, FALSE/*homogenous spacing*/);
        gtk_table_set_col_spacings(GTK_TABLE(table), SPACING);

        for (int i = 0; i < numChannels; ++i)
        {
            vumeters_.push_back(gtk_vumeter_new());
            gtk_table_attach(GTK_TABLE(table), vumeters_[i], i, i + 1, 0, 1,
                    static_cast<GtkAttachOptions>(GTK_SHRINK | GTK_FILL),
                    static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), 0, 0);
            GtkWidget *label = gtk_label_new(0);
            char *markup;

            markup = g_markup_printf_escaped ("<span weight=\"bold\">%d</span>", i + 1);
            gtk_label_set_markup (GTK_LABEL (label), markup);
            g_free (markup);
            gtk_table_attach(GTK_TABLE(table), label, i, i + 1, 1, ROWS, GTK_SHRINK,
                    GTK_SHRINK, 0, 0);
        }

        GtkWidget *scrolled = gtk_scrolled_window_new(0, 0);
        g_object_set(scrolled, "vscrollbar-policy", GTK_POLICY_NEVER, NULL);
        g_object_set(scrolled, "hscrollbar-policy", GTK_POLICY_AUTOMATIC, NULL);
        GtkWidget *viewport = gtk_viewport_new(0, 0);
        g_object_set(viewport, "shadow-type", GTK_SHADOW_NONE, NULL);
        gtk_container_add(GTK_CONTAINER(viewport), table);
        gtk_container_add(GTK_CONTAINER(scrolled), viewport);

        // either make plug...
        if (socketID != 0)
        {
            GtkWidget *plug = gtk_plug_new(socketID);
            /* end main loop when plug is destroyed */
            g_signal_connect(G_OBJECT (plug), "destroy", G_CALLBACK(gutil::killMainLoop), NULL);
            gtk_container_add(GTK_CONTAINER (plug), scrolled);
            /* show window and log its id */
            gtk_widget_show_all(plug);
            LOG_DEBUG("Created plug with ID: " << static_cast<unsigned int>(gtk_plug_get_id(GTK_PLUG(plug))));
        }
        else // ...or make a window
        {
            GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(window), title.c_str());

            // set icon
            std::string iconPath(std::string(PIXMAPS_DIR) + "/scenic.png");
            if (gtk_window_set_icon_from_file(GTK_WINDOW(window), iconPath.c_str(), NULL))
                LOG_DEBUG("Using icon " << iconPath << " for window");
            else
                LOG_DEBUG(iconPath << " does not exist");

            gtk_window_set_default_size(GTK_WINDOW(window), 80, 480);
            g_signal_connect(G_OBJECT (scrolled), "destroy", G_CALLBACK(gutil::killMainLoop), NULL);
            LOG_DEBUG("Created window for vumeters");
            gtk_container_add(GTK_CONTAINER (window), scrolled);
            gtk_widget_show_all(window);
        }

        g_object_set(G_OBJECT(level_), "message", true, NULL);
        static const guint64 INTERVAL_NS = 75000000;
        static const gdouble PEAK_FALLOFF = 750.0;
        static const guint64 PEAK_TTL = 3 * 300000000;  // 3 times default
        g_object_set(G_OBJECT(level_), "interval", INTERVAL_NS, NULL);
        g_object_set(G_OBJECT(level_), "peak-falloff", PEAK_FALLOFF, NULL);
        g_object_set(G_OBJECT(level_), "peak-ttl", PEAK_TTL, NULL);
    }
}

void AudioLevel::setValue(gdouble peak, gdouble decayPeak, GtkWidget *vumeter)
{
    GdkRegion *region;
    if (!vumeter->window)
        return;

    GTK_VUMETER(vumeter)->peak = peak;
    GTK_VUMETER(vumeter)->decay_peak = decayPeak;

    region = gdk_drawable_get_clip_region (vumeter->window);
    gdk_window_invalidate_region (vumeter->window, region, TRUE);
    gdk_window_process_updates (vumeter->window, TRUE);

    gdk_region_destroy (region);
}

/**
 * The level message is posted on the bus by the level element,
 * received by this AudioLevel, and dispatched. */
bool AudioLevel::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);
    std::vector<double> rmsValues;
    const std::string levelStr = "level";

    if (std::string(name) == levelStr)
    {
        // this is level's msg
        if (not hasDisplay_)
            return true; // we don't process it if no display is present
        guint channels;
        double peak_db;
        double decay_db;
        const GValue *list;
        const GValue *value;

        // we can get the number of channels as the length of the value list
        list = gst_structure_get_value (s, "rms");
        channels = gst_value_list_get_size (list);

        for (size_t c = 0; c < channels; ++c)
        {
            list = gst_structure_get_value(s, "peak");
            value = gst_value_list_get_value(list, c);
            peak_db = g_value_get_double(value);
            list = gst_structure_get_value (s, "decay");
            value = gst_value_list_get_value(list, c);
            decay_db = g_value_get_double(value);
            setValue(peak_db, decay_db, vumeters_[c]);
        }

        return true;
    }

    return false;           // this wasn't our msg, someone else should handle it
}

