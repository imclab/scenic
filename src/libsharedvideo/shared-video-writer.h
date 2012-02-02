#ifndef _SCENIC_SHARED_VIDEO_H_
#define _SCENIC_SHARED_VIDEO_H_

#include <string>


namespace ScenicSharedVideo
{
   class Writer {
    public:
       Writer (GstElement *pipeline,GstElement *videoElement,const std::string socketPath);
       Writer ();
	~Writer ();
    private:
	GstElement *qserial_;
	GstElement *serializer_;
	GstElement *shmsink_;   
	gboolean timereset_;
	GstClockTime timeshift_;
	static gboolean reset_time (GstPad * pad, GstMiniObject * mini_obj, gpointer user_data);
	static void pad_unblocked (GstPad * pad, gboolean blocked, gpointer user_data);
	static void switch_to_new_serializer (GstPad * pad, gboolean blocked, gpointer user_data );
	static void on_client_connected (GstElement * shmsink, gint num, gpointer user_data); 
    };

}

#endif //_SCENIC_SHARED_VIDEO_H_

