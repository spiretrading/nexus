#include "Spire/Blotter/BlotterLinkMenu.hpp"
#include <algorithm>
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost::signals2;
using namespace Spire;

BlotterLinkMenu::BlotterLinkMenu(std::shared_ptr<BlotterModel> model,
    std::shared_ptr<BlotterCollection> collection, QWidget& parent)
    : ContextMenu(parent),
      m_model(std::move(model)),
      m_collection(std::move(collection)),
      m_links(m_collection->get_links(*m_model)) {
  rebuild();
  m_collection_connection = m_collection->connect_operation_signal(
    std::bind_front(&BlotterLinkMenu::on_collection, this));
  m_links_connection = m_links->connect_operation_signal(
    std::bind_front(&BlotterLinkMenu::on_links, this));
}

void BlotterLinkMenu::rebuild() {
  m_entries.clear();
  reset();
  if(std::ranges::find(std::as_const(*m_collection), m_model) ==
      m_collection->cend()) {
    setEnabled(false);
    return;
  }
  for(auto& model : std::as_const(*m_collection)) {
    if(model == m_model || model->get_account() != m_model->get_account()) {
      continue;
    }
    auto checked = std::make_shared<LocalBooleanModel>(
      std::ranges::find(*m_links, model) != m_links->cend());
    add_check_box(model->get_name()->get(), checked);
    get_menu_item(get_count() - 1)->setEnabled(!m_model->is_consolidated());
    auto name_connection = model->get_name()->connect_update_signal(
      [=, this] (const auto&) {
        rebuild();
      });
    auto checked_connection = checked->connect_update_signal(
      [=, this] (auto is_checked) {
        if(is_checked) {
          m_collection->link(*m_model, *model);
        } else {
          m_collection->unlink(*m_model, *model);
        }
      });
    m_entries.emplace_back(model, std::move(checked),
      std::move(name_connection), std::move(checked_connection));
  }
}

void BlotterLinkMenu::on_collection(
    const BlotterCollection::Operation& operation) {
  visit(operation,
    [=, this] (const auto& operation) requires (
        std::same_as<std::remove_cvref_t<decltype(operation)>,
          BlotterCollection::AddOperation> ||
        std::same_as<std::remove_cvref_t<decltype(operation)>,
          BlotterCollection::RemoveOperation>) {
      rebuild();
    });
}

void BlotterLinkMenu::on_links(const BlotterCollection::Operation& operation) {
  visit(operation,
    [=, this] (const auto& operation) requires (
        std::same_as<std::remove_cvref_t<decltype(operation)>,
          BlotterCollection::AddOperation> ||
        std::same_as<std::remove_cvref_t<decltype(operation)>,
          BlotterCollection::RemoveOperation>) {
      for(auto& entry : m_entries) {
        auto block = shared_connection_block(entry.m_checked_connection);
        entry.m_checked->set(std::ranges::find(
          *m_links, entry.m_model) != m_links->cend());
      }
    });
}
