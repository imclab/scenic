#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

import signal
import re
import sys
from twisted.internet import reactor

#Midi Stream
from engines.midi.midi_session import MidiSession
from rtpmidi.protocols.rtp.rtp_control import RTPControl
import logging
import getopt

global rtp_control
global midi_session_c

options = [("-h", "--help", "Display help of midi_stream"),
           ("-a", "--address", "Specify the address of the client (mandatory)"),
           ("-d", "--devices-list", "Show a complete list of midi devices"),
           ("-r", "--receiving-port", 
            "Select the listening port, and activate the midi out (-o mandatory)"),

           ("-s", "--sending-port", 
            "Select the writting port, and activate midi in (-i mandatory)"),

           ("-i", "--device-in", "Select a midi in device"),
           ("-o", "--device-out", "Select a midi out device"),
           ("-l", "--latency", 
            "Specify the latency (in ms) of the midi out device (default is 20)"),

           ("-b", "--buffer-size", 
            "Specify the jitter buffer size in ms (default is 10)"),

           ("-k", "--safe-keyboard", 
            "Take care of note ON/OFF alternating, usefull if several notes in a ms (experimental)"),

           ("-j", "--journal", 
            "DISABLE recovery journal (journal provide note recovery when a packet is lost, so at your own risks!)"),

           ("-f", "--follow-standard", 
            "Take care of MIDI standard (ex: omni on) in recovery journal (experimental)"),

           ("-v", "--verbose", "Make midi stream talk")]


def usage():
    print "python midi_stream.py [OPTIONS]"
    print "Create a midi RTP flux between two hosts, uni or bi-directionnal."
    for option in options:
        out = "  " + str(option[0]) + ", " + str(option[1])
        n = 20 - len(option[1])
        out += n * " "
        out += option[2]
        print out

    print "Complete Ex: python midi_stream.py -a 10.0.1.29 -r 44000 -s 44000 -i 1 -o 0"
    print "             This command create a bi-directional connection with 10.0.1.29 "
    print "             on port 44000, midi device 1 is the source for sending data, "
    print "             and all received data are sent to midi device 0." 
    print "Caution: If the flux is bi-directionnal receiving port and sending port must be equal."

    
def check_ip(address):
    """Check address format """
    if re.match('^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address):
	    values = [int(i) for i in address.split('.')]
	    if ip_range(values):
		    return 1

	    else:
		    return 0

    else:
	    return 0
            

def ip_range(nums):
    for num in nums:
        if num < 0 or num > 255:
            return False
    return True

def check_port(portnum):
    if 0 < portnum < 65535:
        return True

    else:
        return False

def main(args):
    global rtp_control
    global midi_session_c

    #Retriving args
    (address, sending_port, receiving_port, device_in, device_out, 
     latency, jitter_buffer_size, safe_keyboard, journal, standard, verbose) = args


    if sending_port == 0 and receiving_port == 0:
        print "Error: Please select at least a port."
        sys.exit(2)

    if sending_port != 0 and receiving_port != 0:
        if sending_port != receiving_port:
            print "Error: receiving port and sending port must be equal."
            usage()
            sys.exit(2)

    midi_session_c = RTPControl().add_session(MidiSession(address, 
                                                       sport=sending_port, 
                                                       rport=receiving_port, 
                                                       latency=latency, 
                                                       jitter_buffer_size=jitter_buffer_size, 
                                                       safe_keyboard=safe_keyboard,
                                                       recovery=journal,
                                                       follow_standard=standard,
                                                       verbose=verbose))
 
    midi_session = RTPControl().get_session(midi_session_c)
    dev_in, dev_out = midi_session.get_devices()

    if device_in != -1:
        res = midi_session.set_device_in(device_in)
        if not res:
            sys.exit(2)

    if device_out != -1:
        res = midi_session.set_device_out(device_out)
        if not res:
            sys.exit(2)

    RTPControl().start_session(midi_session_c)
 

    reactor.run()

def parse_arg(): 

    address = sending_port = receiving_port = device_in = device_out = latency = jitter_buffer_size = safe_keyboard = journal = standard = verbose = -1

    try:                                
        (optis, args) = getopt.getopt(sys.argv[1:], "hda:s:i:r:o:l:b:kjfv", 
                                      ["help", "devices-list", "sending-port", 
                                       "device-in", "receiving-port", "device-out", 
                                       "latency", "buffer-size", "safe-keyboard", 
                                       "journal", "follow-standard", "verbose"])

    except getopt.GetoptError:           
        usage()                          
        sys.exit(2)

    opts = {}

    for o, a in optis:
        opts[o] = a

    if opts.has_key('-h') or opts.has_key('--help'):
        usage()
        sys.exit(0)

    if opts.has_key('-v') or opts.has_key('--verbose'):
        verbose = 1

    else:
        verbose = 0

    if opts.has_key('-j') or opts.has_key('--journal'):
        journal = 0

    else:
        journal = 1

    if opts.has_key('-f') or opts.has_key('--follow-standard'):
        standard = 1
        
    else:
        standard = 0

    if opts.has_key('-k') or opts.has_key('--safe-keyboard'):
        safe_keyboard = 1

    else:
        safe_keyboard = 0

    if opts.has_key('-d') or opts.has_key('--devices-list'):
        print "MidiStream"
        rtp_control = RTPControl()
        midi_session = rtp_control.add_session(MidiSession("127.0.0.1", 
                                                           rport=1, sport=1))

        midi_session = rtp_control.get_session(midi_session)
        dev_in, dev_out = midi_session.get_devices()
        print "List of devices:"
        print "  Device In:"
        for dev in dev_in:
            print "    ", dev[0], " ", dev[1], " ",
            
            if dev[2] == 1:
                print "[open]"

            else:
                print "[closed]"

        print "  Device Out:"
        for dev in dev_out:
            print "    ", dev[0], " ", dev[1], " ",
            
            if dev[2] == 1:
                print "[open]"

            else:
                print "[closed]"

        sys.exit()

    #Address
    if opts.has_key('-a') or opts.has_key('--address'):
        if check_ip(opts ['-a']):
            address = opts ['-a']

        else:
            print "Wrong ip address format, must be like: 10.10.10.123"
            sys.exit(2)

    if opts.has_key('-s'):
        if check_port(int(opts ['-s'])):
            sending_port = int(opts ['-s'])
            
        else:
            print "Incorrect port format for -s"
            print opts
            print "Ex: 44000 (must be between 1 and 65535)"
            sys.exit(2)

    else:
        sending_port = 0

    if opts.has_key('-i'):
        device_in = int(opts ['-i'])

    if opts.has_key('-r'):
        if check_port(int(opts ['-r'])):
            receiving_port = int(opts ['-r'])
    
        else:
            print "Incorrect port format for -r"
            print opts
            print "Ex: 44000 (must be between 1 and 65535)"
            sys.exit(2)

    else:
        receiving_port = 0

    if opts.has_key('-o'):
        device_out = int(opts ['-o'])

    if opts.has_key('-l'):
        latency = int(opts ['-l'])

    else:
        latency = 20

    if opts.has_key('-b'):
        jitter_buffer_size = int(opts ['-b'])

    else:
        jitter_buffer_size = 10

    params = (address, sending_port, receiving_port, device_in, device_out, 
              latency, jitter_buffer_size, safe_keyboard, journal, standard, verbose)

    if check_param_logic(params):
        main(params)
        
    else:
        usage()
        sys.exit(2)


def check_param_logic(params):
    address, sending_port, receiving_port, device_in, device_out, latency, \
        jitter_buffer_size, safe_keyboard, journal, standard, verbose = params

    if address == -1:
        print "Error: You must set an address."
        return False

    if sending_port == 0:
        if device_in != -1:
            print "Error: device in is useless without sending port setup."
            return False

    else:
        if device_in == -1:
            print "Error: Sending port is useless without device in setup."
            return False

    if receiving_port == 0:
        if device_out != -1:
            print "Error: device out is useless without receiving port setup."
            return False

    else:
        if device_out == -1:
            print "Error: Receiving port is useless without device out setup."
            return False

    
    return True


def handler(signum, frame):
    global launch_flag
    print "\nSIGINT caught! Shutting down midi stream module."
    RTPControl().stop()
    reactor.stop()


if __name__ == "__main__":
    signal.signal(signal.SIGINT, handler)
    parse_arg()
