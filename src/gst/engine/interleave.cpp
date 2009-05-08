/* interleave.cpp
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

#include <cstring>      // for memset
#include <gst/audio/multichannel.h>
#include "interleave.h"
#include "pipeline.h"
#include "audioConfig.h"

Interleave::~Interleave()
{
    Pipeline::Instance()->remove(&interleave_);
}


void Interleave::set_channel_layout()
{
    GValue val;
    memset(&val, 0, sizeof(val));

    GValueArray *arr;           // for channel position layout
    arr = g_value_array_new(config_.numChannels());

    g_object_set(interleave_, "channel-positions-from-input", FALSE, NULL);

    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        g_value_set_enum(&val, VORBIS_CHANNEL_POSITIONS[config_.numChannels() -1][channelIdx]);
        g_value_array_append(arr, &val);
        g_value_reset(&val);
    }

    g_value_unset(&val);

    g_object_set(interleave_, "channel-positions", arr, NULL);
    g_value_array_free(arr);
}


void Interleave::init()
{
    interleave_ = Pipeline::Instance()->makeElement("interleave", NULL);
    set_channel_layout();
}


// courtesy of vorbisenc.c
const GstAudioChannelPosition Interleave::VORBIS_CHANNEL_POSITIONS[][8] = {
    {                           /* Mono */
        GST_AUDIO_CHANNEL_POSITION_FRONT_MONO
    },
    {                          /* Stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                          /* Stereo + Centre */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                          /* Quadraphonic */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT
    },
    {                          /* Stereo + Centre + rear stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT
        ,
    },
    {                          /* Full 5.1 Surround */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_LFE
    },
    {                          /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_REAR_CENTER
    },
    {                          /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT,
        GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT
    }
    ,
};

