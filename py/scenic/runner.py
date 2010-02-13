#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

"""
Main of the application.
"""
import sys
import os
from optparse import OptionParser
from twisted.internet import gtk2reactor
gtk2reactor.install() # has to be done before importing reactor
from twisted.internet import reactor
from twisted.internet import error
from twisted.python import log
from twisted.python import logfile

from scenic import application
from scenic import configure

def start_logging_to_stdout():
    log.startLogging(sys.stdout)

def start_file_logging(full_path="/var/tmp/scenic/scenic.log"):
    """
    Starts logging the Master infos to a file.
    @rettype: str path to the log file
    """
    file_name = os.path.basename(full_path)
    directory = os.path.dirname(full_path)
    if not os.path.exists(directory):
        os.makedirs(directory)
    f = open(full_path, 'w') # erases previous file
    f.close()
    _log_file = logfile.DailyLogFile(file_name, directory)
    log.startLogging(_log_file)
    return _log_file.path

LOG_FILE_NAME = "~/.scenic/scenic.log"

def run():
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(configure.VERSION))
    parser.add_option("-k", "--kiosk", action="store_true", help="Run in kiosk mode")
    parser.add_option("-l", "--enable-logging", action="store_true", help="Enables logging to file.")
    parser.add_option("-L", "--log-file-name", type="string", help="Specifies the path to the log file. Default is %s" % (LOG_FILE_NAME), default=LOG_FILE_NAME)
    parser.add_option("-f", "--fullscreen", action="store_true", help="Run in fullscreen mode")
    (options, args) = parser.parse_args()
    kwargs = {}
    if options.enable_logging:
        start_file_logging(os.path.expanduser(options.log_file_name))
        kwargs["log_file_name"] = options.log_file_name
    else:
        start_logging_to_stdout()
    try:
        app = application.Application(kiosk_mode=options.kiosk, fullscreen=options.fullscreen, **kwargs)
    except error.CannotListenError, e:
        print("There must be an other Scenic running.")
        print(str(e))
    else:
        try:
            reactor.run()
        except KeyboardInterrupt:
            pass