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

#include "rtp_receiver.h"
#include <list>
#include <algorithm>
#include <cstring>
#include <gst/gst.h>

#include "gutil/gutil.h"
#include "gst_linkable.h"
#include "pipeline.h"
#include "rtp_pay.h"
#include "remote_config.h"

#include <gtk/gtk.h>

std::list<GstElement *> RtpReceiver::depayloaders_;

RtpReceiver::RtpReceiver(const Pipeline &pipeline, int latency) :
    RtpBin(pipeline),
    rtp_receiver_(0),
    depayloader_(0),
    recv_rtp_sink_(0),
    send_rtcp_src_(0),
    recv_rtcp_sink_(0),
    latency_(latency)
{
    // try to update the latency every two seconds until the pipeline
    // has successfully configured the new latency
    g_timeout_add(2000, static_cast<GSourceFunc>(updateLatencyCb), this);
}

RtpReceiver::~RtpReceiver()
{
    // find this->depayloader in the static list of depayloaders
    if (depayloader_) // in case destructor was called before depayloader was created
    {
        std::list<GstElement *>::iterator iter;
        iter = std::find(depayloaders_.begin(), depayloaders_.end(), depayloader_);

        // make sure we found it and remove it
        assert(iter != depayloaders_.end());
        depayloaders_.erase(iter);
    }

    // Now we can unref our request pads
    if (recv_rtp_sink_)
        gst_object_unref(recv_rtp_sink_);
    if (send_rtcp_src_)
        gst_object_unref(send_rtcp_src_);
    if (recv_rtcp_sink_)
        gst_object_unref(recv_rtcp_sink_);
}

gboolean RtpReceiver::updateLatencyCb(gpointer data)
{
    RtpReceiver *context = static_cast<RtpReceiver*>(data);
    if (not context->pipeline_.latencyUpdated() and context->latency_ != INIT_LATENCY)
    {
        context->setLatency(context->latency_);
        return TRUE; // call again
    }
    else
        return FALSE; // don't call again
}


void RtpReceiver::setLatency(int latency)
{
    assert(rtpbin_);
    if (latency < MIN_LATENCY or latency > MAX_LATENCY)
    {
        LOG_WARNING("Cannot set rtpbin latency to " << latency << ", must be in range "
                << MIN_LATENCY << " to " << MAX_LATENCY);
    }
    else
    {
        LOG_DEBUG("Setting latency to " << latency << " ms");
        g_object_set(G_OBJECT(rtpbin_), "latency", latency, NULL);
    }
}


void RtpReceiver::setCaps(const char *capsStr)
{
    GstCaps *caps;
    if (capsStr and strlen(capsStr) == 0)
        THROW_ERROR("Cannot set RTP receiver caps to empty string");
    else
        LOG_DEBUG("Got caps string " << capsStr);
    caps = gst_caps_from_string(capsStr);
    if (caps == 0)
        THROW_ERROR("Could not generate caps from caps string\n\"" << capsStr <<
                "\"\nThere is potentially a Gstreamer version mismatch between "
                "this host and the sender host");
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);

    gst_caps_unref(caps);
}


void RtpReceiver::onPadAdded(GstElement *  /*rtpbin*/, GstPad * srcPad, void * /*data*/)
{
    gchar *pad_name = gst_pad_get_name(srcPad);
    // don't look at the full name
    if (gst_pad_is_linked(srcPad))
        LOG_DEBUG("Pad is already linked");
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
        LOG_DEBUG("Pad is not a source");
    else if (!g_str_has_prefix(pad_name, "recv_rtp_src"))
        LOG_DEBUG("Wrong pad");
    else
    {
        /// we can't just use context->depayloader because this signal may have been called for the rtp pad
        /// of another rtpreceiver than context
        std::string srcMediaType(getMediaType(srcPad));
        GstPad *sinkPad = getMatchingDepayloaderSinkPad(srcMediaType);

        if (gst_pad_is_linked(sinkPad)) // only link once
        {
            LOG_DEBUG("depayloader's sink pad is already linked, unlinking from old src pad");
            GstPad *oldSrcPad = gst_pad_get_peer(sinkPad);
            gst_pad_unlink(oldSrcPad, sinkPad);
            gst_object_unref(oldSrcPad);
        }
        gstlinkable::link_pads(srcPad, sinkPad);    // link our udpsrc to the corresponding depayloader
        gst_object_unref(sinkPad);

        LOG_INFO("New " << srcMediaType << " stream connected");
    }
    g_free(pad_name);
}


void RtpReceiver::onSenderTimeout(GstElement *  /*rtpbin*/, guint session, guint /* ssrc */, gpointer /*data*/)
{
    LOG_DEBUG("Sender timeout for session " << session);
    //RtpReceiver *context = static_cast<RtpReceiver*>(data);
    //context->printStats_ = false;
}


std::string RtpReceiver::getMediaType(GstPad *pad)
{
    GstStructure *structure = gst_caps_get_structure(gst_pad_get_caps(pad), 0);
    const GValue *str = gst_structure_get_value(structure, "media");
    std::string result(g_value_get_string(str));

    if (result != "video" and result != "audio")
        THROW_ERROR("Media type of depayloader sink pad is neither audio nor video!");

    return result;
}


GstPad *RtpReceiver::getMatchingDepayloaderSinkPad(const std::string &srcMediaType)
{
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(depayloaders_.front(), "sink");

    // match depayloader to rtp pad by media type
    // FIXME: what if we have two video depayloaders? two audio depayloaders?

    std::list<GstElement *>::iterator iter = depayloaders_.begin();

    while (getMediaType(sinkPad) != srcMediaType
            and iter != depayloaders_.end())
    {
        gst_object_unref(sinkPad);
        sinkPad = gst_element_get_static_pad(*iter, "sink");
        ++iter;
    }

    return sinkPad;
}


void RtpReceiver::add(RtpPay * depayloader, const ReceiverConfig & config)
{
    registerSession(config.identifier());

    // KEEP THIS LOW OR SUFFER THE CONSEQUENCES
    // rule of thumb: 2-3 times the maximum network jitter
    setLatency(INIT_LATENCY);

    GstPad *rtpReceiverSrc;
    GstPad *rtcpReceiverSrc;
    GstPad *rtcpSenderSink;

    // store copy so that destructor knows which depayloader to remove from its list
    depayloader_ = depayloader->sinkElement();
    // add to our list of active depayloaders
    depayloaders_.push_back(depayloader_);

    rtp_receiver_ = pipeline_.makeElement("udpsrc", NULL);
    int rtpsrc_socket = RtpBin::createSourceSocket(config.port());
    g_object_set(rtp_receiver_, "sockfd", rtpsrc_socket, "port", config.port(), NULL);

    // this is a multicast session
    if (config.hasMulticastInterface())
    {
        g_object_set(rtp_receiver_, "multicast-group", config.remoteHost(),
                "multicast-iface", config.multicastInterface(), NULL);
        LOG_DEBUG("Using IFACE for multicast" << config.multicastInterface());
    }

    rtcp_receiver_ = pipeline_.makeElement("udpsrc", NULL);
    int rtcpsrc_socket = RtpBin::createSourceSocket(config.rtcpFirstPort());
    g_object_set(rtcp_receiver_, "sockfd", rtcpsrc_socket, "port", config.rtcpFirstPort(), NULL);

    rtcp_sender_ = pipeline_.makeElement("udpsink", NULL);
    int rtcpsink_socket = RtpBin::createSinkSocket(config.remoteHost(), config.rtcpSecondPort());
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "sockfd", rtcpsink_socket, "port",
            config.rtcpSecondPort(), "sync", FALSE, "async", FALSE, NULL);

    /* now link all to the rtpbin, start by getting an RTP sinkpad for session n */
    rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver_, "src");
    assert(rtpReceiverSrc);
    recv_rtp_sink_ = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_"));
    assert(recv_rtp_sink_);
    bool linked = gstlinkable::link_pads(rtpReceiverSrc, recv_rtp_sink_);
    assert(linked);
    gst_object_unref(rtpReceiverSrc);

    /* get an RTCP sinkpad in session n */
    rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
    assert(rtcpReceiverSrc);
    recv_rtcp_sink_ = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink_);
    linked = gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink_);
    assert(linked);
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));

    /* get an RTCP srcpad for sending RTCP back to the sender */
    send_rtcp_src_ = gst_element_get_request_pad (rtpbin_, padStr("send_rtcp_src_"));
    assert(send_rtcp_src_);
    rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);
    linked = gstlinkable::link_pads(send_rtcp_src_, rtcpSenderSink);
    assert(linked);
    gst_object_unref(rtcpSenderSink);

    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added",
            G_CALLBACK(RtpReceiver::onPadAdded),
            NULL);

    g_signal_connect(rtpbin_, "on-sender-timeout",
            G_CALLBACK(RtpReceiver::onSenderTimeout),
            this);
}


void RtpReceiver::subParseSourceStats(GstStructure *stats)
{

    const GValue *val = gst_structure_get_value(stats, "internal");
    if (g_value_get_boolean(val))   // is-internal
        return;

    printStatsVal(sessionName_, "octets-received", "guint64", ":OCTETS-RECEIVED:", stats);
    printStatsVal(sessionName_, "packets-received", "guint64", ":PACKETS-RECEIVED:", stats);
    printStatsVal(sessionName_, "bitrate", "guint64", ":BITRATE:", stats);
}

