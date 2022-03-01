#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

BlotterWindow::BlotterWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle("Blotter");
  set_svg_icon(":/Icons/blotter.svg");
  setFixedSize(scale(384, 346));
}
