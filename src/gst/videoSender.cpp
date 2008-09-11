
// videoSender.cpp
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
#include "videoSender.h"
#include "videoSource.h"
#include "videoSink.h"
#include "videoConfig.h"
#include "codec.h"
#include "logWriter.h"


VideoSender::~VideoSender()
{
    assert(stop());
    delete sink_;
    pipeline_.remove(&payloader_);
    delete encoder_;
    delete source_;
}


void VideoSender::init_source()
{
    assert(source_ = config_.createSource());
    source_->init();
}


void VideoSender::init_codec()
{
    // TODO:
    if (config_.isNetworked()) {
        assert(encoder_ = config_.createEncoder());
        encoder_->init();
        GstLinkable::link(*source_, *encoder_);
    }
}


void VideoSender::init_sink()
{
    if (config_.isNetworked()) {
        // TODO:
        // assert(payloader_ = codec_.createPayloader());
        // payloader_->init();
        assert(payloader_ = gst_element_factory_make("rtph264pay", NULL));
        pipeline_.add(payloader_);
        GstLinkable::link(*encoder_, payloader_);
        session_.add(payloader_, config_);
    }
    else {                 // local test only, no encoding
        assert(sink_ = config_.createSink());
        sink_->init();
        GstLinkable::link(*source_, *sink_);   // FIXME: this shouldn't happen for VideoFileSource
    }
}


bool VideoSender::start()
{
    GstBase::start();
    pipeline_.wait_until_playing(); // otherwise it doesn't know it's playing
    if (!config_.isNetworked())
        sink_->showWindow();
    else
        LOG("Sending video to remote receiver", DEBUG);
    return true;
}


