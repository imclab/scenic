
// rtpSession.cpp
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

#include "gstLinkable.h"
#include "rtpSender.h"
#include "mediaConfig.h"
//#include "logWriter.h"


RtpSender::RtpSender()
    : rtp_sender_(0)
{}

RtpSender::~RtpSender()
{
    assert(stop());
    pipeline_.remove(rtp_sender_);
}


std::string RtpSender::getCaps() const
{
    assert(pipeline_.isPlaying());

    GstPad *pad;
    GstCaps *caps;

    assert(pad = gst_element_get_pad(GST_ELEMENT(rtp_sender_), "sink"));

    do
        caps = gst_pad_get_negotiated_caps(pad);
    while (caps == NULL);

    // goes until caps are initialized

    gst_object_unref(pad);

    std::string result(gst_caps_to_string(caps));
    gst_caps_unref(caps);
    return result;
}


void RtpSender::addDerived(GstElement * newSrc, const MediaConfig * config)
{
    GstPad *send_rtp_sink;
    GstPad *send_rtp_src;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *payloadSrc;
    GstPad *rtpSenderSink;
    GstPad *rtcpSenderSink;
    GstPad *rtcpReceiverSrc;

    assert(rtp_sender_ = gst_element_factory_make("udpsink", NULL));
    g_object_set(rtp_sender_, "host", config->remoteHost(), "port", config->port(), "sync",
                 FALSE, "async", FALSE, NULL);
    pipeline_.add(rtp_sender_);

    assert(send_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_")));
    assert(send_rtp_src = gst_element_get_static_pad(rtpbin_, padStr("send_rtp_src_")));

    assert(send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_")));

    assert(recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_")));

    assert(payloadSrc = gst_element_get_static_pad(newSrc, "src"));
    assert(rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink"));
    assert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));
    assert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));

    // link pads
    assert(GstLinkable::link_pads(payloadSrc, send_rtp_sink));
    assert(GstLinkable::link_pads(send_rtp_src, rtpSenderSink));
    assert(GstLinkable::link_pads(send_rtcp_src, rtcpSenderSink));
    assert(GstLinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtpSenderSink));
    gst_object_unref(GST_OBJECT(payloadSrc));

    // release request and static pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_object_unref(GST_OBJECT(send_rtp_src));
    gst_element_release_request_pad(rtpbin_, send_rtp_sink);
}


