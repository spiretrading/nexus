#include "Spire/Ui/CalendarDatePicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QCoreApplication>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto CALENDAR_DAY_COUNT = 42;

  auto clamp(date value, const optional<date>& min, const optional<date>& max) {
    if(min) {
      value = std::max(*min, value);
    }
    if(max) {
      value = std::min(*max, value);
    }
    return value;
  };

  auto get_start_of_month(const date& day) {
    return date(day.year(), day.month(), 1);
  }

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
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    auto label = make_label(std::move(text), parent);
    label->setFixedSize(scale(24, 24));
    update_style(*label, [&] (auto& style) {
      style.get(Any()).
        set(text_style(font, QColor(QColor(0x808080)))).
        set(TextAlign(Qt::AlignCenter));
    });
    return label;
  }

  auto make_day_header(QWidget* parent) {
    auto header = new QWidget(parent);
    header->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    header->setFocusPolicy(Qt::NoFocus);
    auto layout = make_hbox_layout(header);
    auto locale = QLocale();
    layout->addWidget(make_header_label(locale.dayName(7).at(0), header));
    for(auto i = 1; i < 7; ++i) {
      layout->addWidget(make_header_label(locale.dayName(i).at(0), header));
    }
    return header;
  }
}

class CalendarListModel : public ListModel<date> {
  public:
    CalendarListModel(std::shared_ptr<DateModel> current)
        : m_current(std::move(current)),
          m_current_connection(m_current->connect_update_signal(
            [=] (auto current) { on_current(current); })) {
      on_current(m_current->get());
    }

    const std::shared_ptr<DateModel>& get() const {
      return m_current;
    }

    int get_size() const override {
      return m_dates.size();
    }

    const date& get(int index) const override {
      return m_dates[index];
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

  protected:
    void transact(const std::function<void ()>& transaction) override {
      m_transaction.transact(transaction);
    }

  private:
    std::shared_ptr<DateModel> m_current;
    scoped_connection m_current_connection;
    std::array<date, CALENDAR_DAY_COUNT> m_dates;
    ListModelTransactionLog<date> m_transaction;

    void on_current(date current) {
      auto previous = m_dates;
      m_dates = get_calendar_dates(current);
      m_transaction.transact([&] {
        for(auto i = 0; i < static_cast<int>(m_dates.size()); ++i) {
          m_transaction.push(UpdateOperation(i, previous[i], m_dates[i]));
        }
      });
    }
};

class RequiredDateModel : public DateModel {
  public:
    RequiredDateModel(std::shared_ptr<OptionalDateModel> model)
      : m_model(std::move(model)),
        m_current(m_model->get().value_or(day_clock::local_day())),
        m_current_connection(m_model->connect_update_signal(
          std::bind_front(&RequiredDateModel::on_current, this))) {}

    const Type& get() const override {
      return m_current;
    }

    QValidator::State test(const date& value) const override {
      return QValidator::State::Acceptable;
    }

    QValidator::State set(const date& value) override {
      m_current = value;
      m_update_signal(m_current);
      return QValidator::State::Acceptable;
    }

    optional<date> get_minimum() const {
      return m_model->get_minimum();
    }

    optional<date> get_maximum() const {
      return m_model->get_maximum();
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override {
      return m_update_signal.connect(slot);
    }

  private:
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDateModel> m_model;
    date m_current;
    scoped_connection m_current_connection;

    void on_current(const optional<date>& current) {
      if(current) {
        set(*current);
      }
    }
};

class CalendarDatePicker::MonthSpinner : public QWidget {
  public:
    explicit MonthSpinner(
        std::shared_ptr<RequiredDateModel> current, QWidget* parent = nullptr)
        : QWidget(parent),
          m_current(std::move(current)),
          m_connection(m_current->connect_update_signal(
            std::bind_front(&MonthSpinner::on_current, this))) {
      const auto ICON_SIZE = scale(16, 16);
      const auto BUTTON_SIZE = scale(26, 26);
      m_previous_button = make_icon_button(
        image_from_svg(":Icons/calendar-arrow-left.svg", ICON_SIZE));
      m_previous_button->setFixedSize(BUTTON_SIZE);
      m_previous_button->connect_click_signal([=] { update_current(-1); });
      auto layout = make_hbox_layout(this);
      layout->addWidget(m_previous_button);
      m_label = make_label(std::make_shared<ToTextModel<date>>(m_current,
        [] (const date& current) {
          return QString("%1 %2").
            arg(current.month().as_long_string()).arg(current.year());
        }), this);
      m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      update_style(*m_label, [&] (auto& style) {
        style.get(Any()).
          set(TextAlign(Qt::AlignCenter));
      });
      layout->addWidget(m_label);
      m_next_button = make_icon_button(
        image_from_svg(":Icons/calendar-arrow-right.svg", ICON_SIZE));
      m_next_button->setFixedSize(BUTTON_SIZE);
      m_next_button->connect_click_signal([=] { update_current(1); });
      layout->addWidget(m_next_button);
      on_current(m_current->get());
    }

    const std::shared_ptr<RequiredDateModel>& get() const {
      return m_current;
    }

  private:
    std::shared_ptr<RequiredDateModel> m_current;
    TextBox* m_label;
    Button* m_previous_button;
    Button* m_next_button;
    scoped_connection m_connection;

    void update_current(int direction) {
      auto modifiers = QApplication::keyboardModifiers();
      auto step = [&] {
        if(modifiers & Qt::AltModifier) {
          return months(3);
        } else if(modifiers & Qt::ControlModifier) {
          return months(6);
        } else if(modifiers & Qt::ShiftModifier) {
          return months(12);
        }
        return months(1);
      }();
      m_current->set(
        clamp(m_current->get() + step * direction, *m_current->get_minimum(),
          *m_current->get_maximum()));
    }

    void on_current(const date& current) {
      auto minimum = m_current->get_minimum();
      m_previous_button->setDisabled(minimum &&
        get_start_of_month(current) <= get_start_of_month(*minimum));
      auto maximum = m_current->get_maximum();
      m_next_button->setDisabled(maximum &&
        get_start_of_month(current) >= get_start_of_month(*maximum));
    }
};

class CalendarDayLabel : public QWidget {
  public:
    CalendarDayLabel(std::shared_ptr<ListValueModel<date>> current,
        std::shared_ptr<DateModel> month, QWidget* parent = nullptr)
        : QWidget(parent),
          m_current(std::move(current)),
          m_current_connection(m_current->connect_update_signal(
            std::bind_front(&CalendarDayLabel::on_current, this))),
          m_month(std::move(month)) {
      setFixedSize(scale(24, 24));
      m_label = make_label("", this);
      enclose(*this, *m_label);
      proxy_style(*this, *m_label);
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(12));
      update_style(*this, [&] (auto& style) {
        style.get(Any()).
          set(border(scale_width(1), QColor(Qt::transparent))).
          set(TextAlign(Qt::AlignCenter)).
          set(TextColor(QColor(Qt::black)));
        style.get(OutOfMonth()).
          set(TextColor(QColor(0xA0A0A0)));
        style.get(Today()).
          set(border_color(QColor(0x4B23A0))).
          set(text_style(font, QColor(QColor(0x4B23A0))));
        style.get(Hover()).
          set(BackgroundColor(QColor(0xF2F2FF)));
        style.get(Disabled()).
          set(TextColor(QColor(0xC8C8C8)));
      });
      on_current(m_current->get());
    }

  private:
    std::shared_ptr<ListValueModel<date>> m_current;
    scoped_connection m_current_connection;
    std::shared_ptr<DateModel> m_month;
    TextBox* m_label;

    void on_current(date day) {
      m_label->get_current()->set(QString("%1").arg(day.day()));
      if(day == day_clock::local_day()) {
        match(*this, Today());
      } else {
        unmatch(*this, Today());
      }
      if(day.month() != m_month->get().month()) {
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
    std::shared_ptr<OptionalDateModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  m_current_connection = m_current->connect_update_signal([=] (auto current) {
    on_current(current);
  });
  auto layout = make_vbox_layout(this);
  layout->setSpacing(scale_height(4));
  m_month_spinner =
    new MonthSpinner(std::make_shared<RequiredDateModel>(m_current), this);
  m_month_spinner->installEventFilter(this);
  layout->addWidget(m_month_spinner);
  layout->addWidget(make_day_header(this));
  m_calendar_model =
    std::make_shared<CalendarListModel>(m_month_spinner->get());
  m_calendar_view = new ListView(m_calendar_model,
    [=] (const std::shared_ptr<ListModel<date>>& list, int index) {
      return new CalendarDayLabel(
        make_list_value_model(list, index), m_month_spinner->get());
    }, this);
  m_month_spinner->get()->connect_update_signal([=] (auto current) {
    on_current_month(current);
  });
  m_calendar_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_calendar_view->setFixedWidth(scale_width(168));
  m_month_spinner->setFocusProxy(m_calendar_view);
  m_calendar_view->installEventFilter(this);
  layout->addWidget(m_calendar_view);
  m_list_current_connection =
    m_calendar_view->get_current()->connect_update_signal(
      [=] (const auto& index) { on_list_current(index); });
  m_calendar_view->connect_submit_signal([=] (const auto& value) {
    on_submit(std::any_cast<date>(value));
  });
  auto calendar_style = StyleSheet();
  calendar_style.get(Any()).
    set(Qt::Horizontal).
    set(Overflow(Overflow::WRAP));
  calendar_style.get(Any() > is_a<ListItem>()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border_size(0)).
    set(padding(0));
  calendar_style.get(Any() >
      (is_a<ListItem>() && Current()) > is_a<CalendarDayLabel>()).
    set(BackgroundColor(QColor(0x4B23A0))).
    set(TextColor(QColor(0xFFFFFF)));
  set_style(*m_calendar_view, std::move(calendar_style));
  setFocusProxy(m_calendar_view);
  on_current(m_current->get());
  on_current_month(m_month_spinner->get()->get());
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_current() const {
  return m_current;
}

connection CalendarDatePicker::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool CalendarDatePicker::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    auto& current_date = m_current->get();
    if(current_date) {
      auto new_date = *current_date;
      if(key_event.key() == Qt::Key_Left || key_event.key() == Qt::Key_Right ||
          key_event.key() == Qt::Key_Up || key_event.key() == Qt::Key_Down) {
        auto direction = [&] {
          if(key_event.key() == Qt::Key_Left || key_event.key() == Qt::Key_Up) {
            return -1;
          }
          return 1;
        }();
        auto step = [&] () -> date_duration {
          if(key_event.key() == Qt::Key_Left ||
              key_event.key() == Qt::Key_Right) {
            return days(1);
          }
          return weeks(1);
        }();
        if(!get_index(*current_date)) {
          if(direction > 0) {
            new_date = get_start_of_month(m_month_spinner->get()->get());
          } else {
            new_date = m_month_spinner->get()->get().end_of_month();
          }
        } else if(direction > 0) {
          new_date += step;
        } else {
          new_date -= step;
        }
      } else if(key_event.key() == Qt::Key_PageUp) {
        if(key_event.modifiers().testFlag(Qt::AltModifier)) {
          new_date -= years(1);
        } else {
          new_date -= months(1);
        }
      } else if(key_event.key() == Qt::Key_PageDown) {
        if(key_event.modifiers().testFlag(Qt::AltModifier)) {
          new_date += years(1);
        } else {
          new_date += months(1);
        }
      } else if(key_event.key() == Qt::Key_Home) {
        if(current_date->day() == 1) {
          new_date -= months(1);
        } else {
          new_date = get_start_of_month(new_date);
        }
      } else if(key_event.key() == Qt::Key_End) {
        if(current_date->day() == current_date->end_of_month().day()) {
          new_date += months(1);
        } else {
          new_date = current_date->end_of_month();
        }
      } else {
        return QWidget::eventFilter(watched, event);
      }
      m_current->set(
        clamp(new_date, m_current->get_minimum(), m_current->get_maximum()));
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

boost::optional<int> CalendarDatePicker::get_index(date day) const {
  auto first_date = m_calendar_model->get(0);
  auto last_date =  m_calendar_model->get(m_calendar_model->get_size() - 1);
  if(day < first_date || last_date < day) {
    return {};
  }
  return date_duration(day - first_date).days();
}

void CalendarDatePicker::set_current_index(const optional<int>& index) {
  auto current_block =
    shared_connection_block(m_list_current_connection);
  m_calendar_view->get_current()->set(index);
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
  auto minimum = m_current->get_minimum();
  auto maximum = m_current->get_maximum();
  for(auto i = 0; i < m_calendar_model->get_size(); ++i) {
    auto current = m_calendar_model->get(i);
    auto item = m_calendar_view->get_list_item(i);
    auto is_disabled = (minimum && current < *minimum) ||
      (maximum && current > *maximum);
    if(is_disabled) {
      if(item->hasFocus()) {
        m_calendar_view->setFocusProxy(nullptr);
        m_calendar_view->setFocus();
      }
    }
    item->setDisabled(is_disabled);
    if(current == m_current->get()) {
      current_index = i;
    }
  }
  if(current_index != m_calendar_view->get_current()->get()) {
    set_current_index(current_index);
  }
  if(list_has_focus) {
    m_calendar_view->setFocus();
  }
}

void CalendarDatePicker::on_list_current(const optional<int>& index) {
  if(index) {
    auto current_block = shared_connection_block(m_current_connection);
    m_current->set(m_calendar_model->get(*index));
  }
}

void CalendarDatePicker::on_submit(date day) {
  m_submit_signal(day);
}
