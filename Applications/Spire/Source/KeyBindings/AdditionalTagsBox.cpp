#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

AdditionalTagsBox::AdditionalTagsBox(
    std::shared_ptr<AdditionalTagsModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_is_read_only(false) {
  m_tags_text = make_transform_value_model(m_current,
    [] (const auto& current) {
      return QString();
    });
  m_label = new TextBox(m_tags_text);
  m_label->set_read_only(true);
  enclose(*this, *m_label);
  proxy_style(*this, *m_label);
}

const std::shared_ptr<AdditionalTagsModel>&
    AdditionalTagsBox::get_current() const {
  return m_current;
}

void AdditionalTagsBox::set_placeholder(const QString& placeholder) {
  m_label->set_placeholder(placeholder);
}

bool AdditionalTagsBox::is_read_only() const {
  return m_is_read_only;
}

void AdditionalTagsBox::set_read_only(bool is_read_only) {
  if(is_read_only == m_is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
}

connection AdditionalTagsBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return {};
}
