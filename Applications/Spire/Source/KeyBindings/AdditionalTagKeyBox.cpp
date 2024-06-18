#include "Spire/KeyBindings/AdditionalTagKeyBox.hpp"
#include "Spire/Spire/ListIndexValueModel.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

AdditionalTagKeyBox::AdditionalTagKeyBox(
    std::shared_ptr<AdditionalTagKeyModel> current,
    std::shared_ptr<ListModel<int>> available_tags,
    AdditionalTagDatabase additional_tags,
    std::shared_ptr<DestinationModel> destination,
    std::shared_ptr<RegionModel> region, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_available_tags(std::move(available_tags)),
      m_additional_tags(std::move(additional_tags)),
      m_destination(std::move(destination)),
      m_region(std::move(region)) {
  m_drop_down_box = new DropDownBox(m_available_tags,
    std::make_shared<ListIndexValueModel<int>>(m_available_tags, m_current),
    std::bind_front(&AdditionalTagKeyBox::make_key_item, this),
    std::bind_front(&AdditionalTagKeyBox::key_to_text, this));
  enclose(*this, *m_drop_down_box);
  proxy_style(*this, *m_drop_down_box);
}

const std::shared_ptr<AdditionalTagKeyModel>&
    AdditionalTagKeyBox::get_current() const {
  return m_current;
}

bool AdditionalTagKeyBox::is_read_only() const {
  return m_drop_down_box->is_read_only();
}

void AdditionalTagKeyBox::set_read_only(bool is_read_only) {
  m_drop_down_box->set_read_only(is_read_only);
}

connection AdditionalTagKeyBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_drop_down_box->connect_submit_signal([=] (const auto& submission) {
    slot(std::any_cast<int>(submission));
  });
}

QWidget* AdditionalTagKeyBox::make_key_item(
    const std::shared_ptr<ListModel<int>>& available_tags, int index) const {
  auto schema = Spire::find(m_additional_tags, m_destination->get(),
    m_region->get(), available_tags->get(index));
  if(!schema) {
    throw std::runtime_error("Schema not found.");
  }
  return make_label(QString::fromStdString(schema->get_name()));
}

QString AdditionalTagKeyBox::key_to_text(int key) const {
  auto schema = Spire::find(
    m_additional_tags, m_destination->get(), m_region->get(), key);
  if(!schema) {
    throw std::runtime_error("Schema not found.");
  }
  return QString::fromStdString(schema->get_name());
}
