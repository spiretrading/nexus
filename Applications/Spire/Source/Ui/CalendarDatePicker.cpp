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
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;
using DateModel = ValueModel<date>;
using LocalDateModel = LocalValueModel<date>;
using MonthModel = LocalValueModel<date>;

namespace {
  const auto DISPLAYED_DAYS = 42;

  class CalendarDayButton : public QWidget {
    public:
      CalendarDayButton(std::shared_ptr<DateModel> model,
          std::shared_ptr<MonthModel> month_model, QWidget* parent = nullptr)
          : QWidget(parent),
            m_model(std::move(model)),
            m_month_model(std::move(month_model)) {
        setFixedSize(scale(24, 24));
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins({});
        m_button = make_label_button("", this);
        proxy_style(*this, *m_button);
        auto style = get_style(*m_button);
        style.get(Any() / Body()).
          set(BackgroundColor(QColor::fromRgb(0xFFFFFF))).
          set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
          set(border_radius(scale_width(3))).
          set(TextColor(QColor::fromRgb(0x000000)));
        style.get(OutOfMonth() / Body()).
          set(TextColor(QColor::fromRgb(0xA0A0A0)));
        style.get(Today() / Body()).
          set(BackgroundColor(QColor::fromRgb(0xFFF2AB))).
          set(TextColor(QColor::fromRgb(0xDB8700)));
        style.get((Hover() || Press()) / Body()).
          set(BackgroundColor(QColor::fromRgb(0xF2F2FF)));
        style.get(Disabled() / Body()).
          set(TextColor(QColor::fromRgb(0xC8C8C8)));
        set_style(*m_button, std::move(style));
        layout->addWidget(m_button);
        on_current(m_model->get_current());
        m_model->connect_current_signal([=] (auto day) { on_current(day); });
      }

    private:
      std::shared_ptr<DateModel> m_model;
      std::shared_ptr<MonthModel> m_month_model;
      // TODO: should this just be a label; isn't the ListItem a button?
      Button* m_button;

      void on_current(date day) {
        static_cast<TextBox*>(&m_button->get_body())->get_model()->set_current(
          QString("%12").arg(day.day()));
        if(day == day_clock::local_day()) {
          match(*this, Today());
        } else {
          unmatch(*this, Today());
        }
        if(day.month() == m_month_model->get_current().month()) {
          match (*this, OutOfMonth());
        } else {
          unmatch(*this, OutOfMonth());
        }
      }
  };

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
    // TODO: use boost day names?
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
    MonthSelector(std::shared_ptr<OptionalDateModel> date_model,
        QWidget* parent = nullptr)
        : m_date_model(std::move(date_model)) {
       m_model = std::make_shared<MonthModel>(
            m_date_model->get_current().value_or(day_clock::local_day()));
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->setSpacing(0);
      layout->addSpacing(scale_width(4));
      m_previous_button = make_icon_button(
        imageFromSvg(":Icons/calendar-arrow-left.svg", scale(16, 16)));
      m_previous_button->setFixedSize(scale(16, 16));
      m_previous_button->connect_clicked_signal([=] { decrement(); });
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
      m_next_button->connect_clicked_signal([=] { increment(); });
      layout->addWidget(m_next_button);
      layout->addSpacing(scale_width(4));
      m_date_model->connect_current_signal(
        [=] (const auto& date) { on_current(date); });
      auto initial_date = [&] {
        auto current = m_date_model->get_current();
        if(current) {
          return *current;
        }
        return day_clock::local_day();
      }();
      on_current(initial_date);
    }

    std::shared_ptr<MonthModel> get_model() const {
      return m_model;
    }

  private:
    std::shared_ptr<MonthModel> m_model;
    std::shared_ptr<OptionalDateModel> m_date_model;
    // TODO: use boost month names?
    QLocale m_locale;
    Button* m_previous_button;
    TextBox* m_label;
    Button* m_next_button;

    void decrement() {
      m_model->set_current(m_model->get_current() - months(1));
      update_label();
    }

    void increment() {
      m_model->set_current(m_model->get_current() + months(1));
      update_label();
    }

    QString get_month_name(int month) const {
      return m_locale.monthName(month);
    }

    void update_label() {
      m_label->get_model()->set_current(QString("%1 %2").
        arg(get_month_name(m_model->get_current().month())).
        arg(m_model->get_current().year()));
    }

    void on_current(const boost::optional<date>& date) {
      if(date) {
        m_model->set_current(*date);
        update_label();
      }
    }
};

CalendarDatePicker::CalendarDatePicker(QWidget* parent)
  : CalendarDatePicker(std::make_shared<LocalOptionalDateModel>(
      day_clock::local_day()), parent) {}

CalendarDatePicker::CalendarDatePicker(date current, QWidget* parent)
  : CalendarDatePicker(
    std::make_shared<LocalOptionalDateModel>(current), parent) {}

CalendarDatePicker::CalendarDatePicker(
    std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_calendar_view(nullptr),
      m_calendar_model(std::make_shared<ArrayListModel>()) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(4), scale_width(8), scale_width(4),
    scale_height(4));
  layout->setSpacing(scale_height(4));
  m_month_selector = new MonthSelector(m_model, this);
  m_month_selector->get_model()->connect_current_signal(
    [=] (auto current) { update_calendar_model(); });
  layout->addWidget(m_month_selector);
  auto header = make_day_header(this);
  layout->addWidget(header);
  populate_calendar([=] (auto index, auto day) {
    m_calendar_model->push(std::make_shared<LocalDateModel>(day));
  });
  m_calendar_view = new ListView(m_calendar_model,
    [=] (const ArrayListModel& model, int index) {
      return new CalendarDayButton(
        model.get<std::shared_ptr<LocalDateModel>>(index),
        m_month_selector->get_model());
    }, this);
  m_calendar_view->setFixedWidth(scale_width(182));
  auto calendar_style = get_style(*m_calendar_view);
  //auto calendar_style = StyleSheet();
  calendar_style.get(Any()).
    set(Qt::Horizontal).
    set(Overflow(Overflow::WRAP));
  set_style(*m_calendar_view, std::move(calendar_style));
  update_calendar_model();
  m_model->connect_current_signal([=] (const auto& day) { on_current(day); });
  m_calendar_view->get_selection_model()->connect_current_signal(
    [=] (const auto& index) {
      if(index) {
        auto current = m_calendar_model->
          get<std::shared_ptr<LocalDateModel>>(*index)->get_current();
        m_model->set_current(current);
        m_submit_signal(current);
      }
    });
  layout->addWidget(m_calendar_view);
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_model() const {
  return m_model;
}

connection CalendarDatePicker::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void CalendarDatePicker::populate_calendar(const std::function<
    void (int index, boost::gregorian::date day)> assign) {
  auto displayed_month = m_month_selector->get_model()->get_current();
  auto day = date(displayed_month.year(), displayed_month.month(), 1);
  if(day.day_of_week() != 0) {
    day += days(-day.day_of_week());
  }
  for(auto i = 0; i < DISPLAYED_DAYS; ++i) {
    assign(i, day);
    day += days(1);
  }
}

void CalendarDatePicker::update_calendar_model() {
  populate_calendar([=] (auto index, auto day) {
    m_calendar_model->
      get<std::shared_ptr<LocalDateModel>>(index)->set_current(day);
  });
}

void CalendarDatePicker::on_current(const boost::optional<date>& day) {
  if(day) {
    auto display = m_month_selector->get_model()->get_current();
    if(display.month() != day->month() || display.year() != day->year()) {
      update_calendar_model();
    }
  }
}
