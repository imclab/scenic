/* audioReceiver.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include "pipeline.h"
#include "gstLinkable.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "rtpPay.h"
#include "codec.h"
#include "audioSink.h"

/** Constructor parameterized by an AudioSinkConfig 
 * and a ReceiverConfig */
AudioReceiver::AudioReceiver(const AudioSinkConfig aConfig, const ReceiverConfig rConfig) : 
    audioConfig_(aConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    gotCaps_(false),
    depayloader_(0), 
    decoder_(0), 
    level_(), 
    sink_(0)
{ 
    tassert(remoteConfig_.hasCodec()); 
    remoteConfig_.checkPorts();
}

/// Destructor 
AudioReceiver::~AudioReceiver()
{
    remoteConfig_.cleanupPorts();
    delete sink_;
    delete decoder_;
    delete depayloader_;
}


void AudioReceiver::init_codec()
{
    tassert(decoder_ = remoteConfig_.createAudioDecoder());
    decoder_->init();
}


void AudioReceiver::init_depayloader()
{
    tassert(depayloader_ = decoder_->createDepayloader());
    depayloader_->init();
    gstlinkable::link(*depayloader_, *decoder_);
    session_.add(depayloader_, remoteConfig_);

    //init_level();
}


void AudioReceiver::init_level()
{
    level_.init();
    gstlinkable::link(*decoder_, level_);
}


void AudioReceiver::init_sink()
{
    tassert(sink_ = audioConfig_.createSink());
    sink_->init();
    //gstlinkable::link(level_, *sink_);   
    gstlinkable::link(*decoder_, *sink_);   
    setCaps();
    tassert(gotCaps_);
    tassert(remoteConfig_.capsMatchCodec()); 
}

/// Used to set this AudioReceiver's RtpReceiver's caps 
void AudioReceiver::setCaps() 
{ 
    session_.setCaps(remoteConfig_.caps()); 
    gotCaps_ = true;
}


