#include "Spire/Styles/SelectConnection.hpp"

using namespace Spire;
using namespace Spire::Styles;

SelectConnection::SelectConnection(SelectConnection&& connection) {
  if(connection.is_connected()) {
    m_executor = std::move(connection.m_executor);
  }
}

bool SelectConnection::is_connected() const {
  return m_executor != nullptr && m_executor->is_connected();
}

void SelectConnection::disconnect() {
  m_executor = nullptr;
}

SelectConnection& SelectConnection::operator =(SelectConnection&& connection) {
  if(connection.is_connected()) {
    m_executor = std::move(connection.m_executor);
  } else {
    m_executor = nullptr;
  }
  return *this;
}
