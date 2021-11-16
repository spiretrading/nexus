#include "Spire/Spire/Utility.hpp"
#include <QWidget>

using namespace Spire;

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
