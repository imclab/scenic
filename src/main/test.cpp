// headerGPL.c
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

/** \file
 *      Just the License GPL 3+
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */


#include <glib.h>
#include <iostream>
#include <unistd.h>
#include "gutil/baseModule.h"
#include "logWriter.h"
#include "gutil/optionArgs.h"

#include "gst/videoSender.h"
#include "gst/videoConfig.h"


class DModule
    : public BaseModule
{
    bool pass;
   public:
    bool run();
};


bool DModule::run()
{
    if(!pass)
        while(1)
        {
            usleep(100000); LOG("x ");
        }
    return true;
}


int main (int argc, char** argv)
{
    DModule m;

    m.run();
    return 0;
}


