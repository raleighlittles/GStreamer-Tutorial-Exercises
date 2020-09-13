#include <gst/gst.h>

 
typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *aconvert;
  GstElement *resample;
  GstElement *vconvert;
  GstElement *vsink;
  GstElement *asink;
} CustomData;

 
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  data.source = gst_element_factory_make ("uridecodebin", "source");
  data.aconvert = gst_element_factory_make ("audioconvert", "convert");
  data.resample = gst_element_factory_make ("audioresample", "resample");

  data.asink = gst_element_factory_make ("autoaudiosink", "audio_sink");
  data.vconvert = gst_element_factory_make("videoconvert", "csp");
  data.vsink = gst_element_factory_make("autovideosink", "video_sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");

  if (!data.pipeline || !data.source || !data.aconvert || !data.resample || !data.asink || !data.vsink || !data.vconvert ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  gst_bin_add_many (GST_BIN (data.pipeline), data.source, data.aconvert, data.resample, data.vconvert, data.vsink, data.asink, NULL);

  if (!gst_element_link_many (data.aconvert, data.resample, data.asink, NULL)) {
    g_printerr ("Audio pipeline elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  if (!gst_element_link_many (data.vconvert, data.vsink, NULL)) {
      g_printerr ("Video pipeline elements could not be linked.\n");
      gst_object_unref (data.pipeline);
      return -1;
  }

  /* Set the URI to play */
  g_object_set (data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

  /* Connect to the pad-added signal */
  g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  /* Listen to the bus */
  bus = gst_element_get_bus (data.pipeline);
  do {
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* Parse message */
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
          terminate = TRUE;
          break;
        case GST_MESSAGE_EOS:
          g_print ("End-Of-Stream reached.\n");
          terminate = TRUE;
          break;
        case GST_MESSAGE_STATE_CHANGED:
          /* We are only interested in state-changed messages from the pipeline */
          if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            g_print ("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
          }
          break;
        default:
          /* We should not reach here */
          g_printerr ("Unexpected message received.\n");
          break;
      }
      gst_message_unref (msg);
    }
  } while (!terminate);

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}


// TODO: Correctly link the video converter element depending on the capability of the new pad discovered.
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {

  GstPad *aconverter_sink_pad = gst_element_get_static_pad (data->aconvert, "sink");
  GstPad *vconverter_sink_pad = gst_element_get_static_pad (data->vconvert, "sink");

  if (aconverter_sink_pad == NULL) {
      g_printerr ("Audio sink pad is null.\n");
  }

  if (vconverter_sink_pad == NULL) {
    g_printerr ("Video sink pad is null.\n");
  }

  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (aconverter_sink_pad)) {
    g_print ("Audio converter pad is already linked. Ignoring.\n");
    goto exit;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);


  if (g_str_has_prefix (new_pad_type, "audio/x-raw")) {
    g_print("New pad type is raw audio, attempting to link with audio converter...\n");
    ret = gst_pad_link (new_pad, aconverter_sink_pad);

    if (GST_PAD_LINK_FAILED (ret)) {
      g_print ("Type is '%s' but link failed.\n", new_pad_type);
    } else{ g_print ("Link succeeded (type '%s').\n", new_pad_type); }
  }

  else if (g_str_has_prefix (new_pad_type, "video/x-raw")) {
    g_print("New pad type is raw video, attempting to link with video converter...\n");
    ret = gst_pad_link (new_pad, vconverter_sink_pad);

    if (GST_PAD_LINK_FAILED (ret)) {
        g_print ("Type is '%s' but link failed.\n", new_pad_type);
    } else{ g_print ("Link succeeded (type '%s').\n", new_pad_type); }
  }
  
  else {
    g_print("New pad type isn't raw audio or raw video. Ignoring\n");
    goto exit;
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (aconverter_sink_pad);
}
