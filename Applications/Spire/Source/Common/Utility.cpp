#include "Spire/Spire/Utility.hpp"
#include <QWidget>

using namespace Spire;

void QObjectDeleter::operator ()(QObject* object) const noexcept {
  if(object) {
    object->deleteLater();
  }
}

bool Spire::is_ancestor(const QWidget* ancestor, const QWidget* descendant) {
  auto current = descendant;
  while(current) {
    if(current == ancestor) {
      return true;
    }
    current = current->parentWidget();
  }
  return false;
}
