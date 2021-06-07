#include "Spire/Spire/Utility.hpp"
#include <QWidget>

using namespace Spire;

void Spire::clear_layout(QLayout* layout) {
  while(auto item = layout->takeAt(0)) {
    item->widget()->deleteLater();
    delete item;
  }
}
