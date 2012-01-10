
#include <stdlib.h>
#include <clutter-gst/clutter-gst.h>

void
size_change (ClutterTexture *texture,
             gint            width,
             gint            height,
             gpointer        user_data)
{
  ClutterActor *stage;
  gfloat new_x, new_y, new_width, new_height;
  gfloat stage_width, stage_height;

  stage = clutter_actor_get_stage (CLUTTER_ACTOR (texture));
  if (stage == NULL)
    return;

  clutter_actor_get_size (stage, &stage_width, &stage_height);

  new_height = (height * stage_width) / width;
  if (new_height <= stage_height)
    {
      new_width = stage_width;

      new_x = 0;
      new_y = (stage_height - new_height) / 2;
    }
  else
    {
      new_width  = (width * stage_height) / height;
      new_height = stage_height;

      new_x = (stage_width - new_width) / 2;
      new_y = 0;
    }

  clutter_actor_set_position (CLUTTER_ACTOR (texture), new_x, new_y);
  clutter_actor_set_size (CLUTTER_ACTOR (texture), new_width, new_height);
}





int
main (int argc, char *argv[])
{
  ClutterTimeline  *timeline;
  ClutterActor     *stage;
  ClutterActor     *texture;
  GstPipeline      *pipeline;
  GstElement       *source;
  GstElement       *deserializer;
  GstElement       *cluttersink;

  if (argc != 2) {
    g_printerr ("Usage: %s <socket-path>\n", argv[0]);
    return -1;
  }


  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    {
      g_error ("Failed to initialize clutter\n");
      return EXIT_FAILURE;
    }
  gst_init (&argc, &argv);

  stage = clutter_stage_get_default ();

  /* Make a timeline */
  timeline = clutter_timeline_new (1000);
  g_object_set(timeline, "loop", TRUE, NULL);

  /* We need to set certain props on the target texture currently for
   * efficient/corrent playback onto the texture (which sucks a bit)
  */
  texture = (ClutterActor*) g_object_new (CLUTTER_TYPE_TEXTURE,
			  "disable-slicing", TRUE,
			  NULL);

  g_signal_connect (CLUTTER_TEXTURE (texture),
		    "size-change",
		    G_CALLBACK (size_change), NULL);

  /* Set up pipeline */
  pipeline = GST_PIPELINE(gst_pipeline_new (NULL));

  /* Create gstreamer elements */
  source     = gst_element_factory_make ("shmsrc",  "video-source");
  deserializer = gst_element_factory_make ("gdpdepay",  "deserializer");
  cluttersink = clutter_gst_video_sink_new (CLUTTER_TEXTURE (texture));

if ( !source || !deserializer || !cluttersink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

 /* Set up the pipeline */
  g_object_set (G_OBJECT (source), "socket-path", argv[1], NULL);

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, deserializer, cluttersink, NULL);
 
  /* we link the elements together */
  gst_element_link_many (source, deserializer,cluttersink, NULL);

  /* Set the pipeline to "playing" state*/
  g_print ("Now reading: %s\n", argv[1]);
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PLAYING);

  /* start the timeline */
  clutter_timeline_start (timeline);

  clutter_group_add (CLUTTER_GROUP (stage), texture);
  // clutter_actor_set_opacity (texture, 0x11);
  clutter_actor_show_all (stage);

  clutter_main();

  return EXIT_SUCCESS;
}
