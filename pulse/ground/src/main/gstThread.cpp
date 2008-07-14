/* GTHREAD-QUEUE-PAIR - Library of GstThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gstThread.h"

//Caller gains ownership of args in list
GstThread::GstThread()
{
	args.clear();
	args.push_back(new StringArg(&conf_str,"sender",'s',"video", "try videotestsrc"));
}


int GstThread::main()
{
	BaseMessage r(BaseMessage::ping);
	conf = new VideoConfig(conf_str);
	sender = new VideoSender(*conf);
	bool quit = false;
	while(!quit)
	{
		BaseMessage f = queue.copy_timed_pop(100);
		switch(f.get_type())
		{
			case BaseMessage::quit: 
			{
				BaseMessage f(BaseMessage::quit);
				queue.push(f);
				quit = true;
			}
			break;
			case BaseMessage::start:
			{
				sender->start();
			}
			break;
			case BaseMessage::init:
			{
				sender->init();
			}
			break;
			case BaseMessage::stop:
			{
				sender->stop();
			}
			break;

			default:
			break;
		}
	}
	return 0;
}



