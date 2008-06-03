
// videoSender.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <gst/gst.h>

#include "mediaBase.h"
#include "videoSender.h"

VideoSender::VideoSender() : MediaBase()
{
    // empty
}



VideoSender::~VideoSender() 
{
    // empty
}



bool VideoSender::init(const std::string media,const int port, const std::string addr) 
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    remoteHost_ = std::string(addr);

    //  Create sender pipeline
    //  TODO: should these be subclasses of VideoSender?
    if (!media.compare("dv"))
    {
            initDv();
            return true;
    }
    else if (!media.compare("dvRtp"))
    {
            initDvRtp();
            return true;
    }
    else if (!media.compare("v4l"))
    {
            initV4l();
            return true;
    }
    else if (!media.compare("v4lRtp"))
    {
            initV4l();
            return true;
    }
    else if (!media.compare("test"))
    {
            initTest();
            return true;
    }
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void VideoSender::initDv()
{
    GError* error = NULL;
    std::string launchStr = "dv1394src ! dvdemux name=demux demux. ! queue ! dvdec ! xvimagesink "
                            "sync=false demux. ! queue ! audioconvert ! alsasink sync=false";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
}



void VideoSender::initDvRtp()
{
    GError* error = NULL;
    std::string launchStr = "dv1394src ! dvdemux name=demux demux. ! queue ! dvdec ! ffmpegcolorspace !"
                            "x264enc bitrate=12000 byte-stream=true threads=4 ! rtph264pay ! "
                            "udpsink host="; 
                                  
    std::stringstream istream;
    istream << remoteHost_ << " port = " << port_;           
    launchStr += istream.str();     // get port number into launch string
    launchStr += " demux. ! queue ! fakesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
}



void VideoSender::initV4l()
{
    GError *error = NULL;
    std::string launchStr = "v4l2src ! ffmpegcolorspace ! xvimagesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
}



void VideoSender::initV4lRtp()
{
    GError *error = NULL;
    std::string launchStr = "v4l2src ! ffmpegcolorspace ! x264enc bitrate=12000 byte-stream=true" 
                            "threads=4 ! rtph264pay !  udpsink host="; 
    std::stringstream istream;
    istream << remoteHost_ << " port = " << port_;           
    launchStr += istream.str();     // get port number into launch string
    launchStr += " demux. ! queue ! fakesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
}



void VideoSender::initTest()
{
    GstElement *txSrc, *txSink, *txCsp, *x264enc, *rtph264pay;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    txSrc = gst_element_factory_make("videotestsrc", "txSrc");
    assert(txSrc);

    txCsp = gst_element_factory_make("ffmpegcolorspace", "txCsp");
    assert(txCsp);

    x264enc = gst_element_factory_make("x264enc", "x264enc");
    assert(x264enc);

    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    assert(rtph264pay);

    txSink = gst_element_factory_make("udpsink", "txSink");
    assert(txSink);

    g_object_set(G_OBJECT(x264enc),"bitrate", 1500, NULL);
    g_object_set(G_OBJECT(x264enc),"byte-stream", TRUE, NULL);
    g_object_set(G_OBJECT(x264enc),"threads", 4, NULL);
    
    g_object_set(G_OBJECT(txSink), "host", remoteHost_.c_str(), "port", port_, NULL);
    

    gst_bin_add_many(GST_BIN(pipeline_), txSrc, txCsp, x264enc, rtph264pay, 
                     txSink, NULL);
 
    // links testsrc, colorspace converter, encoder, payloader and udpsink
    gst_element_link_many(txSrc, txCsp, x264enc, rtph264pay, txSink, NULL);
}



bool VideoSender::start()
{
    std::cout << "Sending media on port " << port_ << " to host " << remoteHost_
        << std::endl;

    return MediaBase::start();
}
