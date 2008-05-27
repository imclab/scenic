
#include <cpptest.h>
#include <iostream>

#include "gstTestSuite.h"
#include "videoSender.h"
#include "audioSender.h"

void GstTestSuite::setup()
{
    gst_init(0, NULL);
    // empty
}



void GstTestSuite::tear_down()
{
    // empty
}



void GstTestSuite::init_test()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
}



void GstTestSuite::start_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
    char c;
    std::cin >> c;
}



void GstTestSuite::stop_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.stop();
    char c;
    std::cin >> c;
}



void GstTestSuite::start_and_stop_video()
{
    char c;
    VideoSender tx;

    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
    // block
    std::cout << "Enter any key: ";
    std::cin >> c;
    tx.stop();
}



void GstTestSuite::start_audio()
{
    char c;
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
    // block
    std::cout << "Enter any key: ";
    std::cin >> c;
}



void GstTestSuite::stop_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.stop();
}



void GstTestSuite::start_and_stop_audio()
{
    char c;
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
    // block
    std::cout << "Enter any key: ";
    std::cin >> c;
    tx.stop();
}



int main(int argc, char** argv)
{
    GstTestSuite tester;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

