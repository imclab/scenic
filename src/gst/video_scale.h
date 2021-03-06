
/* videoScale.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/** @file
 * The VideoScale component.
 * Wraps and capsfilters the GStreamer videoscale element.
 */

#ifndef _VIDEO_SCALE_H_
#define _VIDEO_SCALE_H_

#include "noncopyable.h"

// forward declarations
class Pipeline;
class _GstElement;

/**
 *  A filter that scales video to a specified resolution.
 */

class VideoScale : private boost::noncopyable
{
    public:
        VideoScale(const Pipeline &pipeline, int width, int height);
        /// Found by trial and error, neither width nor height may exceed this value
        static const int MAX_SCALE = 2046;
        _GstElement *sinkElement() { return videoscale_; }
        _GstElement *srcElement() { return capsfilter_; }


    private:
        _GstElement *videoscale_;
        _GstElement *capsfilter_;
};

#endif //_VIDEO_SCALE_H_

