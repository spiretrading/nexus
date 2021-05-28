#include "Spire/Styles/StyleSheetMap.hpp"

using namespace Spire;
using namespace Spire::Styles;

StyleSheetMap::StyleSheetMap(std::function<void ()> commit)
  : m_commit(std::move(commit)),
    m_is_buffering(false) {}

void StyleSheetMap::write(QString& stylesheet) const {
  for(auto& style : m_properties) {
    stylesheet += style.first + ": " + style.second + ";";
  }
  stylesheet += "}";
}

void StyleSheetMap::set(const QString& property, QColor color) {
  m_properties.insert_or_assign(property, color.name(QColor::HexArgb));
  if(!m_is_buffering) {
    m_commit();
  }
}

void StyleSheetMap::set(const QString& property, int value) {
  m_properties.insert_or_assign(property, QString::number(value));
  if(!m_is_buffering) {
    m_commit();
  }
}

void StyleSheetMap::clear() {
  m_properties.clear();
}
