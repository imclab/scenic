SUBDIRS=test

bin_PROGRAMS = milhouse

milhouse_SOURCES = main.cpp milhouse.cpp milhouse_logger.cpp milhouse_logger.h \
                   $(top_srcdir)/src/include/concurrent_queue.h milhouse.h \
                   program_options.cpp program_options.h
#FIXME GLIB_CFLAGS only here because pipeline.h has include glib/gtypes.h, also GDK flags because of audioSender.h->audioLevel.h
milhouse_CPPFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir)/src/include -I$(top_srcdir)/src

milhouse_LDADD = $(GST_LIBS) $(GSTAUDIO_LIBS) $(GSTVIDEO_LIBS) $(JACK_LIBS) \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) \
		$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(LIBDC1394_LIBS) \
        $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(GST_RTSP_SERVER_LIBS) \
        $(top_builddir)/src/gst/libgst.la  \
        $(top_builddir)/src/caps/libcaps.la \
        $(top_builddir)/src/devices/libdevices.la \
        $(top_builddir)/src/util/libutil.la \
        $(top_builddir)/src/gutil/libgutil.la \
        $(top_builddir)/src/shared_video/libshared_video.la \
        $(top_builddir)/src/vumeter/libvumeter.la \
        $(top_builddir)/src/rtsp/librtsp.la
