#include "Spire/Ui/CalendarDatePicker.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::gregorian;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DISPLAYED_DAYS = 42;

  auto date_button_builder(const ArrayListModel& model, int index) {
    // TODO:
    //auto button = make_label_button(
    //  QString("%1").arg(model.get<date>(index).month()));
    auto button = make_label_button(QString("%1").arg(index));
    button->setFixedSize(scale(24, 24));
    auto style = get_style(*button);
    style.get(Body()).
      set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
      set(border_radius(scale_width(3))).
      set(TextAlign(Qt::AlignCenter)).
      set(padding(0));
    set_style(*button, std::move(style));
    return button;
  }

  auto make_header_label(QString text, QWidget* parent) {
    auto label = make_label(text, parent);
    label->setFixedSize(scale(24, 24));
    auto style = get_style(*label);
    auto font = QFont("Roboto");
    font.setWeight(60);
    font.setPixelSize(scale_width(12));
    style.get(Disabled() && ReadOnly()).
      set(Font(font)).
      set(TextAlign(Qt::AlignCenter)).
      set(TextColor(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    set_style(*label, std::move(style));
    return label;
  }

  auto make_day_header(QWidget* parent) {
    auto header = new QWidget(parent);
    auto layout = new QHBoxLayout(header);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    auto locale = QLocale();
    layout->addWidget(make_header_label(locale.dayName(7).at(0), header));
    for(auto i = 1; i < 7; ++i) {
      layout->addWidget(make_header_label(locale.dayName(i).at(0), header));
    }
    return header;
  }
}

class CalendarDatePicker::MonthSelector : public QWidget {
  public:
    MonthSelector(std::shared_ptr<DateModel> model,
        QWidget* parent = nullptr)
        : m_model(std::move(model)) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->setSpacing(0);
      layout->addSpacing(scale_width(4));
      m_previous_button = make_icon_button(
        imageFromSvg(":Icons/calendar-arrow-left.svg", scale(16, 16)));
      m_previous_button->setFixedSize(scale(16, 16));
      m_previous_button->connect_clicked_signal([=] { adjust_month(-1); });
      layout->addWidget(m_previous_button);
      layout->addSpacing(scale_width(8));
      m_label = make_label("", this);
      auto label_style = get_style(*m_label);
      label_style.get(Disabled() && ReadOnly()).
        set(TextAlign(Qt::AlignCenter));
      set_style(*m_label, std::move(label_style));
      layout->addWidget(m_label);
      layout->addSpacing(scale_width(8));
      m_next_button = make_icon_button(
        imageFromSvg(":Icons/calendar-arrow-right.svg", scale(16, 16)));
      m_next_button->setFixedSize(scale(16, 16));
      m_next_button->connect_clicked_signal([=] { adjust_month(1); });
      layout->addWidget(m_next_button);
      layout->addSpacing(scale_width(4));
      m_model->connect_current_signal(
        [=] (const auto& date) { on_current(date); });
      auto initial_date = [&] {
        auto current = m_model->get_current();
        if(current) {
          return *current;
        }
        return day_clock::local_day();
      }();
      on_current(initial_date);
    }

  private:
    std::shared_ptr<DateModel> m_model;
    int m_displayed_month;
    int m_displayed_year;
    QLocale m_locale;
    Button* m_previous_button;
    TextBox* m_label;
    Button* m_next_button;

    void adjust_month(int adjustment) {
      auto adjusted_month = m_displayed_month + adjustment;
      if(adjusted_month > 12) {
        m_displayed_month = 1;
        ++m_displayed_year;
      } else if(adjusted_month < 1) {
        m_displayed_month = 12;
        --m_displayed_year;
      } else {
        m_displayed_month = adjusted_month;
      }
      update_label();
    }

    QString get_month_name(int month) const {
      return m_locale.monthName(month);
    }

    void update_label() {
      m_label->get_model()->set_current(QString("%1 %2").
        arg(get_month_name(m_displayed_month)).arg(m_displayed_year));
    }

    void on_current(const boost::optional<date>& date) {
      if(date) {
        m_displayed_month = date->month();
        m_displayed_year = date->year();
        update_label();
      }
    }
};

CalendarDatePicker::CalendarDatePicker(QWidget* parent)
  : CalendarDatePicker(std::make_shared<LocalDateModel>(
      day_clock::local_day()), parent) {}

CalendarDatePicker::CalendarDatePicker(date current, QWidget* parent)
  : CalendarDatePicker(std::make_shared<LocalDateModel>(current), parent) {}

CalendarDatePicker::CalendarDatePicker(std::shared_ptr<DateModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_calendar_view(nullptr),
      m_calendar_model(std::make_shared<ArrayListModel>()) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(4), scale_width(8), scale_width(4),
    scale_height(4));
  layout->setSpacing(scale_height(4));
  m_month_selector = new MonthSelector(m_model, this);
  layout->addWidget(m_month_selector);
  auto header = make_day_header(this);
  layout->addWidget(header);
  create_calendar_model();
  m_calendar_view = new ListView(m_calendar_model, date_button_builder, this);
  // TODO: fixed height only
  m_calendar_view->setFixedSize(scale(scale(182, 144)));
  auto calendar_style = get_style(*m_calendar_view);
  calendar_style.get(Any()).
    set(Qt::Horizontal).
    set(Overflow(Overflow::WRAP));
  set_style(*m_calendar_view, std::move(calendar_style));
  m_calendar_model->connect_operation_signal(
    [=] (const auto& operation) {
      visit(operation,
        [&] (const ListModel::UpdateOperation& operation) {
          auto item = m_calendar_view->get_list_item(operation.m_index);
          item->setFixedSize(scale(24, 24));
          auto style = StyleSheet();
          style.get(Any()).
            set(BackgroundColor(QColor::fromRgb(0xFF, 0x0, 0x0))).
            set(padding(0));
          set_style(*item, std::move(style));
        });
    });
  update_calendar_model();
  layout->addWidget(m_calendar_view);
}

const std::shared_ptr<DateModel>& CalendarDatePicker::get_model() const {
  return m_model;
}

void CalendarDatePicker::create_calendar_model() {
  m_calendar_model = std::make_shared<ArrayListModel>();
  for(auto i = 0; i < DISPLAYED_DAYS; ++i) {
    m_calendar_model->push(date());
  }
}

void CalendarDatePicker::update_calendar_model() {
  for(auto i = 0; i < DISPLAYED_DAYS; ++i) {
    m_calendar_model->set(i, m_model->get_current());
  }
}

void CalendarDatePicker::on_current(const boost::optional<date>& date) {
  update_calendar_model();
}
