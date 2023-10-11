#include "Spire/Canvas/Operations/Translation.hpp"

using namespace Spire;

const std::type_info& Translation::GetTypeInfo() const {
  return *m_type;
}

Translation Translation::ToWeak() const {
  return Translation(*m_type, m_holder->ToWeak());
}

Translation Translation::ToShared() const {
  return Translation(*m_type, m_holder->ToShared());
}

Translation::Translation(
    const std::type_info& type, std::shared_ptr<const BaseHolder> holder)
  : m_type(&type),
    m_holder(std::move(holder)) {}
