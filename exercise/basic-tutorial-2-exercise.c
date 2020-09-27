#include <gst/gst.h>

/* 
Exercide description:
Add a video filter element in between the source and the sink of this pipeline. Use vertigotv for a nice effect.

Note: I got the dreaded 'negotiation error' that the Exercise description warns about, which is why
      I had to follow the suggestion of adding a `videoconvert` element.
*/



int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink, *vfilter, *vconverter; // New
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  gst_init (&argc, &argv);

  source = gst_element_factory_make ("videotestsrc", "source");
  sink = gst_element_factory_make ("autovideosink", "sink");
  vfilter = gst_element_factory_make("vertigotv", "filter"); // New
  vconverter = gst_element_factory_make("videoconvert", "csp");

  pipeline = gst_pipeline_new ("test-pipeline");

  if (!pipeline || !source || !sink || !vfilter || !vconverter) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  gst_bin_add_many (GST_BIN (pipeline), source, vfilter, vconverter, sink, NULL); // New

  if (gst_element_link (source, vfilter) != TRUE) {
    g_printerr ("Source and filter could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  if (gst_element_link(vfilter, vconverter) != TRUE) {
      g_printerr("Filter and converter could not be linked.\n");
      gst_object_unref(pipeline);
      return -1;
  }

  if (gst_element_link(vconverter, sink) != TRUE) {
      g_printerr("Converter and sink could not be linked.\n");
      gst_object_unref(pipeline);
      return -1;
  }

  g_object_set (source, "pattern", 1, NULL);

  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }

  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
