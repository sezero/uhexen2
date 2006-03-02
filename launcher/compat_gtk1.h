
// "gtk-1.2 doesnt have it" wrappers...
#ifdef WITH_GTK1

#define gtk_widget_set_size_request gtk_widget_set_usize
#define gtk_window_set_resizable(x, y) gtk_window_set_policy((x), (y), (y), (y))
#define gtk_button_set_label(x, y) gtk_label_set_text (GTK_LABEL(GTK_BIN((x))->child), (y))

#endif

