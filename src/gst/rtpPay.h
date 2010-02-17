// rtpPay.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _PAY_H_
#define _PAY_H_

#include "gstLinkable.h"
#include "messageHandler.h"

#include "noncopyable.h"

class Pipeline;
class _GstElement;

class RtpPay : public GstLinkableFilter, boost::noncopyable
{
    public:
        explicit RtpPay(Pipeline &pipeline) : pipeline_(pipeline), rtpPay_(0) {}
        virtual ~RtpPay();
        _GstElement *srcElement() { return rtpPay_; }
        _GstElement *sinkElement() { return rtpPay_; }

    protected:
        Pipeline &pipeline_;
        _GstElement *rtpPay_;
};

class Pay : public RtpPay
{
    public:
        virtual ~Pay();

    protected:
        Pay(Pipeline &pipeline);
        static const long long MAX_PTIME = 2000000LL;

    private:
        // Use max-ptime to limit the amount of 
        // encoded media packets in an RTP packet. Reduces latency
        void setMTU(unsigned long long mtu);
        // hardcoded in gst-plugins-base/gst-libs/gst/rtp/gstbasertppayload.c
        static const unsigned long long INIT_MTU = 1400;    
        static const unsigned long long MIN_MTU = 28;    
        static const unsigned long long MAX_MTU = 14000;
};


class Depay : public RtpPay
{
    protected:
        Depay(Pipeline &pipeline);
};


class TheoraPay : public Pay
{
    public:
        explicit TheoraPay(Pipeline &pipeline);
};


class TheoraDepay : public Depay
{
    public:
        explicit TheoraDepay(Pipeline &pipeline);
};


class H264Pay : public Pay
{
    public:
        explicit H264Pay(Pipeline &pipeline);
};


class H264Depay : public Depay
{
    public:
        explicit H264Depay(Pipeline &pipeline);
};


class H263Pay : public Pay
{
    public:
        explicit H263Pay(Pipeline &pipeline);
};


class H263Depay : public Depay
{
    public:
        explicit H263Depay(Pipeline &pipeline);
};


class Mpeg4Pay : public Pay, public MessageHandler
{
    private: 
        bool handleMessage(const std::string &path, const std::string &arguments); 
    public:
        explicit Mpeg4Pay(Pipeline &pipeline);
};


class Mpeg4Depay : public Depay
{
    public:
        explicit Mpeg4Depay(Pipeline &pipeline);
};


class VorbisPay : public Pay
{
    public:
        explicit VorbisPay(Pipeline &pipeline);
};


class VorbisDepay : public Depay 
{
    public:
        explicit VorbisDepay(Pipeline &pipeline);
};


class L16Pay : public Pay
{
    public:
        explicit L16Pay(Pipeline &pipeline);
};

class L16Depay : public Depay 
{
    public:
        explicit L16Depay(Pipeline &pipeline);
};


class MpaPay : public Pay
{
    public:
       explicit MpaPay(Pipeline &pipeline);
};

class MpaDepay : public Depay
{
    public:
        explicit MpaDepay(Pipeline &pipeline);
};

#endif //_PAY_H_
