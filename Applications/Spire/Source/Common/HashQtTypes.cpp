#include "Spire/Spire/HashQtTypes.hpp"
#include <QKeySequence>

std::size_t std::hash<QKeySequence>::operator ()(
    const QKeySequence& value) const {
  return std::hash<std::string>()(value.toString().toStdString());
}
