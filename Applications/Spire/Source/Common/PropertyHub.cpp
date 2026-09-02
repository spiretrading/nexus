#include "Spire/Spire/PropertyHub.hpp"
#include <boost/uuid/uuid_generators.hpp>

using namespace boost::uuids;
using namespace Spire;

namespace {
  auto generate_id() {
    static auto generator = random_generator();
    return generator();
  }
}

PropertyHub::PropertyHub()
  : PropertyHub(generate_id()) {}

PropertyHub::PropertyHub(uuid id)
  : m_id(id) {}

const uuid& PropertyHub::get_id() const {
  return m_id;
}

std::shared_ptr<AnyValueModel> PropertyHub::find(
    const std::string& name) const {
  auto i = m_properties.find(name);
  if(i == m_properties.end()) {
    return nullptr;
  }
  return i->second;
}

void PropertyHub::adopt(const PropertyHub& hub) {
  for(auto& property : hub.m_properties) {
    m_properties.emplace(property.first, property.second);
  }
}
