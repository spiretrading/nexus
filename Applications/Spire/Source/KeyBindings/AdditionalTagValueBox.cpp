#include "Spire/KeyBindings/AdditionalTagValueBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

AdditionalTagValueBox::AdditionalTagValueBox(
    std::shared_ptr<AdditionalTagValueModel> current,
    std::shared_ptr<AdditionalTagSchemaModel> schema, QWidget* parent)
    : m_current(std::move(current)),
      m_schema(std::move(schema)) {
  make_vbox_layout(this);
  m_schema_connection = m_schema->connect_update_signal(
    std::bind_front(&AdditionalTagValueBox::on_schema, this));
  on_schema(m_schema->get());
}

const std::shared_ptr<AdditionalTagValueModel>&
    AdditionalTagValueBox::get_current() const {
  return m_current;
}

bool AdditionalTagValueBox::is_read_only() const {
  return false;
}

void AdditionalTagValueBox::set_read_only(bool is_read_only) {}

connection AdditionalTagValueBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void AdditionalTagValueBox::on_schema(
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  if(auto item = layout()->takeAt(0)) {
    item->widget()->deleteLater();
    delete item;
  }
  auto box = schema->make_input_box(m_current);
  layout()->addWidget(box);
}
