
// rtpSender.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _RTP_SENDER_H_
#define _RTP_SENDER_H_

#include "rtp_bin.h"

class SenderConfig;
class _GstElement;
class _GstPad;
class _GParamSpec;
class RtpPay;

class RtpSender
    : public RtpBin
{
    public:
        RtpSender(const Pipeline &pipeline) :
            RtpBin(pipeline),
            rtp_sender_(0),
            send_rtp_sink_(0),
            send_rtcp_src_(0),
            recv_rtcp_sink_(0)
            {}
        ~RtpSender();

        void add(RtpPay * payloader, const SenderConfig & config);

    private:
        _GstElement *rtp_sender_;
        _GstPad *send_rtp_sink_;
        _GstPad *send_rtcp_src_;
        _GstPad *recv_rtcp_sink_;
        static void sendCapsChanged(_GstPad *pad, _GParamSpec *pspec, RtpSender *context);
        static void onNewSSRC(_GstElement *rtpbin, unsigned int session, unsigned int ssrc, void *data);
        virtual void subParseSourceStats(_GstStructure *stats);
        void deltaPacketLoss(_GstStructure *stats);
};

#endif // _RTP_SENDER_H_

