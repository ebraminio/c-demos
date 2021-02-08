// gcc xlib-gui.c `pkg-config --cflags --libs x11` -Wall && ./a.out
// https://rosettacode.org/wiki/Window_creation/X11#C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

int main() {
  Display *d = XOpenDisplay(NULL);
  if (!d)
    return 1;

  int s = DefaultScreen(d);
  Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 100, 100, 1,
                                 BlackPixel(d, s), WhitePixel(d, s));
  XSelectInput(d, w, ExposureMask | KeyPressMask);
  XMapWindow(d, w);

  while (1) {
    XEvent e;
    XNextEvent(d, &e);
    if (e.type == Expose) {
      XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
      const char *msg = "Hello, World!";
      XDrawString(d, w, DefaultGC(d, s), 10, 50, msg, strlen(msg));
    }
    if (e.type == KeyPress)
      break;
  }

  XCloseDisplay(d);
  return 0;
}