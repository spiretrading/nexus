#include "Spire/Ui/SecurityBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/SecurityListItem.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityBox::SecurityBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : SecurityBox(std::move(query_model),
      std::make_shared<LocalValueModel<Security>>(), parent) {}

SecurityBox::SecurityBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  auto combo_box_current = make_transform_value_model(m_current,
    [] (const Security& current) {
      return std::any(current);
    },
    [] (const std::any& current) {
      return std::any_cast<SecurityInfo>(current).m_security;
    });
  m_combo_box = new ComboBox(std::move(query_model), combo_box_current,
    [] (const auto& list, auto index) {
      return new SecurityListItem(std::any_cast<SecurityInfo>(list->at(index)));
    });
  m_combo_box->connect_submit_signal([=] (const auto& submission) {
    m_submit_signal(std::any_cast<SecurityInfo>(submission).m_security);
  });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(m_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    SecurityBox::get_query_model() const {
  return m_combo_box->get_query_model();
}

const std::shared_ptr<SecurityBox::CurrentModel>&
    SecurityBox::get_current() const {
  return m_current;
}

const Security& SecurityBox::get_submission() const {
  return m_submission;
}

bool SecurityBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void SecurityBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection SecurityBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
