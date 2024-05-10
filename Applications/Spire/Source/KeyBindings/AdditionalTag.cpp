#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

const QString& AdditionalTag::get_name() const {
  return m_name;
}

int AdditionalTag::get_key() const {
  return m_key;
}

QWidget* AdditionalTag::make_view() const {
  return make_view(make_value());
}

AdditionalTag::AdditionalTag(QString name, int key)
  : m_name(std::move(name)),
    m_key(key) {}

Nexus::Tag to_tag(const AdditionalTag& tag) {
  if(auto value = tag.make_value()->get()) {
    return Nexus::Tag(tag.get_key(), *value);
  }
  return Nexus::Tag();
}
