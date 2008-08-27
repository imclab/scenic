import sys

# app import
import libsip_export as sip

def connect():
    return uac.connect("<sip:bloup@192.168.1.230:5060>")

def disconnect():
    return uac.disconnect()

intro = """\n
           Unitary tests.
           Will be tested:
              - library initialisation
              - connection to an UAS in AUTO mode
              - disconnection
              - connection to an UAS in MANUAL mode 
              - list media
              - set media
              - list media
              - reinvite with the new SDP
              - chat test
              - disconnection
              - shutdown
           \n""";

success = " [OK]\n\n "
failure = " [failed]\n\n  "

print intro 

step = " \nLibrary initialisation and user agent instance creation  ....................................................... "
port = int(sys.argv[1])
uac = sip.SIPSession( port )
uac.setMedia("audio", "vorbis/PCMA/", 12345)
print step+success

step = " \nConnection to a UAS runnning on the local host on the port 5060 in AUTO answer mode  ..........................  "
status = connect()
if(status == 0):
    print step+success
else:
    print step+failure

while( uac.getConnectionState() != 'CONNECTION_STATE_CONNECTED' ):
    pass

step = " \nList the enabled media list ....................................................................................  "
list = uac.mediaToString();
print list
print step+success

step = " \nChange the media and list it ...................................................................................  "
#uac.setMedia("audio", "PCMA/vorbis/", 12345)
list = uac.mediaToString()
print list
print step+success

#step = " \nReinvite the UAS with the new media description .................................................................."
#status = uac.reinvite()
#if(status == 0):
#    print step+success
#else:
#    print step+failure

step = " \nChat test. Envoi du message 'SALUT' .............................................................................. "
uac.sendInstantMessage("SALUT")
print step+success

step = " \nDeconnection from the UAS. Send BYE message .....................................................................  "
status = disconnect()
if(status == 0):
    print step+success
else:
    print step+failure
    sys.exit(0)

step = " \nShutdown pjsip, free allocated memory ...........................................................................  "
status = uac.shutdown()
if(status == 0):
    print step+success
else:
    print step+failure
    sys.exit(0)

print " \n Done! Unitary tests successful\n"
