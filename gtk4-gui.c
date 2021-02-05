// clang gtk4-gui.c `pkg-config --libs --cflags gtk4` -Wall && ./a.out
// https://developer.gnome.org/gtk4/unstable/ch01s03.html#gtk-getting-started-grid-packing
#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
  printf("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(window), "Window");
  GtkWidget *grid = gtk_grid_new();
  gtk_window_set_child(GTK_WINDOW(window), grid);

  GtkWidget *button1 = gtk_button_new_with_label("Button 1");
  g_signal_connect(button1, "clicked", G_CALLBACK(print_hello), NULL);
  gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
  // gtk_grid_attach(GtkGrid*, GtkWidget*, left, top, width, height);

  GtkWidget *button2 = gtk_button_new_with_label("Button 2");
  g_signal_connect(button2, "clicked", G_CALLBACK(print_hello), NULL);
  gtk_grid_attach(GTK_GRID(grid), button2, 1, 0, 1, 1);

  GtkWidget *button3 = gtk_button_new_with_label("Quit");
  g_signal_connect_swapped(button3, "clicked", G_CALLBACK(gtk_window_destroy),
                           window);
  gtk_grid_attach(GTK_GRID(grid), button3, 0, 1, 2, 1);

  GtkWidget *label = gtk_label_new("\nHello World\n");
  gtk_label_set_selectable(GTK_LABEL(label), TRUE);
  gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 2, 1);

  gtk_widget_show(window);
}

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
