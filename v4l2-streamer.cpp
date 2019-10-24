#include <functional>
#include <thread>
#include <gst/gst.h>


#include "websocket.h"

typedef struct
{
  int id;
  GstElement *pipeline;
  GstElement *v4l2src;
  GstElement *capsfilter1;
  GstElement *jpegdec;
  GstElement *queue1;
  GstElement *h264enc;
  GstElement *queue2;
  GstElement *h264parse;
  GstElement *capsfilter2;
  GstElement *appsink;
  GMainLoop *main_loop;
  BroadcastServer *server;
  bool ready;
} AppGstElements;

static GstFlowReturn new_frame(GstElement *sink, AppGstElements *elements)
{
  GstSample *sample;

  g_signal_emit_by_name(sink, "pull-sample", &sample);
  
  if (sample)
  {
    GstBuffer *buffer = gst_sample_get_buffer( sample );
    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_READ);
    elements->server->broadcast(info.data, info.size);
    gst_buffer_unmap(buffer, &info);
    gst_sample_unref(sample);

    /*GstStructure *stat;
    g_object_get(sink, "stats", &stat, NULL);
    GST_LOG ("structure is %" GST_PTR_FORMAT, stat);
    gst_structure_free(stat);*/
    return GST_FLOW_OK;
  }

  return GST_FLOW_ERROR;
}

static void error_cb (GstBus *bus, GstMessage *msg, AppGstElements *data) {
  GError *err;
  gchar *debug_info;

  gst_message_parse_error (msg, &err, &debug_info);
  g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  g_clear_error (&err);
  g_free (debug_info);

  g_main_loop_quit (data->main_loop);
}

int launch_pipeline(
    AppGstElements *elements,
    char *video_device,
    int width,
    int height,
    int framerate,
    int bitrate = 0,
    bool jpegdec_vaapi = true,
    bool h264enc_vaapi = true,
    bool h264parse = true
    )
{
  elements->v4l2src = gst_element_factory_make("v4l2src", NULL);

  elements->capsfilter1 = gst_element_factory_make("capsfilter", NULL);

  elements->jpegdec = gst_element_factory_make((jpegdec_vaapi) ? "vaapijpegdec" : "jpegdec", NULL);

  elements->queue1 = gst_element_factory_make("queue", NULL);

  elements->h264enc = gst_element_factory_make((h264enc_vaapi) ? "vaapih264enc" : "x264enc", NULL);

  elements->queue2 = gst_element_factory_make("queue", NULL);

  if (h264parse) elements->h264parse = gst_element_factory_make("h264parse", NULL);

  elements->capsfilter2 = gst_element_factory_make("capsfilter", NULL);

  elements->appsink = gst_element_factory_make("appsink", NULL);

  char pipeline_name[32];
  sprintf(pipeline_name, "pipeline%d", elements->id);
  elements->pipeline = gst_pipeline_new(pipeline_name);

  if (!elements->pipeline || !elements->v4l2src || !elements->capsfilter1 || !elements->jpegdec || !elements->queue1 || !elements->h264enc || !elements->queue2 || (h264parse && !elements->h264parse) || !elements->capsfilter2 || !elements->appsink)
  {
    g_printerr("Not all elements could be created.\n");
    return 1;
  }

  g_object_set(elements->h264enc, "bitrate", bitrate, "keyframe-period", 300, NULL);
  g_object_set(elements->v4l2src, "do-timestamp", TRUE, NULL);
  
  if (video_device != NULL)
    g_object_set(elements->v4l2src, "device", video_device, NULL);

  g_object_set(elements->capsfilter1, "caps", gst_caps_new_simple(
    "image/jpeg", 
    "width", G_TYPE_INT, width, 
    "height", G_TYPE_INT, height, 
    "framerate", GST_TYPE_FRACTION, framerate, 1, 
    NULL), NULL);
  
  g_object_set(elements->capsfilter2, "caps", gst_caps_new_simple(
    "video/x-h264", 
    "stream-format", G_TYPE_STRING, "byte-stream", 
    "alignment", G_TYPE_STRING, "au", 
    "profile", G_TYPE_STRING, "constrained-baseline",
    NULL), NULL);

  g_object_set(elements->appsink, 
    "emit-signals", TRUE, 
    "sync", FALSE, 
    "drop", TRUE, 
    "max-buffers", 2,
    NULL);
  
  g_signal_connect(elements->appsink, "new-sample", G_CALLBACK(new_frame), elements);

  gst_bin_add_many(GST_BIN(elements->pipeline), elements->v4l2src, elements->capsfilter1, elements->jpegdec, elements->queue1, elements->h264enc, elements->queue2, elements->capsfilter2, elements->appsink, NULL);

  gboolean link_ret;

  if (h264parse) {
    gst_bin_add(GST_BIN(elements->pipeline), elements->h264parse);
    link_ret = gst_element_link_many(elements->v4l2src, elements->capsfilter1, elements->jpegdec, elements->queue1, elements->h264enc, elements->queue2, elements->h264parse, elements->capsfilter2, elements->appsink, NULL);
  }
  else {
    link_ret = gst_element_link_many(elements->v4l2src, elements->capsfilter1, elements->jpegdec, elements->queue1, elements->h264enc, elements->capsfilter2, elements->appsink, NULL);
  }

  if (link_ret != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (elements->pipeline);
    return 1;
  }

  GstBus *bus = gst_element_get_bus (elements->pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, elements);
  gst_object_unref (bus);

  /* Start playing the pipeline */
  g_print("Pipeline is ready\n");
  //gst_element_set_state (elements->pipeline, GST_STATE_PLAYING);
  elements->ready = true;

  elements->main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (elements->main_loop);

  gst_element_set_state (elements->pipeline, GST_STATE_NULL);
  gst_object_unref (elements->pipeline);
  elements->ready = false;
  return 0;
}



int main(int argc, char *argv[])
{
  gst_init (&argc, &argv);
  AppGstElements elements;
  memset(&elements, 0, sizeof(AppGstElements));

  elements.server = new BroadcastServer([&](BroadcastServerEvent e) {
    if (!elements.ready) {
      g_printerr("Pipeline is not ready\n");
      return;
    }
    switch (e) {
    case BroadcastServerEvent::FirstOpen:
      g_print("Streaming started\n");
      gst_element_set_state (elements.pipeline, GST_STATE_PLAYING);
      break;
    case BroadcastServerEvent::LastClose:
      g_print("Streaming paused\n");
      gst_element_set_state (elements.pipeline, GST_STATE_PAUSED);
      break;
    }
  });
  std::thread th([&]() {
    g_print("Server started\n");
    elements.server->run(8003);
  });

  int ret = launch_pipeline(&elements, NULL, 1280, 720, 30);
  g_print("launch_pipeline returned %d\n", ret);
  th.join();

  return 0;
}