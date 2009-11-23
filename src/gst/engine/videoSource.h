// videoSource.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _VIDEO_SOURCE_H_
#define _VIDEO_SOURCE_H_

#include "gstLinkable.h"

#include "noncopyable.h"

#include <gst/gstclock.h>

class VideoSourceConfig;
class _GstElement;
class _GstPad;

class VideoSource
    : public GstLinkableSource, boost::noncopyable
{
    public:
        ~VideoSource();
        virtual void init();
        virtual std::string srcCaps() const;
        void setCapsFilter(const std::string &srcCaps);

    protected:
        explicit VideoSource(const VideoSourceConfig &config);
        const VideoSourceConfig &config_;
        _GstElement *source_;
        _GstElement *capsFilter_;
        std::string defaultSrcCaps() const;

    private:
        _GstElement *srcElement() { return source_; }
};

class VideoTestSource
    : public VideoSource
{
    public:
        explicit VideoTestSource(const VideoSourceConfig &config);
        void filterCaps();

    private:
        ~VideoTestSource();
        _GstElement *srcElement() { return capsFilter_; }
        void init();
};

class VideoFileSource
    : public VideoSource
{
    public:
        explicit VideoFileSource(const VideoSourceConfig &config);

    private:
        ~VideoFileSource();
        _GstElement *srcElement() { return identity_; }      
        void init();

        // FIXME: maybe just use the queue we acquire?
        _GstElement *identity_;
};

class VideoDvSource
    : public VideoSource
{
    public:
        explicit VideoDvSource(const VideoSourceConfig &config);

    private:
        ~VideoDvSource();
        
        _GstElement *srcElement() { return dvdec_; }
        void init();

        _GstElement *queue_, *dvdec_;
};

class VideoV4lSource
    : public VideoSource
{
    public:
        explicit VideoV4lSource(const VideoSourceConfig &config)
            : VideoSource(config), expectedStandard_("NTSC") {}
    private:
        void init();
        std::string expectedStandard_;
        std::string deviceStr() const;
        std::string srcCaps() const;
        _GstElement *srcElement() { return capsFilter_; }
};



class VideoDc1394Source
    : public VideoSource
{
    public:
        explicit VideoDc1394Source(const VideoSourceConfig &config)
            : VideoSource(config) {}
    private:
        enum {DMA_BUFFER_SIZE_IN_FRAMES = 2};
        void init();
        std::string srcCaps() const;
        _GstElement *srcElement() { return capsFilter_; }
};

#endif //_VIDEO_SOURCE_H_
