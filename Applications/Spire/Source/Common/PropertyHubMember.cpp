#include "Spire/Spire/PropertyHubMember.hpp"
#include <utility>
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

PropertyHubMember::PropertyHubMember(
    std::shared_ptr<ListModel<PropertyHubMember*>> roster, QWidget& component,
    QString name, QString icon_path, std::shared_ptr<PropertyHub> hub)
    : m_roster(std::move(roster)),
      m_component(&component),
      m_name(std::make_shared<LocalValueModel<QString>>(std::move(name))),
      m_icon_path(std::move(icon_path)),
      m_hub(std::make_shared<LocalValueModel<std::shared_ptr<PropertyHub>>>(
        std::move(hub))) {
  m_roster->push(this);
}

PropertyHubMember::~PropertyHubMember() {
  for(auto i = m_roster->get_size() - 1; i >= 0; --i) {
    if(m_roster->get(i) == this) {
      m_roster->remove(i);
      return;
    }
  }
}

QWidget& PropertyHubMember::get_component() const {
  return *m_component;
}

const std::shared_ptr<ValueModel<QString>>&
    PropertyHubMember::get_name() const {
  return m_name;
}

const QString& PropertyHubMember::get_icon_path() const {
  return m_icon_path;
}

const std::shared_ptr<PropertyHubMember::HubModel>&
    PropertyHubMember::get_hub() const {
  return m_hub;
}
