// audioSource.h
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

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include <cassert>
#include "gstLinkable.h"
#include "interleave.h"

#include "audioDelaySource.h"

class AudioConfig;

class AudioSource
    : public GstLinkable
{
    public:
        virtual ~AudioSource();
        void init();

    protected:
        explicit AudioSource(const AudioConfig &config);
        virtual void init_source();
        virtual void sub_init() = 0;
        virtual void link_elements();
        virtual void link_interleave();

        const AudioConfig &config_;

        Interleave interleave_;
        
        std::vector<GstElement *>sources_, aconvs_;
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                      gpointer user_data);

        virtual gboolean callback() { return FALSE; }
        GstElement *element() { return interleave_.element(); }

    private:
        friend class AudioSender;
};

class AudioTestSource
    : public AudioSource
{
    public:
        explicit AudioTestSource(const AudioConfig &config)
            : AudioSource(config), clockId_(0), offset_(0) {}
        ~AudioTestSource();
        void sub_init();

    protected:
        gboolean callback();

    private:
        void toggle_frequency();

        GstClockID clockId_;
        int offset_;

        AudioTestSource(const AudioTestSource&);     //No Copy Constructor
        AudioTestSource& operator=(const AudioTestSource&);     //No Assignment Operator
};

class AudioFileSource
    : public AudioSource
{
    public:
        explicit AudioFileSource(const AudioConfig &config)
            : AudioSource(config), decoders_() {}
        ~AudioFileSource();
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last,
                                   void *data);

    protected:
        void sub_init();
        void link_elements();

        void link_interleave(){};        // FIXME: AudioFileSource shouldn't even have an
        // interleave, unless we support the option of
        void init_source();              // multiple mono files combined into one stream

        GstElement *element() { return aconvs_[0]; }

        std::vector<GstElement*> decoders_;
};

class AudioAlsaSource
    : public AudioSource
{
    public:
        explicit AudioAlsaSource(const AudioConfig &config)
            : AudioSource(config) {}
        void sub_init(){};
};

class AudioJackSource
    : public AudioSource
{
    public:
        explicit AudioJackSource(const AudioConfig &config)
            : AudioSource(config) {}
        void sub_init();
};


class AudioDvSource
    : public AudioSource 
{
    public:
        explicit AudioDvSource(const AudioConfig &config);
        void sub_init();
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data);

    protected:
        void link_elements();
        void link_interleave(); // FIXME: AudioFileSource shouldn't even have an

        GstElement *demux_, *queue_;
        AudioDvSource(const AudioDvSource&);     //No Copy Constructor
        AudioDvSource& operator=(const AudioDvSource&);     //No Assignment Operator
};

#endif //_AUDIO_SOURCE_H_


