#include "Spire/Ui/CalendarDatePicker.hpp"
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

class CalendarDatePicker::MonthSpinner : public QWidget {
  public:
    explicit MonthSpinner(QWidget* parent = nullptr)
      : MonthSpinner(
          std::make_shared<LocalDateModel>(day_clock::local_day()), parent) {}

    explicit MonthSpinner(date date, QWidget* parent = nullptr)
      : MonthSpinner(std::make_shared<LocalDateModel>(date), parent) {}

    explicit MonthSpinner(
        std::shared_ptr<DateModel> model, QWidget* parent = nullptr)
        : QWidget(parent),
          m_model(std::move(model)) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({scale_width(4), 0, scale_width(4), 0});
      layout->setSpacing(scale_width(8));
      const auto BUTTON_SIZE = scale(16, 16);
      m_previous_button = make_icon_button(
        imageFromSvg(":Icons/calendar-arrow-left.svg", BUTTON_SIZE));
      m_previous_button->setFixedSize(BUTTON_SIZE);
      m_previous_button->connect_clicked_signal([=] { decrement(); });
      layout->addWidget(m_previous_button);
      m_label = make_label(std::make_shared<ToTextModel<date>>(m_model,
        [] (const date& current) {
          return QString("%1 %2").
            arg(current.month().as_long_string()).arg(current.year());
        }), this);
      auto label_style = get_style(*m_label);
      label_style.get(Disabled() && ReadOnly()).
        set(TextAlign(Qt::AlignCenter));
      set_style(*m_label, std::move(label_style));
      layout->addWidget(m_label);
      m_next_button = make_icon_button(
        imageFromSvg(":Icons/calendar-arrow-right.svg", BUTTON_SIZE));
      m_next_button->setFixedSize(BUTTON_SIZE);
      m_next_button->connect_clicked_signal([=] { increment(); });
      layout->addWidget(m_next_button);
    }

    const std::shared_ptr<DateModel>& get_model() const {
      return m_model;
    }

  private:
    std::shared_ptr<DateModel> m_model;
    TextBox* m_label;
    Button* m_previous_button;
    Button* m_next_button;

    void decrement() {
      m_model->set_current(m_model->get_current() - months(1));
    }
    void increment() {
      m_model->set_current(m_model->get_current() + months(1));
    }
};

CalendarDatePicker::CalendarDatePicker(QWidget* parent)
  : CalendarDatePicker(
      std::make_shared<LocalOptionalDateModel>(day_clock::local_day()),
      parent) {}

CalendarDatePicker::CalendarDatePicker(date current, QWidget* parent)
  : CalendarDatePicker(
      std::make_shared<LocalOptionalDateModel>(current), parent) {}

CalendarDatePicker::CalendarDatePicker(
    std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(
    scale_width(4), scale_height(8), scale_width(4), scale_height(4));
  layout->setSpacing(scale_height(4));
  setFixedWidth(scale_width(176));
  m_month_spinner = new MonthSpinner(std::make_shared<LocalDateModel>(
    m_model->get_current().get_value_or(day_clock::local_day())), this);
  layout->addWidget(m_month_spinner);
  m_model->connect_current_signal([=] (const auto& current) {
    on_current(current);
  });
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_model() const {
  return m_model;
}

connection CalendarDatePicker::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void CalendarDatePicker::on_current(const boost::optional<date>& current) {
  if(current) {
    m_month_spinner->get_model()->set_current(*current);
  }
}
