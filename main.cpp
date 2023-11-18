#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[])
{
  GstElement *pipeline, *source, *encoder, *converter, *sink, *queue1, *queue2, *queue3, *flvmux;
  GstBus *bus;
  GstMessage *message;
  GstStateChangeReturn ret;

  gst_init(&argc, &argv);

  converter = gst_element_factory_make("videoconvert", "converter");
  encoder = gst_element_factory_make("x264enc", "encoder");
  sink = gst_element_factory_make("rtmpsink", "sink");
  queue1 = gst_element_factory_make("queue", "queue1");
  queue2 = gst_element_factory_make("queue", "queue2");
  queue3 = gst_element_factory_make("queue", "queue3");
  flvmux = gst_element_factory_make("flvmux", "flvmux");

  if (!converter || !encoder || !sink || !queue1 || !queue2 || !queue3 || !flvmux)
  {
    std::cerr << "Not all elements could be created." << std::endl;

    return -1;
  }

  g_object_set(sink, "location", "rtmp://localhost/stream", NULL);

  pipeline = gst_pipeline_new("pipeline");

  if (!pipeline)
  {
    std::cerr << "Pipeline could not be created." << std::endl;

    return -1;
  }

#if defined(_WIN32) || defined(_WIN64)
  source = gst_element_factory_make("dx9screencapsrc", "source");
#elif defined(__APPLE__)
  source = gst_element_factory_make("avfvideosrc", "source");
#else
  source = gst_element_factory_make("ximagesrc", "source");
#endif

  if (!source)
  {
    std::cerr << "Screen capture source could not be created." << std::endl;
    gst_object_unref(pipeline);

    return -1;
  }

  gst_bin_add_many(GST_BIN(pipeline), source, converter, queue1, encoder, queue2, flvmux, queue3, sink, NULL);

  if (!gst_element_link_many(source, converter, queue1, encoder, queue2, flvmux, queue3, sink, NULL))
  {
    std::cerr << "Elements could not be linked" << std::endl;
    gst_object_unref(pipeline);

    return -1;
  }

  ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

  if (ret == GST_STATE_CHANGE_FAILURE)
  {
    std::cerr << "Unable to set the pipeline to playing state" << std::endl;
    gst_object_unref(pipeline);

    return -1;
  }

  bus = gst_element_get_bus(pipeline);
  message = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  if (message != NULL)
  {
    gst_message_unref(message);
  }

  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);

  return 0;
}
