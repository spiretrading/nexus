#include "Spire/Ui/DateBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/IntegerBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_body() {
  }
}

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto input_box = make_input_box(make_body());
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(input_box);
}

const std::shared_ptr<OptionalDateModel>& DateBox::get_model() const {
  return m_model;
}

connection DateBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection DateBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}
