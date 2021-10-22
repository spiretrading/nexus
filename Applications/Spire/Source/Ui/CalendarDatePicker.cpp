#include "Spire/Ui/CalendarDatePicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QCoreApplication>
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
  const auto CALENDAR_DAY_COUNT = 42;

  std::array<date, CALENDAR_DAY_COUNT> get_calendar_dates(date day) {
    day = date(day.year(), day.month(), 1);
    if(day.day_of_week() != 0) {
      day += days(-day.day_of_week());
    }
    auto dates = std::array<date, CALENDAR_DAY_COUNT>();
    for(auto i = 0; i < CALENDAR_DAY_COUNT; ++i) {
      dates[i] = day + days(i);
    }
    return dates;
  }

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
      set(TextColor(QColor(0x4B23A0)));
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
      return m_dates.size();
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
    std::array<std::any, CALENDAR_DAY_COUNT> m_dates;
    ListModelTransactionLog m_transaction;

    void on_current(date current) {
      auto dates = get_calendar_dates(current);
      for(auto i = std::size_t(0); i < dates.size(); ++i) {
        m_dates[i] = dates[i];
      }
      m_transaction.transact([&] {
        for(auto i = 0; i < static_cast<int>(m_dates.size()); ++i) {
          m_transaction.push(UpdateOperation{i});
        }
      });
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

    void on_current(const optional<date>& current) {
      if(current) {
        set_current(*current);
      }
    }
};

class CalendarDatePicker::MonthSpinner : public QWidget {
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
      m_label = make_label("", this);
      proxy_style(*this, *m_label);
      auto style = get_style(*m_label);
      style.get(Any()).
        set(BackgroundColor(QColor(Qt::transparent))).
        set(border(scale_width(1), QColor(Qt::transparent))).
        set(border_radius(scale_width(3))).
        set(TextAlign(Qt::AlignCenter)).
        set(TextColor(QColor(Qt::black))).
        set(padding(0));
      style.get(OutOfMonth() && !Disabled()).
        set(TextColor(QColor(0xA0A0A0)));
      style.get(Today() && !Disabled()).
        set(BackgroundColor(QColor(0xFFF2AB))).
        set(TextColor(QColor(0xDB8700)));
      style.get(Hover() || Press()).
        set(BackgroundColor(QColor(0xF2F2FF))).
        set(border_color(QColor(Qt::transparent)));
      style.get(Focus()).
        set(border_color(QColor(Qt::transparent)));
      style.get(Disabled()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(border_color(QColor(Qt::transparent))).
        set(TextColor(QColor(0xC8C8C8)));
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
      m_label->get_model()->set_current(QString("%1").arg(day.day()));
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
  m_current_connection = m_model->connect_current_signal([=] (auto current) {
    on_current(current);
  });
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(
    scale_width(4), scale_height(8), scale_width(4), scale_height(4));
  layout->setSpacing(scale_height(4));
  m_month_spinner =
    new MonthSpinner(std::make_shared<RequiredDateModel>(m_model), this);
  m_month_spinner->installEventFilter(this);
  layout->addWidget(m_month_spinner);
  layout->addWidget(make_day_header(this));
  m_calendar_view = new ListView(
    std::make_shared<CalendarListModel>(m_month_spinner->get_model()),
    [=] (const std::shared_ptr<ListModel>& model, int index) {
      return new CalendarDayLabel(
        std::make_shared<ListValueModel>(model, index),
        m_month_spinner->get_model());
    }, this);
  m_month_spinner->get_model()->connect_current_signal([=] (auto current) {
    on_current_month(current);
  });
  m_calendar_view->setFixedSize(scale(168, 144));
  m_month_spinner->setFocusProxy(m_calendar_view);
  m_calendar_view->installEventFilter(this);
  layout->addWidget(m_calendar_view);
  m_list_current_connection =
    m_calendar_view->get_current_model()->connect_current_signal(
      [=] (const auto& index) { on_list_current(index); });
  m_calendar_view->connect_submit_signal([=] (const auto& value) {
    on_submit(std::any_cast<date>(value));
  });
  auto calendar_style = StyleSheet();
  calendar_style.get(Any()).
    set(Qt::Horizontal).
    set(EdgeNavigation(EdgeNavigation::CONTAIN)).
    set(Overflow(Overflow::WRAP));
  calendar_style.get(Any() >> is_a<ListItem>()).
    set(border_size(0)).
    set(padding(0));
  calendar_style.get(Any() >> (is_a<ListItem>() && Hover())).
    set(BackgroundColor(QColor(0xFFFFFF)));
  calendar_style.get(
      Any() >> (is_a<ListItem>() && Selected()) >> is_a<CalendarDayLabel>()).
    set(BackgroundColor(QColor(0x4B23A0))).
    set(border(0, QColor(Qt::transparent))).
    set(TextColor(QColor(0xFFFFFF)));
  set_style(*m_calendar_view, std::move(calendar_style));
  setFocusProxy(m_calendar_view);
  on_current(m_model->get_current());
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_model() const {
  return m_model;
}

connection CalendarDatePicker::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool CalendarDatePicker::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(watched == m_month_spinner &&
        (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)) {
      QCoreApplication::sendEvent(m_calendar_view, e);
    } else {
      auto current_index = m_calendar_view->get_current_model()->get_current();
      if(current_index) {
        if(*current_index == 0 && e->key() == Qt::Key_Left) {
          m_model->set_current(*m_model->get_current() - days(1));
          return true;
        } else if(*current_index ==
            m_calendar_view->get_list_model()->get_size() - 1 &&
            e->key() == Qt::Key_Right) {
          m_model->set_current(*m_model->get_current() + days(1));
          return true;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

boost::optional<int> CalendarDatePicker::get_index(date day) const {
  auto first_date = m_calendar_view->get_list_model()->get<date>(0);
  auto last_date =  m_calendar_view->get_list_model()->get<date>(
    m_calendar_view->get_list_model()->get_size() - 1);
  if(day < first_date || last_date < day) {
    return {};
  }
  return date_duration(day - first_date).days();
}

void CalendarDatePicker::set_current_index(const optional<int>& index) {
  auto current_block =
    shared_connection_block(m_list_current_connection);
  m_calendar_view->get_current_model()->set_current(index);
}

void CalendarDatePicker::on_current(const optional<date>& current) {
  if(current) {
    set_current_index(get_index(*current));
  } else {
    set_current_index({});
  }
}

void CalendarDatePicker::on_current_month(date month) {
  auto current_index = optional<int>();
  auto list_has_focus = m_calendar_view->hasFocus() ||
    m_calendar_view->isAncestorOf(focusWidget());
  auto minimum = m_model->get_minimum();
  auto maximum = m_model->get_maximum();
  for(auto i = 0; i < m_calendar_view->get_list_model()->get_size(); ++i) {
    auto current = m_calendar_view->get_list_model()->get<date>(i);
    if(current < *minimum || current > *maximum) {
      auto item = m_calendar_view->get_list_item(i);
      if(item->hasFocus()) {
        m_calendar_view->setFocusProxy(nullptr);
        m_calendar_view->setFocus();
      }
      item->setDisabled(true);
    }
    if(current == m_model->get_current()) {
      current_index = i;
    }
  }
  if(current_index != m_calendar_view->get_current_model()->get_current()) {
    set_current_index(current_index);
  }
  if(list_has_focus) {
    m_calendar_view->setFocus();
  }
}

void CalendarDatePicker::on_list_current(const optional<int>& index) {
  if(index) {
    auto current_block = shared_connection_block(m_current_connection);
    m_model->set_current(m_calendar_view->get_list_model()->get<date>(*index));
  }
}

void CalendarDatePicker::on_submit(date day) {
  m_submit_signal(day);
}
