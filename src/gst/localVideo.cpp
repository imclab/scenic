/* localVideo.cpp
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

#include "localVideo.h"

#include "gstLinkable.h"
#include "pipeline.h"

#include "videoSource.h"
#include "videoConfig.h"
#include "videoScale.h"
#include "videoFlip.h"
#include "textOverlay.h"
#include "videoSink.h"

#include "dv1394.h"

using std::tr1::shared_ptr;

/// Constructor
LocalVideo::LocalVideo(Pipeline &pipeline, 
        const VideoSourceConfig &sourceConfig,
        const VideoSinkConfig &sinkConfig) :
    source_(sourceConfig.createSource(pipeline)),
    colourspace_(0),
    videoscale_(sinkConfig.createVideoScale(pipeline)),
    textoverlay_(sinkConfig.createTextOverlay(pipeline)),
    videoflip_(sinkConfig.createVideoFlip(pipeline)),
    sink_(sinkConfig.createSink(pipeline))
{
    // dc1394src needs an extra colourspace converter if not being encoded or flipped
    // FIXME: maybe it just needs a capsfilter?
    if (sourceConfig.sourceString() == "dc1394src")
    {
        colourspace_ = pipeline.makeElement("ffmpegcolorspace", NULL);
        gstlinkable::link(*source_, colourspace_);
        gstlinkable::link(colourspace_, *videoscale_);
    }
    else
    {
        bool linked = false;
        int framerateIndex = 0;
        while (not linked)
        {
            try 
            {
                gstlinkable::link(*source_, *videoscale_);
                linked = true;
            }
            catch (const gstlinkable::LinkExcept &e)
            {
                LOG_WARNING("Link failed, trying another framerate");
                ++framerateIndex;
                source_->setCapsFilter(source_->srcCaps(framerateIndex));
            }
        }
    }
    
    gstlinkable::link(*videoscale_, *textoverlay_);
    gstlinkable::link(*textoverlay_, *videoflip_);
    gstlinkable::link(*videoflip_, *sink_);

    /// FIXME: hack for dv1394src
    if (sourceConfig.sourceString() == "dv1394src")
        Dv1394::Instance(pipeline)->doTimestamp();
}
