/* logWriter.h
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

#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include "except.h"
#include <string>
#include <sstream>
#include "config.h"


#define ENABLE_LOG 1


#if !ENABLE_LOG
#define LOG(msg, level)
#else
#define LOG(msg, level)     LOG_(msg,level,0)
#endif

#define THROW_ERRNO(msg,err)      THROW_(msg, ERROR,err)
#define THROW_END_THREAD(msg)  THROW_(msg, THROW,0)
#define THROW_ERROR(msg)      THROW_(msg, ERROR,0)
#define THROW_CRITICAL(msg)   THROW_(msg, CRITICAL,0)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_ERROR(msg)       LOG(msg, ERROR)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)

#define COUT_LOG(msg)       LOG(msg, NONE)
/**  
 *      Utility functions for logWriter 
 *
 *      Log writer macro, usage:
 *
 *      LOG_DEBUG("This is a log");
 *      
 *      LOG_INFO("Got Error errno: " << 100);
 *      
 *      THROW_ERROR(99);
 *
 *      To disable logging, define the macro ENABLE_LOG (in this file) to 0.
 *
 */


namespace Log
{
    /// log Subscriber 
    class Subscriber
    {
        public:
            void enable();
            void hold();
            Subscriber();
            virtual void operator()(LogLevel&, std::string &){}
            virtual void setLevel(int){}
            virtual ~Subscriber();
    };
}

#define QUIET_THROW(msg)           LOG_(msg,NONE,-1)
#define THROW_(msg, level,err)     LOG_(msg,level,err)
//Note mangle84579568749576948 varible name so that hiding an existing 
//variable is unlikely
//Do{} while(0) construct to preserve one statement syntax of LOG()

#define LOG_(msg, level, err)                 \
            do{                         \
            std::ostringstream mangle84579568749576948;      \
            mangle84579568749576948 << msg;                  \
            cerr_log_throw(mangle84579568749576948.str(), level, __FILE__, __FUNCTION__, __LINE__, err);    \
            }                           \
            while(0)

std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum);

void cerr_log_throw( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum,int err);


#endif //  _LOG_WRITER_H_
