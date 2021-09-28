#include "Spire/Ui/MonthSpinner.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::gregorian;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }
}

MonthSpinner::MonthSpinner(QWidget* parent)
  : MonthSpinner(
      std::make_shared<LocalDateModel>(day_clock::local_day()), parent) {}

MonthSpinner::MonthSpinner(date date, QWidget* parent)
  : MonthSpinner(std::make_shared<LocalDateModel>(date), parent) {}

MonthSpinner::MonthSpinner(std::shared_ptr<DateModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({scale_width(4), 0, scale_width(4), 0});
  layout->setSpacing(scale_width(8));
  m_previous_button = make_icon_button(
    imageFromSvg(":Icons/calendar-arrow-left.svg", BUTTON_SIZE()));
  m_previous_button->setFixedSize(BUTTON_SIZE());
  m_previous_button->connect_clicked_signal([=] { decrement(); });
  layout->addWidget(m_previous_button);
  m_label = make_label("", this);
  auto label_style = get_style(*m_label);
  label_style.get(Disabled() && ReadOnly()).
    set(TextAlign(Qt::AlignCenter));
  set_style(*m_label, std::move(label_style));
  layout->addWidget(m_label);
  m_next_button = make_icon_button(
    imageFromSvg(":Icons/calendar-arrow-right.svg", BUTTON_SIZE()));
  m_next_button->setFixedSize(BUTTON_SIZE());
  m_next_button->connect_clicked_signal([=] { increment(); });
  layout->addWidget(m_next_button);
  m_model->connect_current_signal([=] (auto date) { on_current(date); });
  on_current(m_model->get_current());
}

const std::shared_ptr<DateModel>& MonthSpinner::get_model() const {
  return m_model;
}

void MonthSpinner::decrement() {
  m_model->set_current(m_model->get_current() - months(1));
}

void MonthSpinner::increment() {
  m_model->set_current(m_model->get_current() + months(1));
}

void MonthSpinner::update_label() {
  m_label->get_model()->set_current(QString("%1 %2").
    arg(m_model->get_current().month().as_long_string()).
    arg(m_model->get_current().year()));
}

void MonthSpinner::on_current(date date) {
  update_label();
}
