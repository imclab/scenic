// dc1394.h
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

#ifndef _DC1394_H_
#define _DC1394_H_

#include <gst/gst.h>
#include <string>

class DC1394 
{
    public:
        static void listCameras();
        static int capsToMode(int cameraNumber, int width, 
                int height, 
                const std::string &colourspace,
                int framerate);
        static int GUIDToCameraNumber(unsigned long long GUID);
        static bool requiresMoreISOSpeed(int mode);
        static const int MAX_ISO_SPEED = 800;
        static bool areCamerasConnected();
};

#endif //_DC1394_H_
