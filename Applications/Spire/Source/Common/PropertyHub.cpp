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
  : m_id(generate_id()) {}

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

void PropertyHub::remove(const std::string& name) {
  m_properties.erase(name);
}
