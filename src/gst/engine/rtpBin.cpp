/* rtpBin.cpp
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

#include <gst/gst.h>
#include <cstring>
#include <netdb.h>
#include "rtpBin.h"
#include "rtpPay.h"
#include "remoteConfig.h"
#include "pipeline.h"
#include "playback.h"

// for posting 
#include "mapMsg.h"
#include <boost/lexical_cast.hpp>

#ifdef CONFIG_DEBUG_LOCAL
#define RTP_REPORTING 0
#endif

GstElement *RtpBin::rtpbin_ = 0;
int RtpBin::sessionCount_ = 0;
bool RtpBin::destroyed_ = false;


std::map<int, RtpBin*> RtpBin::sessions_;

RtpBin::RtpBin() : 
    rtcp_sender_(0), 
    rtcp_receiver_(0), 
    sessionId_((++sessionCount_) - 1), 
    sessionName_(), 
    printStats_(true)  // 0 based
{
    // only initialize rtpbin element once per process
    if (rtpbin_ == 0) 
    {
        rtpbin_ = Pipeline::Instance()->makeElement("gstrtpbin", NULL);

        // uncomment this to print stats
        g_timeout_add(REPORTING_PERIOD_MS /* ms */, 
                static_cast<GSourceFunc>(printStatsCallback),
                this);
    }
    // DON'T USE THE DROP-ON-LATENCY SETTING, WILL CAUSE AUDIO TO DROP OUT WITH LITTLE OR NO FANFARE
}


void RtpBin::printStatsVal(const std::string &idStr, 
        const char *key, 
        const std::string &type, 
        const std::string &formatStr, 
        GstStructure *stats)
{
    MapMsg mapMsg;
    std::string paramStr("");
    if (type == "guint64")
    {
        guint64 val = g_value_get_uint64(gst_structure_get_value(stats, key));
        paramStr += formatStr + boost::lexical_cast<std::string>(val);
    }
    else if (type == "guint32")
    {
        guint32 val = g_value_get_uint(gst_structure_get_value(stats, key));
        paramStr += formatStr + boost::lexical_cast<std::string>(val);
    }
    else if (type == "gint32")
    {
        gint32 val = g_value_get_int(gst_structure_get_value(stats, key));
        paramStr += formatStr + boost::lexical_cast<std::string>(val);
    }
    else
        THROW_ERROR("Unexpected type");

    mapMsg["stats"] = idStr + paramStr;
    LOG_INFO(mapMsg["stats"]);
}


void RtpBin::parseSourceStats(GObject * source, RtpBin *context)
{
    GstStructure *stats;

    // get the source stats
    g_object_get(source, "stats", &stats, NULL);

    /* simply dump the stats structure */
    // gchar *str = gst_structure_to_string (stats);
    // LOG_DEBUG("source stats: " << str);

    context->subParseSourceStats(stats);  // let our subclasses parse the stats

    // free structures
    gst_structure_free (stats);
    //g_free (str);
}


// callback to print the rtp stats 
gboolean RtpBin::printStatsCallback(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    if (destroyed_)
    {
        LOG_DEBUG("No active rtpsessions, unregistering reporting callback");
        return FALSE;
    }
    else if (!context->printStats_)
    {
        LOG_DEBUG("Finished printing stats");
        playback::quit();
        return FALSE;
    }
    else if (sessionCount_ <= 0) // no sessions to print yet
        return TRUE; 
    else if (not Pipeline::Instance()->isPlaying())
        return TRUE; // not playing yet


    GObject *session;
    GValueArray *arrayOfSources;
    GValue *val;
    guint i;

    // get sessions
    for (int sessionId = 0; sessionId < sessionCount_; ++sessionId)
    {
        g_signal_emit_by_name(rtpbin_, "get-internal-session", sessionId, &session);

        // parse stats of all the sources in the session, this include the internal source
        g_object_get(session, "sources", &arrayOfSources, NULL);

        for (i = 0; i < arrayOfSources->n_values; ++i)
        {
            GObject *source;

            val = g_value_array_get_nth(arrayOfSources, i);
            source = static_cast<GObject*>(g_value_get_object(val));

            parseSourceStats(source, sessions_[sessionId]);
        }
        g_value_array_free(arrayOfSources);

        g_object_unref(session);
    }
    return TRUE;
}



const char *RtpBin::padStr(const char *padName) const
{
    tassert(sessionCount_ > 0);  // we have a session going
    std::string result(padName);
    result = result + boost::lexical_cast<std::string>(sessionId_);
    return result.c_str();
}


RtpBin::~RtpBin()
{
    unregisterSession();
    Pipeline::Instance()->remove(&rtcp_sender_);    // a pair for each session
    Pipeline::Instance()->remove(&rtcp_receiver_);

    --sessionCount_;
    if (sessionCount_ == 0) // destroy if no streams are present
    {
        LOG_DEBUG("No rtp sessions left, destroying rtpbin");
        Pipeline::Instance()->remove(&rtpbin_); // one shared by all sessions
        rtpbin_ = 0;
        destroyed_ = true;
    }
    else if (sessionCount_ < 0)
        LOG_WARNING("Rtp session count is somehow less than zero!!!");
}


void RtpBin::registerSession(const std::string &codec)
{
    std::string sessionName("");
    sessionName += codec + "_" + boost::lexical_cast<std::string>(sessionId_);
    sessionName_ = sessionName;
    sessions_[sessionId_] = this;
}

void RtpBin::unregisterSession()
{
    // does NOT call this->destructor (and that's a good thing)
    sessions_.erase(sessionId_); // remove session name by id
}

int RtpBin::createSinkSocket(const char *hostname, int port)
{
    using boost::lexical_cast;
    using std::string;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    std::string portStr(lexical_cast<string>(port));
    LOG_DEBUG("Trying socket for host " << hostname << ", port " << portStr);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(hostname, portStr.c_str(), &hints, &servinfo)) != 0) 
        THROW_ERROR("getaddrinfo: " << gai_strerror(rv));

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) 
        {
            perror("socket error");
            continue;
        }
        else if (p->ai_family == AF_INET)
            LOG_DEBUG("IPV4 socket");
        else if (p->ai_family == AF_INET6)
            LOG_DEBUG("IPV4 socket");
        else 
            LOG_DEBUG("Unknown address family");

        break;
    }

    if (p == NULL) 
    {
        close(sockfd);
        THROW_ERROR("failed to create socket");
    }

    freeaddrinfo(servinfo);
    LOG_DEBUG("socket created successfully\n");
    return sockfd;
}


int RtpBin::createSourceSocket(int port)
{
    using std::string;
    using boost::lexical_cast;

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    //int reuse = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    string portStr(lexical_cast<string>(port));
    LOG_DEBUG("Trying socket for port " << portStr);

    if ((rv = getaddrinfo(NULL, portStr.c_str(), &hints, &servinfo)) != 0) 
        THROW_ERROR("getaddrinfo: " << gai_strerror(rv));

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) 
        {
            LOG_WARNING("socket error");
            continue;
        }
        else if (p->ai_family == AF_INET)
            LOG_DEBUG("IPV4 Socket");
        else if (p->ai_family == AF_INET6)
            LOG_DEBUG("IPV6 Socket");
        else 
            LOG_DEBUG("Unknown address family");
#if 0
        /// GST uses this, we don't necessarily want to enable reuse
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse,
                        sizeof (reuse))) < 0)
        {
            close(sockfd);
            LOG_WARNING("setsockopt failed");
            continue;
        }
#endif

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            LOG_WARNING("bind error");
            continue;
        }

        break;
    }

    if (p == NULL) 
    {
        close(sockfd);
        THROW_ERROR("Failed to bind socket");
    }

    freeaddrinfo(servinfo);
    LOG_DEBUG("socket created successfully");
    return sockfd;
}

