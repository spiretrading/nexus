#include "Spire/Ui/CalendarDatePicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListValueModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_header_label(QString text, QWidget* parent) {
    auto font = QFont("Roboto");
    font.setWeight(60);
    font.setPixelSize(scale_width(12));
    auto label = make_label(std::move(text), parent);
    label->setFixedSize(scale(24, 24));
    auto style = get_style(*label);
    style.get(Disabled() && ReadOnly()).
      set(Font(font)).
      set(TextAlign(Qt::AlignCenter)).
      set(TextColor(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    set_style(*label, std::move(style));
    return label;
  }

  auto make_day_header(QWidget* parent) {
    auto header = new QWidget(parent);
    header->setFocusPolicy(Qt::NoFocus);
    header->setFixedSize(scale(168, 26));
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

class CalendarListModel : public ListModel {
  public:
    static const int DAY_COUNT = 42;

    CalendarListModel(std::shared_ptr<DateModel> model)
        : m_model(std::move(model)),
          m_current_connection(m_model->connect_current_signal(
            [=] (auto current) { on_current(current); })) {
      on_current(m_model->get_current());
    }

    const std::shared_ptr<DateModel>& get_model() const {
      return m_model;
    }

    int get_size() const override {
      return DAY_COUNT;
    }

    const std::any& at(int index) const override {
      return m_dates[index];
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

  private:
    std::shared_ptr<DateModel> m_model;
    scoped_connection m_current_connection;
    std::array<std::any, DAY_COUNT> m_dates;
    ListModelTransactionLog m_transaction;

    void on_current(date current) {
      auto day = date(current.year(), current.month(), 1);
      if(day.day_of_week() != 0) {
        day += days(-day.day_of_week());
      }
      for(auto i = 0; i < DAY_COUNT; ++i) {
        m_dates[i] = day;
        m_transaction.push(UpdateOperation{i});
        day += days(1);
      }
    }
};

class RequiredDateModel : public DateModel {
  public:
    RequiredDateModel(std::shared_ptr<OptionalDateModel> model)
      : m_model(std::move(model)),
        m_current(m_model->get_current().value_or(day_clock::local_day())),
        m_current_connection(m_model->connect_current_signal(
          [=] (const auto& current) { on_current(current); })) {}

    const Type& get_current() const override {
      return m_current;
    }

    QValidator::State set_current(const date& value) override {
      m_current = value;
      m_current_signal(m_current);
      return QValidator::State::Acceptable;
    }

    connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override {
      return m_current_signal.connect(slot);
    }

  private:
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDateModel> m_model;
    scoped_connection m_current_connection;
    date m_current;

    void on_current(const boost::optional<date>& current) {
      if(current) {
        set_current(*current);
      }
    }
};

class MonthSpinner : public QWidget {
  public:
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

class CalendarDayLabel : public QWidget {
  public:
    CalendarDayLabel(std::shared_ptr<ListValueModel> model,
        std::shared_ptr<DateModel> month_model, QWidget* parent = nullptr)
        : QWidget(parent),
          m_model(std::move(model)),
          m_current_connection(m_model->connect_current_signal(
            [=] (const auto& current) {
              on_current(std::any_cast<date>(current));
            })),
          m_month_model(std::move(month_model)) {
      setFixedSize(scale(24, 24));
      // TODO: ToTextModel?
      m_label = make_label("", this);
      proxy_style(*this, *m_label);
      auto style = get_style(*m_label);
      style.get(Any()).
        set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
        set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
        set(border_radius(scale_width(3))).
        set(TextAlign(Qt::AlignCenter)).
        set(TextColor(QColor::fromRgb(0x000000))).
        set(padding(0));
      style.get(OutOfMonth() && !Disabled()).
        set(TextColor(QColor::fromRgb(0xA0A0A0)));
      style.get(Today() && !Disabled()).
        set(BackgroundColor(QColor::fromRgb(0xFFF2AB))).
        set(TextColor(QColor::fromRgb(0xDB8700)));
      style.get(Hover() || Press()).
        set(BackgroundColor(QColor::fromRgb(0xF2F2FF))).
        set(border_color(QColor::fromRgb(0, 0, 0, 0)));
      style.get(Focus()).
        set(border_color(QColor::fromRgb(0, 0, 0, 0)));
      style.get(Disabled()).
        set(BackgroundColor(QColor::fromRgb(0xFFFFFF))).
        set(border_color(QColor::fromRgb(0, 0, 0, 0))).
        set(TextColor(QColor::fromRgb(0xC8C8C8)));
      set_style(*this, std::move(style));
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->addWidget(m_label);
      on_current(std::any_cast<date>(m_model->get_current()));
    }

  private:
    std::shared_ptr<ListValueModel> m_model;
    scoped_connection m_current_connection;
    std::shared_ptr<DateModel> m_month_model;
    TextBox* m_label;

    void on_current(date day) {
      m_label->get_model()->set_current(QString("%12").arg(day.day()));
      if(day == day_clock::local_day()) {
        match(*this, Today());
      } else {
        unmatch(*this, Today());
      }
      if(day.month() != m_month_model->get_current().month()) {
        match(*this, OutOfMonth());
      } else {
        unmatch(*this, OutOfMonth());
      }
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
  auto month_spinner =
    new MonthSpinner(std::make_shared<RequiredDateModel>(m_model), this);
  layout->addWidget(month_spinner);
  layout->addWidget(make_day_header(this));
  auto calendar_view = new ListView(
    std::make_shared<CalendarListModel>(month_spinner->get_model()),
    [=] (const std::shared_ptr<ListModel>& model, int index) {
      return new CalendarDayLabel(
        std::make_shared<ListValueModel>(model, index),
        month_spinner->get_model());
    }, this);
  calendar_view->setFixedSize(scale(168, 144));
  setFocusProxy(calendar_view);
  month_spinner->setFocusProxy(calendar_view);
  calendar_view->installEventFilter(this);
  auto calendar_style = StyleSheet();
  calendar_style.get(Any()).
    set(Qt::Horizontal).
    set(EdgeNavigation(EdgeNavigation::CONTAIN)).
    set(Overflow(Overflow::WRAP));
  calendar_style.get(Any() >> is_a<ListItem>()).
    set(border_size(0)).
    set(padding(0));
  calendar_style.get(Any() >> (is_a<ListItem>() && Hover())).
    set(BackgroundColor(QColor::fromRgb(0xFFFFFF)));
  calendar_style.get(
      Any() >> (is_a<ListItem>() && Selected()) >> is_a<CalendarDayLabel>()).
    set(BackgroundColor(QColor::fromRgb(0x4B23A0))).
    set(border(0, QColor::fromRgb(0, 0, 0, 0))).
    set(TextColor(QColor::fromRgb(0xFFFFFF)));
  set_style(*calendar_view, std::move(calendar_style));
  layout->addWidget(calendar_view);
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_model() const {
  return m_model;
}

connection CalendarDatePicker::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
