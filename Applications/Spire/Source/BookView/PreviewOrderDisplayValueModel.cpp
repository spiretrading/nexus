#include "Spire/BookView/PreviewOrderDisplayValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

PreviewOrderDisplayValueModel::PreviewOrderDisplayValueModel(
    std::shared_ptr<BookViewModel::PreviewOrderModel> preview,
    std::shared_ptr<BookViewPropertiesModel> properties)
    : m_preview(std::move(preview)),
      m_properties(std::move(properties)),
      m_has_preview(m_preview->get().has_value()),
      m_is_displayed(false) {
  on_properties(m_properties->get());
  m_preview_connection = m_preview->connect_update_signal(
    std::bind_front(&PreviewOrderDisplayValueModel::on_preview, this));
  m_properties_connection = m_properties->connect_update_signal(
    std::bind_front(&PreviewOrderDisplayValueModel::on_properties, this));
}

const PreviewOrderDisplayValueModel::Type&
    PreviewOrderDisplayValueModel::get() const {
  if(m_is_displayed) {
    return m_preview->get();
  }
  static const auto NONE = optional<OrderFields>();
  return NONE;
}

connection PreviewOrderDisplayValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void PreviewOrderDisplayValueModel::on_preview(const Type& preview) {
  m_has_preview = preview.has_value();
  if(m_is_displayed) {
    m_update_signal(preview);
  }
}

void PreviewOrderDisplayValueModel::on_properties(
    const BookViewProperties& properties) {
  auto is_displayed = properties.m_highlight_properties.m_order_visibility !=
    BookViewHighlightProperties::OrderVisibility::HIDDEN;
  if(is_displayed == m_is_displayed) {
    return;
  }
  m_is_displayed = is_displayed;
  if(m_has_preview) {
    if(m_is_displayed) {
      m_update_signal(m_preview->get());
    } else{
      m_update_signal(none);
    }
  }
}
