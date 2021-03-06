/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "audio_factory.h"
#include <boost/program_options.hpp>
#include "audio_sender.h"
#include "audio_receiver.h"
#include "local_audio.h"
#include "codec.h"
#include "audio_config.h"
#include "remote_config.h"
#include "caps/caps_server.h"
#include "util/log_writer.h"
#include "pipeline.h"

using std::tr1::shared_ptr;
namespace po = boost::program_options;

void audiofactory::printMaxChannels(const std::string &codec)
{
    LOG_PRINT(codec << " supports up to " <<
            Encoder::maxChannels(codec) << " channels\n");
}

shared_ptr<AudioSender> audiofactory::buildAudioSender(Pipeline &pipeline, const po::variables_map &options)
{
    using std::string;
    shared_ptr<AudioSourceConfig> aConfig(new AudioSourceConfig(options));

    shared_ptr<SenderConfig> rConfig(new SenderConfig(pipeline,
                options["audiocodec"].as<string>(),
                options["address"].as<string>(),
                options["audioport"].as<int>(),
                options["multicast-interface"].as<string>()));

    shared_ptr<AudioSender> tx(new AudioSender(pipeline, aConfig, rConfig));

    return tx;
}

shared_ptr<AudioReceiver> audiofactory::buildAudioReceiver(Pipeline &pipeline, const po::variables_map &options)
{
    using std::string;
    shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(options));

    shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(options["audiocodec"].as<string>(),
                options["address"].as<string>(),
                options["audioport"].as<int>(),
                options["multicast-interface"].as<string>(),
                options["jitterbuffer"].as<int>()));

    return shared_ptr<AudioReceiver>(new AudioReceiver(pipeline, aConfig, rConfig));
}

shared_ptr<LocalAudio> audiofactory::buildLocalAudio(Pipeline &pipeline, const po::variables_map &options)
{
    AudioSourceConfig sourceConfig(options);
    return shared_ptr<LocalAudio>(new LocalAudio(pipeline, sourceConfig));
}
