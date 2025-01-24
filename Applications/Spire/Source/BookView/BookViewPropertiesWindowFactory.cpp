#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"

using namespace Nexus;
using namespace Spire;

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory(
  std::shared_ptr<KeyBindingsModel> key_bindings, const MarketDatabase& markets)
  : BookViewPropertiesWindowFactory(
      std::make_shared<LocalBookViewPropertiesModel>(
        BookViewProperties(BookViewLevelProperties::get_default(),
          BookViewHighlightProperties::get_default())),
      std::move(key_bindings), markets) {}

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory(
  std::shared_ptr<BookViewPropertiesModel> properties,
  std::shared_ptr<KeyBindingsModel> key_bindings,
  const Nexus::MarketDatabase& markets)
  : m_properties(std::move(properties)),
    m_key_bindings(std::move(key_bindings)),
    m_markets(markets),
    m_security(std::make_shared<LocalSecurityModel>()) {}

const std::shared_ptr<BookViewPropertiesModel>&
    BookViewPropertiesWindowFactory::get_properties() const {
  return m_properties;
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(const Security& security) {
  if(m_properties_window) {
    if(m_security->get() != security) {
      m_security->set(security);
    }
    return m_properties_window.get();
  }
  m_properties_window = std::make_unique<BookViewPropertiesWindow>(
    m_properties, m_key_bindings, m_security, m_markets);
  return m_properties_window.get();
}
