
// audioSender.cpp
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
//
// TODO: Client code should just call this with some kind of parameter object which
// specifies number of channels, how to compress it (if at all), and host and port info.

#include <cassert>
#include <iostream>
#include "lo/lo.h"
#include <sstream>
#include "logWriter.h"

#include "audioSender.h"
#include "audioSource.h"
#include "pipeline.h"
#include "codec.h"
#include "rtpPay.h"

AudioSender::~AudioSender()
{
    assert(stop());
    delete payloader_;
    delete encoder_;
    delete source_;
}


void AudioSender::init_source()
{
    assert(source_ = audioConfig_.createSource());
    source_->init();
}


void AudioSender::init_codec()
{
        assert(encoder_ = remoteConfig_.createEncoder());
        encoder_->init();
}


void AudioSender::init_sink()   // FIXME: this should be init_payloader
{
     assert(payloader_ = encoder_->createPayloader());
     payloader_->init();
     GstLinkable::link(*source_, *encoder_);
     
     GstLinkable::link(*encoder_, *payloader_);
     session_.add(payloader_, remoteConfig_);   // FIXME: session should take RtpPay pointer
}



void AudioSender::send_caps() const
{
    //#ifdef USE_OSC
    // returns caps for last sink, needs to be sent to receiver for rtpvorbisdepay
    LOG("Sending caps...", DEBUG);

    lo_address t = lo_address_new(NULL, MediaBase::OSC_PORT);
    if (lo_send(t, "/audio/rx/caps", "s", session_.getCaps().c_str()) == -1)
        std::cerr << "OSC error " << lo_address_errno(t) << ": " << lo_address_errstr(t)
            << std::endl;
    //#endif
}


bool AudioSender::start()
{
    MediaBase::start();

    std::stringstream logstr;
    logstr << "Sending audio to host " << remoteConfig_.remoteHost() << " on port " << remoteConfig_.port() << std::endl;
    LOG_DEBUG(logstr.str());

    pipeline_.wait_until_playing();
    send_caps();

    return true;
}


