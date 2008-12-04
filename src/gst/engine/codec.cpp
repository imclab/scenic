// codec.cpp
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
#include <gst/audio/multichannel.h>
#include "codec.h"
#include "rtpPay.h"
#include "pipeline.h"
#include "logWriter.h"

/// Constructor 
Codec::Codec() : 
    codec_(0) 
{}


/// Destructor 
Codec::~Codec()
{
    Pipeline::Instance()->remove(&codec_);
}


/// Constructor 
AudioConvertedEncoder::AudioConvertedEncoder() : 
    aconv_(0) 
{}

void AudioConvertedEncoder::init()
{
    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);
}

/// Destructor 
AudioConvertedEncoder::~AudioConvertedEncoder()
{
    Pipeline::Instance()->remove(&aconv_);
}


/// Constructor 
AudioConvertedDecoder::AudioConvertedDecoder() : 
    aconv_(0) 
{}


void AudioConvertedDecoder::init()
{
    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);
}

/// Destructor 
AudioConvertedDecoder::~AudioConvertedDecoder()
{
    Pipeline::Instance()->remove(&aconv_);
}


/// Constructor 
H264Encoder::H264Encoder() : 
    colorspc_(0) 
{}


/// Destructor 
H264Encoder::~H264Encoder()
{
    Pipeline::Instance()->remove(&colorspc_);
}


void H264Encoder::init()
{
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    codec_ = Pipeline::Instance()->makeElement("x264enc", NULL);
    g_object_set(G_OBJECT(codec_), "bitrate", 4096, "byte-stream", TRUE, "threads", 4,
            NULL);

    gstlinkable::link(colorspc_, codec_);
}


/// Creates an h.264 rtp payloader 
RtpPay* H264Encoder::createPayloader() const
{
    return new H264Payloader();
}


void H264Decoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("ffdec_h264", NULL);
}


/// Creates an h.264 RtpDepayloader 
RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depayloader();
}

/// Constructor 
VorbisEncoder::VorbisEncoder() 
{}

void VorbisEncoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("vorbisenc", NULL);
    gstlinkable::link(aconv_, codec_);
}


/// Creates an RtpVorbisPayloader 
RtpPay* VorbisEncoder::createPayloader() const
{
    return new VorbisPayloader();
}


void VorbisDecoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("vorbisdec", NULL);
}

/// Creates an RtpVorbisDepayloader 
RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepayloader();
}

/// Constructor
RawEncoder::RawEncoder()
{}

/// Creates an RtpL16Payloader 
RtpPay* RawEncoder::createPayloader() const
{
    return new L16Payloader();
}

/// Constructor
RawDecoder::RawDecoder()
{}

/// Creates an RtpL16Depayloader 
RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depayloader();
}

/// Constructor
LameEncoder::LameEncoder()
{}

void LameEncoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("lame", NULL);
    gstlinkable::link(aconv_, codec_);
}


/// Constructor
MadDecoder::MadDecoder()
{}


void MadDecoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("mad", NULL);
    gstlinkable::link(codec_, aconv_);
}

/** 
* Creates an RtpMpaPayloader */
RtpPay* LameEncoder::createPayloader() const
{
    return new MpaPayloader();
}

/// Creates an RtpMpaDepayloader 
RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepayloader();
}

