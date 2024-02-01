#include "Spire/LegacyUI/HashQtTypes.hpp"
#include <QKeySequence>

using namespace std;

size_t hash<QKeySequence>::operator ()(const QKeySequence& value) const {
  return hash<string>()(value.toString().toStdString());
}
