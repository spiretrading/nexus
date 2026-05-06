#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"

using namespace Nexus;
using namespace Spire;

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory()
  : BookViewPropertiesWindowFactory(
      std::make_shared<LocalBookViewPropertiesModel>(
        BookViewProperties::get_default())) {}

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory(
  std::shared_ptr<BookViewPropertiesModel> properties)
  : m_properties(std::move(properties)),
    m_ticker(std::make_shared<LocalTickerModel>()) {}

const std::shared_ptr<BookViewPropertiesModel>&
    BookViewPropertiesWindowFactory::get_properties() const {
  return m_properties;
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(
    std::shared_ptr<KeyBindingsModel> key_bindings) {
  if(!m_properties_window) {
    m_properties_window = std::make_unique<BookViewPropertiesWindow>(
      m_properties, std::move(key_bindings), m_ticker);
  }
  return m_properties_window.get();
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(
    std::shared_ptr<KeyBindingsModel> key_bindings, const Ticker& ticker) {
  m_ticker->set(ticker);
  return make(std::move(key_bindings));
}
