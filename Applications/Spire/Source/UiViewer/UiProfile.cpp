#include "Spire/UiViewer/UiProfile.hpp"

using namespace Spire;

UiProfile::UiProfile(QString name,
    std::vector<std::shared_ptr<UiProperty>> properties,
    std::function<QWidget* (const UiProfile&)> factory)
  : m_name(std::move(name)),
    m_properties(std::move(properties)),
    m_factory(std::move(factory)) {
  reset();
}

const QString& UiProfile::get_name() const {
  return m_name;
}

const std::vector<std::shared_ptr<UiProperty>>&
    UiProfile::get_properties() const {
  return m_properties;
}

QWidget* UiProfile::get_widget() const {
  return m_widget;
}

QWidget* UiProfile::reset() {
  m_widget = m_factory(*this);
  return get_widget();
}
