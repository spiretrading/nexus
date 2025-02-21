#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"

using namespace Nexus;
using namespace Spire;

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory()
  : BookViewPropertiesWindowFactory(
      std::make_shared<LocalBookViewPropertiesModel>(
        BookViewProperties(BookViewLevelProperties::get_default(),
          BookViewHighlightProperties::get_default()))) {}

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory(
  std::shared_ptr<BookViewPropertiesModel> properties)
  : m_properties(std::move(properties)),
    m_security(std::make_shared<LocalSecurityModel>()) {}

const std::shared_ptr<BookViewPropertiesModel>&
    BookViewPropertiesWindowFactory::get_properties() const {
  return m_properties;
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    const Security& security, const MarketDatabase& markets) {
  m_security->set(security);
  if(m_properties_window) {
    return m_properties_window.get();
  }
  m_properties_window = std::make_unique<BookViewPropertiesWindow>(
    m_properties, key_bindings, m_security, markets);
  return m_properties_window.get();
}
