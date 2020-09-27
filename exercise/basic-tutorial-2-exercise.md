# Description

https://gstreamer.freedesktop.org/documentation/tutorials/basic/concepts.html?gi-language=c#exercise

If you feel like practicing, try this exercise: Add a video filter element in between the source and the sink of this pipeline. Use vertigotv for a nice effect. You will need to create it, add it to the pipeline, and link it with the other elements.

Depending on your platform and available plugins, you might get a “negotiation” error, because the sink does not understand what the filter is producing (more about negotiation in Basic tutorial 6: Media formats and Pad Capabilities). In this case, try to add an element called videoconvert after the filter (this is, build a pipeline of 4 elements. More on videoconvert in Basic tutorial 14: Handy elements).