#include "Spire/UI/HashQtTypes.hpp"
#include <QKeySequence>

using namespace std;

size_t hash<QKeySequence>::operator ()(
    const QKeySequence& value) const noexcept {
  return hash<string>()(value.toString().toStdString());
}
