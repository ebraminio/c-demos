// clang++ qt5-gui.cc `pkg-config --libs --cflags Qt5Core Qt5Widgets` -Wall && ./a.out
#include <QApplication>
#include <QLabel>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QLabel label("Hello World");
  label.setTextInteractionFlags(Qt::TextSelectableByMouse);
  label.show();
  return app.exec();
}
