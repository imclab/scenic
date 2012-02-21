

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappbuffer.h>
//#include <gst/app/gstappsink.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <unistd.h>//sleep
#include <string>
#include "shared-video.h"

typedef struct _App App;
struct _App
{
    GstElement *pipe;
    GstElement *src;
    GstElement *id;
    gboolean on;
    ScenicSharedVideo::Writer *writer;
};

App s_app;

static void dont_eat_my_chicken_wings (void *priv);

//clean up pipeline when ctrl-c
void
leave(int sig) {
    s_app.on = false;
    gst_element_set_state (s_app.pipe, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (s_app.pipe));
    exit(sig);
}


int
main (int argc, char *argv[])
{
    App *app = &s_app;
    int i;

    (void) signal(SIGINT,leave);

    std::string socketName;

    /* Check input arguments */
    if (argc != 2) {
	g_printerr ("Usage: %s <socket-path>\n", argv[0]);
	return -1;
    }
    socketName.append (argv[1]);
    
    gst_init (&argc, &argv);

    app->pipe = gst_pipeline_new (NULL);
    g_assert (app->pipe);

    app->src = gst_element_factory_make ("appsrc", NULL);
    g_assert (app->src);
    gst_bin_add (GST_BIN (app->pipe), app->src);

    GstCaps *mycaps = gst_caps_new_simple ("application/nico", NULL); \
    gst_app_src_set_caps (GST_APP_SRC(app->src), mycaps);
    //unref ?

    app->id = gst_element_factory_make ("identity", NULL);
    g_assert (app->id);
    gst_bin_add (GST_BIN (app->pipe), app->id);

    app->writer = new ScenicSharedVideo::Writer (app->pipe,app->id,socketName);
    gst_element_link (app->src, app->id);

    // GstElement *shmsink = gst_element_factory_make ("shmsink", NULL);
    // g_assert (shmsink);
    // gst_bin_add (GST_BIN (app->pipe), shmsink);
    
    // g_object_set (G_OBJECT (shmsink), "socket-path", socketName.c_str(), NULL);


    // gst_element_link_many (app->src, app->id, shmsink, NULL);

    app->on = true;
    gst_element_set_state (app->pipe, GST_STATE_PLAYING);

    while (app->on){  
	for (i = 0; i < 1; i++) {
	    GstBuffer *buf;
	    void *data;

	    data = malloc (100);
	    memset (data, i, 100);
    
	    buf = gst_app_buffer_new (data, 100, dont_eat_my_chicken_wings, data);
	    
	    printf ("%d: creating buffer for pointer %p, %p\n", i, data, buf);
	    gst_app_src_push_buffer (GST_APP_SRC (app->src), buf);
	}
	sleep (1);
    }

    /* push EOS */
    //gst_app_src_end_of_stream (GST_APP_SRC (app->src));

    gst_element_set_state (app->pipe, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (app->pipe));

    return 0;
}

static void
dont_eat_my_chicken_wings (void *priv)
{
    printf ("freeing buffer for pointer %p\n", priv);
    free (priv);
}