#include "Spire/KeyBindings/AdditionalTagValueBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

AdditionalTagValueBox::AdditionalTagValueBox(
    std::shared_ptr<AdditionalTagValueModel> current,
    std::shared_ptr<AdditionalTagSchemaModel> schema, QWidget* parent)
    : m_current(std::move(current)),
      m_schema(std::move(schema)) {
  make_vbox_layout(this);
  m_schema_connection = m_schema->connect_update_signal(
    std::bind_front(&AdditionalTagValueBox::on_schema, this));
  update_schema(m_schema->get(), false);
}

const std::shared_ptr<AdditionalTagValueModel>&
    AdditionalTagValueBox::get_current() const {
  return m_current;
}

bool AdditionalTagValueBox::is_read_only() const {
  return get_input_box().is_read_only();
}

void AdditionalTagValueBox::set_read_only(bool is_read_only) {
  get_input_box().set_read_only(is_read_only);
}

connection AdditionalTagValueBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

AnyInputBox& AdditionalTagValueBox::get_input_box() {
  auto box = layout()->itemAt(0)->widget();
  return static_cast<AnyInputBox&>(*box);
}

const AnyInputBox& AdditionalTagValueBox::get_input_box() const {
  auto box = layout()->itemAt(0)->widget();
  return static_cast<AnyInputBox&>(*box);
}

void AdditionalTagValueBox::update_schema(
    const std::shared_ptr<AdditionalTagSchema>& schema, bool is_read_only) {
  auto box = schema->make_input_box(m_current);
  box->set_read_only(is_read_only);
  layout()->addWidget(box);
  proxy_style(*this, *box);
  setFocusProxy(box);
}

void AdditionalTagValueBox::on_schema(
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  auto is_read_only = this->is_read_only();
  auto item = layout()->takeAt(0);
  delete item->widget();
  delete item;
  m_current->set(none);
  update_schema(schema, is_read_only);
}
