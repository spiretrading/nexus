#include "Spire/UiViewer/DateInputTestWidget.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto INPUT_SIZE() {
    static auto size = scale(52, 26);
    return size;
  }
}

DateInputTestWidget::DateInputTestWidget(QWidget* parent)
    : QWidget(parent),
      m_date_input(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 2);
  m_year_input = new TextInputWidget("1970", this);
  m_year_input->setPlaceholderText(tr("Year"));
  m_year_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_year_input, 1, 0);
  m_month_input = new TextInputWidget("1", this);
  m_month_input->setPlaceholderText(tr("Month"));
  m_month_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_month_input, 1, 1);
  m_day_input = new TextInputWidget("1", this);
  m_day_input->setPlaceholderText(tr("Day"));
  m_day_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_day_input, 1, 2);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedHeight(scale_height(26));
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 2, 0, 1, 3);
  on_reset_button();
}

void DateInputTestWidget::on_reset_button() {
  auto year_ok = false;
  auto year = m_year_input->text().toUShort(&year_ok);
  auto month_ok = false;
  auto month = m_month_input->text().toUShort(&month_ok);
  auto day_ok = false;
  auto day = m_day_input->text().toUShort(&day_ok);
  try {
    if(year_ok && month_ok && day_ok) {
      auto date = boost::posix_time::ptime({year, month, day});
      delete_later(m_date_input);
      m_date_input = new DateInputWidget(date, this);
      m_date_input->setFixedSize(CONTROL_SIZE());
      m_layout->addWidget(m_date_input, 0, 0);
      m_status_label->setText("");
      return;
    }
  } catch(const std::exception&) {}
  m_status_label->setText(tr("Invalid input."));
}
