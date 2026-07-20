#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include <ranges>

using namespace Nexus;
using namespace Spire;

namespace {
  void copy_interactions(const InteractionsKeyBindingsModel& from,
      InteractionsKeyBindingsModel& to) {
    to.get_default_quantity()->set(from.get_default_quantity()->get());
    for(auto i :
        std::views::iota(0, InteractionsKeyBindingsModel::MODIFIER_COUNT)) {
      auto modifier = to_modifier(i);
      to.get_quantity_increment(modifier)->set(
        from.get_quantity_increment(modifier)->get());
      to.get_price_increment(modifier)->set(
        from.get_price_increment(modifier)->get());
    }
    to.is_cancel_on_fill()->set(from.is_cancel_on_fill()->get());
  }
}

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory()
  : BookViewPropertiesWindowFactory(
      std::make_shared<LocalBookViewPropertiesModel>(
        BookViewProperties::get_default())) {}

BookViewPropertiesWindowFactory::BookViewPropertiesWindowFactory(
  std::shared_ptr<BookViewPropertiesModel> properties)
  : m_properties(std::move(properties)),
    m_ticker(std::make_shared<LocalTickerModel>()),
    m_are_interactions_detached(true),
    m_has_interactions_snapshot(false) {}

const std::shared_ptr<BookViewPropertiesModel>&
    BookViewPropertiesWindowFactory::get_properties() const {
  return m_properties;
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(
    std::shared_ptr<KeyBindingsModel> key_bindings) {
  if(!m_properties_window) {
    m_key_bindings = std::move(key_bindings);
    m_window_proxy = make_proxy_value_model(m_properties);
    m_properties_window = std::make_unique<BookViewPropertiesWindow>(
      m_window_proxy, m_key_bindings, m_ticker);
    m_submit_connection = m_properties_window->connect_submit_signal(
      std::bind_front(&BookViewPropertiesWindowFactory::on_submit, this));
    m_cancel_connection = m_properties_window->connect_cancel_signal(
      std::bind_front(&BookViewPropertiesWindowFactory::on_cancel, this));
  }
  return m_properties_window.get();
}

BookViewPropertiesWindow* BookViewPropertiesWindowFactory::make(
    std::shared_ptr<KeyBindingsModel> key_bindings, const Ticker& ticker,
    std::shared_ptr<ProxyValueModel<BookViewProperties>> live_preview) {
  make(std::move(key_bindings));
  if(m_live_preview == live_preview) {
    if(m_ticker->get() != ticker) {
      m_ticker->set(ticker);
    }
    return m_properties_window.get();
  }
  if(m_live_preview) {
    revert_interactions();
    m_live_preview->set_source(m_properties);
  }
  if(m_ticker->get() != ticker) {
    m_ticker->set(ticker);
  }
  m_preview = std::make_shared<LocalBookViewPropertiesModel>(
    m_properties->get());
  m_live_preview = std::move(live_preview);
  m_live_preview->set_source(m_preview);
  m_window_proxy->set_source(m_preview);
  snapshot_interactions();
  return m_properties_window.get();
}

void BookViewPropertiesWindowFactory::snapshot_interactions() {
  auto& current =
    m_key_bindings->get_interactions_key_bindings(m_ticker->get());
  if(current) {
    m_are_interactions_detached = current->is_detached();
    copy_interactions(*current, m_initial_interactions);
  } else {
    m_are_interactions_detached = true;
  }
  m_has_interactions_snapshot = true;
}

void BookViewPropertiesWindowFactory::revert_interactions() {
  if(!m_has_interactions_snapshot) {
    return;
  }
  auto& current =
    m_key_bindings->get_interactions_key_bindings(m_ticker->get());
  if(current && current->is_detached()) {
    if(m_are_interactions_detached) {
      copy_interactions(m_initial_interactions, *current);
    } else {
      current->reset();
    }
  }
  m_has_interactions_snapshot = false;
}

void BookViewPropertiesWindowFactory::on_submit() {
  if(!m_live_preview) {
    return;
  }
  m_properties->set(m_preview->get());
  m_live_preview->set_source(m_properties);
  m_window_proxy->set_source(m_properties);
  m_live_preview.reset();
  m_preview.reset();
  m_has_interactions_snapshot = false;
}

void BookViewPropertiesWindowFactory::on_cancel() {
  if(!m_live_preview) {
    return;
  }
  m_live_preview->set_source(m_properties);
  m_window_proxy->set_source(m_properties);
  m_live_preview.reset();
  m_preview.reset();
  revert_interactions();
}
