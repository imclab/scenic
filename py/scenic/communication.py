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

from scenic import sig
from scenic import sic
from twisted.internet import reactor
from twisted.internet import defer
from scenic import logger

log = logger.start(name="communication")

# these value should all be readable
CANCEL_REASON_CANCELLED = "cancelled"
REFUSE_REASON_PROBLEMS = "technical problems"
REFUSE_REASON_BUSY = "busy"
REFUSE_REASON_REFUSED = "refused"
REFUSE_REASON_PROBLEM_JACKD_NOT_RUNNING = "jackd is not running"
REFUSE_REASON_PROBLEM_JACKD_RATE_MISMATCH = "jackd rate mismatch"
REFUSE_REASON_XVIDEO_NOT_FOUND = "xvideo extension not found"
REFUSE_REASON_MIDI_DEVICE_NOT_FOUND = "midi device not found"
REFUSE_REASON_CAMERA_NOT_FOUND = "v4l2 device not found"
REFUSE_REASON_DISPLAY_NOT_FOUND = "x11 display not found"
REFUSE_REASON_PROBLEM_UNSUPPORTED_AUDIO_CODEC = "unsupported audio codec"
REFUSE_REASON_PROBLEM_UNSUPPORTED_VIDEO_CODEC = "unsupported video codec"
BYE_REASON_PROBLEMS = "technical problems"

class Server(object):
    """
    TCP receiver
    """
    def __init__(self, app, negotiation_port):
        self.port = negotiation_port
        self.server_factory = sic.ServerFactory()
        self.server_factory.dict_received_signal.connect(self.on_dict_received)
        self._port_obj = None
        self.remote_ip = None
        self.last_message_received = ""
        self.received_command_signal = sig.Signal()
        self.received_command_signal.connect(app.on_server_receive_command)

    def start_listening(self):
        if not self.is_listening():
            self._port_obj = reactor.listenTCP(self.port, self.server_factory)
            return self.server_factory.connected_deferred
        else:
            log.error("Already listening !!!!!")
            return defer.succeed(True) #FIXME

    def on_dict_received(self, server_proto, d):
        log.info("Received %s" % (d["msg"]))
        log.debug("Received %s" % (d))
        msg = d
        addr = server_proto.get_peer_ip()
        self.remote_ip = addr
        self.received_command_signal(msg, addr)
        self.last_message_received = d["msg"]

    def change_port(self, new_port):
        """
        Closes the server and starts it on an other port.
       """
        self.port = new_port
        def _on_closed(unused_result):
            return self.start_listening()

        deferred = self.close()
        deferred.addCallback(_on_closed)
        return deferred

    def get_peer_ip(self):
        return self.remote_ip

    def close(self): # TODO: important !
        if self.is_listening():
            def _cb(result):
                self._port_obj = None
                return result
            d = self._port_obj.stopListening()
            d.addCallback(_cb)
            return d
        else:
            log.warning("no server to close")
            return defer.succeed(True) # FIXME!!!

    def is_listening(self):
        return self._port_obj is not None

class Client(object):
    """
    TCP sender
    Attribute connection_error_signal is a signal with arguments: error string, legible message.
    """
    def __init__(self):
        self.port = None
        self.host = None
        self.sic_sender = None
        self.clientPort = None
        self.connection_error_signal = sig.Signal()
        self.last_message_sent = ""# ACK, BYE, ACCEPT, etc.

    def connect(self, host, port):
        """
        Connects and sends an INVITE message
        @rtype: L{Deferred}
        """
        def _on_connected(proto):
            log.info("Client is connected")
            self.sic_sender = proto
            return proto

        def _on_error(reason):
            log.error("Client could not connect to %s on port %s" % (host, port))
            self._connected = False
            self.sic_sender = None
            err = str(reason.getErrorMessage())
            msg = "Could not send to remote host."
            self.connection_error_signal(err, msg)
            return reason

        if not self.is_connected():
            self.host = host
            self.port = port
            self.client_factory = sic.ClientFactory()
            log.debug('Trying to connect client to %s on port %s' % (self.host, self.port))

            self.clientPort = reactor.connectTCP(self.host, self.port, self.client_factory)
            self.client_factory.connected_deferred.addCallback(_on_connected).addErrback(_on_error)
            return self.client_factory.connected_deferred
        else:
            msg = "The client is already connected to some host."
            log.warning(msg)
            #TODO: return failure?
            return defer.succeed(True) # FIXME

    def send(self, msg):
        """
        Sends a dict, which has to have the key "msg".
        @param msg: dict
        @rtype: None
        """
        self.last_message_sent = msg["msg"]
        if self.is_connected():
            log.info("Sending %s" % (msg["msg"]))
            self.sic_sender.send_message(msg)
        else:
            error = "Not connected, cannot send message " + str(msg)
            log.error(error)

    def is_connected(self):
        return self.sic_sender is not None

    def disconnect(self):
        """
        @rtype: Deferred
        """
        if self.is_connected():
            unused_d = self.clientPort.transport.loseConnection() # TODO: trigger a deffered when connection lost
            self.port = None
            self.sic_sender = None
            return defer.succeed(True)
        else:
            log.warning("Already disconnected.")
            return defer.succeed(True) # FIXME

def connect_send_and_disconnect(host, port, mess):
    d = defer.Deferred()
    def _on_connected(sic_sender):
        sic_sender.send_message(mess)
        clientPort.transport.loseConnection()
        d.callback(True)
    def _on_error(reason):
        log.error(str(reason))
        d.errback(reason)
        return None
    client_factory = sic.ClientFactory()
    log.info('trying to connect to %s %s to send message %s' % (host, port, mess["msg"]))
    clientPort = reactor.connectTCP(host, port, client_factory)
    client_factory.connected_deferred.addCallback(_on_connected).addErrback(_on_error)
    return d
