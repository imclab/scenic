
// pipeline.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include <gst/gst.h>

#include <cassert>
#include <unistd.h>
#include "pipeline.h"
//#include "logWriter.h"

Pipeline *Pipeline::instance_ = 0;

Pipeline::Pipeline()
    : pipeline_(0), startTime_(0), verbose_(false)
{
    // empty
}


Pipeline & Pipeline::Instance()
{
    if (instance_ == 0) {
        instance_ = new Pipeline();
        instance_->init();
    }
    return *instance_;
}


Pipeline::~Pipeline()
{
    assert(stop());
    gst_object_unref(GST_OBJECT(pipeline_));
}


void Pipeline::init()
{
    if (!pipeline_)
    {
        gst_init(0, NULL);
        assert(pipeline_ = gst_pipeline_new("pipeline"));

        if (verbose_)
            make_verbose();
        // this will be used as a reference for future
        // pipeline synchronization
        startTime_ = gst_clock_get_time(clock());
    }
}


// FIXME: check if this is safe, we're destroying and recreating the pipeline
void Pipeline::reset()
{
    if (pipeline_)
    {
        assert(stop());
        delete instance_;
        instance_ = 0;
    }
}


void Pipeline::make_verbose()
{
    // Get verbose output
    if (verbose_) {
        gchar *exclude_args = NULL;     // set args to be excluded from output
        gchar **exclude_list = exclude_args ? g_strsplit(exclude_args, ",", 0) : NULL;
        g_signal_connect(pipeline_, "deep_notify",
                         G_CALLBACK(gst_object_default_deep_notify), exclude_list);
    }
}


bool Pipeline::isPlaying() const
{
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PLAYING))
        return true;
    else
        return false;
}


void Pipeline::wait_until_playing() const
{
    while (!isPlaying())
        usleep(10000);
}


void Pipeline::wait_until_stopped() const
{
    while (isPlaying())
        usleep(10000);
}


bool Pipeline::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    return isPlaying();
}


bool Pipeline::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}


void Pipeline::add(GstElement *element)
{
    gst_bin_add(GST_BIN(pipeline_), element);
}


void Pipeline::add(std::vector<GstElement*> &elementVec)
{
    std::vector< GstElement * >::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); ++iter)
        add(*iter);
}


void Pipeline::remove(GstElement *element)
{
    if (element)
        assert(gst_bin_remove(GST_BIN(pipeline_), element));
}


void Pipeline::remove(std::vector<GstElement*> &elementVec)
{
    std::vector<GstElement *>::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); ++iter)
        remove(*iter);
}


GstClockID Pipeline::add_clock_callback(GstClockCallback callback, gpointer user_data)
{
    GstClockID clockId = gst_clock_new_periodic_id(clock(), startTime_, GST_SECOND);
    gst_clock_id_wait_async(clockId, callback, user_data);
    return clockId;
}


void Pipeline::remove_clock_callback(GstClockID clockId)
{
    gst_clock_id_unschedule(clockId);
    gst_clock_id_unref(clockId);
}


