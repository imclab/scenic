// audioSource.cpp
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

#include <iostream>
#include <string>
#include <cassert>
#include "audioSource.h"
#include "audioConfig.h"

AudioSource *AudioSource::create(const AudioConfig &config)
{
    std::string source(config.source());

    if (!source.compare("audiotestsrc"))
        return new AudioTestSource(config);
    else if (!source.compare("filesrc"))
        return new AudioFileSource(config);
    else if (!source.compare("alsasrc"))
        return new AudioAlsaSource(config);
    else if (!source.compare("jackaudiosrc"))
        return new AudioJackSource(config);
    else
    {
        std::cerr << "Invalid source!" << std::endl;
        return 0;
    }
}

AudioSource::AudioSource(const AudioConfig &config) : config_(config), interleave_(config)
{
}

// parts of init that are common to all AudioSource classes
void AudioSource::init()
{
    interleave_.init();

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources_.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources_[channelIdx]);
        aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs_[channelIdx]);
        queues_.push_back(gst_element_factory_make("queue", NULL));
        assert(queues_[channelIdx]);

    }

    pipeline_.add_vector(sources_);
    pipeline_.add_vector(aconvs_);
    pipeline_.add_vector(queues_);
}

AudioSource::~AudioSource()
{
    assert(pipeline_.stop());
    pipeline_.remove_vector(queues_);
    pipeline_.remove_vector(aconvs_);
    pipeline_.remove_vector(sources_);
}

void AudioSource::linkElements()
{
    GstIter src, aconv;
    for (src = sources_.begin(), aconv = aconvs_.begin(); src != sources_.end(); ++src, ++aconv)
    {
        assert(gst_element_link(*src, *aconv));
        interleave_.linkInput(*aconv);
    }
}

void AudioTestSource::init()
{
    AudioSource::init();
    GstIter src, aconv;

    const double GAIN = 1.0 / config_.numChannels();        // so sum of tones' amplitude equals 1.0
    double frequency = 100.0;

    for (src = sources_.begin(); src != sources_.end(); ++src, frequency += 100.0)
        g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", frequency, "is-live", TRUE, NULL);

    linkElements();
}

void AudioFileSource::init()
{
    AudioSource::init();

    GstIter src, aconv, dec;

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        decoders_.push_back(gst_element_factory_make("wavparse", NULL));
        assert(decoders_[channelIdx]);
    }

    pipeline_.add_vector(decoders_);

    // FIXME: location should be changeable somewhere else
    int counter = 1;
    for (src = sources_.begin(); src != sources_.end(); ++src)
    {
        char filename[15];
        sprintf(filename, "audiofile%d.pcm", counter++);
        g_object_set(G_OBJECT(*src), "location", filename, NULL);
    }

    for (dec = decoders_.begin(), aconv = aconvs_.begin(); dec != decoders_.end(); ++dec, ++aconv)
        g_signal_connect(*dec, "pad-added", G_CALLBACK(Pipeline::cb_new_src_pad), (void *) *aconv);

    linkElements(); // overloaded
}

void AudioFileSource::linkElements()
{
    GstIter src, aconv, dec;
    for (src = sources_.begin(), dec = decoders_.begin(); src != sources_.end(); ++src, ++dec)
        assert(gst_element_link(*src, *dec));

    for (aconv = aconvs_.begin(), aconv = aconvs_.begin(); aconv != aconvs_.end(); ++aconv)
        interleave_.linkInput(*aconv);
}


AudioFileSource::~AudioFileSource()
{
    assert(pipeline_.stop());
    pipeline_.remove_vector(decoders_);
}

void AudioAlsaSource::init()
{
    AudioSource::init();
    linkElements();
}

void AudioJackSource::init()
{
    AudioSource::init();

    // turn off autoconnect to avoid Jack-killing input-output feedback loop, i.e.
    // jackOut -> jackIn -> jackOut ->jackIn.....
    for (GstIter src = sources_.begin(); src != sources_.end(); ++src)
        g_object_set(G_OBJECT(*src), "connect", 0, NULL);

    linkElements();
}

