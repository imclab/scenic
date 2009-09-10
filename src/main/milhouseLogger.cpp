
/* milhouseLogger.cpp
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

#include "milhouseLogger.h"
#include <boost/bind.hpp>


MilhouseLogger::MilhouseLogger() : 
    printQueue_(), printThread_(boost::bind<void>(boost::mem_fn(&MilhouseLogger::printMessages), boost::ref(*this)))
{
}


MilhouseLogger::~MilhouseLogger()
{
    // end our print thread
    printQueue_.push("quit:");
    printThread_.join(); // wait for print thread to go out before main thread does
}

/// This is called in the main thread
void MilhouseLogger::operator()(LogLevel& /*level*/, std::string& msg)
{
    //std::cout << msg;
    printQueue_.push(msg);
//    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
}

void MilhouseLogger::printMessages()
{
    bool done = false;
    while (!done)
    {
        std::string msg;
        printQueue_.wait_and_pop(msg);
        
        if (msg != "quit:")
        {
            std::cout << msg << std::endl;
        }
        else  // got a sentinel
        {
            done = true;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(MILLISEC_WAIT));
    }
}

