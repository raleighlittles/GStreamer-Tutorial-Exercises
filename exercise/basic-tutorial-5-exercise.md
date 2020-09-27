# Description

https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html?gi-language=c#exercise

If this media player is not good enough for you, try to change the text widget that displays the information about the streams into a proper list view (or tree view). Then, when the user selects a different stream, make GStreamer switch streams! To switch streams, you will need to read Playback tutorial 1: Playbin usage.

# (WIP) Hint/Solution (WIP)

Per the instructions in the exercise description, we have to use either a list view or a tree view. I opted to use a tree view, buth their design is fairly similar within GTK.

The part I am stuck on is why the row I am creating within the tree view (to store the stream information) is not getting displayed. Once this part is fixed, I'll have to also
figure out how to use [`gtk_tree_selection_get_selected()`](https://developer.gnome.org/gtk3/stable/GtkTreeSelection.html) to switch between audio and video streams depending on 
what the user selects in the tree view.