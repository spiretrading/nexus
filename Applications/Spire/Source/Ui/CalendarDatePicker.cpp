#include "Spire/Ui/CalendarDatePicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
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

  auto make_header_label(QString text, QWidget* parent) {
    auto label = make_label(text, parent);
    label->setFocusPolicy(Qt::NoFocus);
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

class CalendarDatePicker::MonthSelector : public QWidget {
  public:
    MonthSelector(std::shared_ptr<OptionalDateModel> date_model,
        QWidget* parent = nullptr)
        : m_date_model(std::move(date_model)) {
      m_model = std::make_shared<MonthModel>(
        m_date_model->get_current().value_or(day_clock::local_day()));
      setFixedSize(scale(168, 26));
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
      m_label->installEventFilter(this);
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

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::MouseButtonPress) {
        parentWidget()->setFocus();
      }
      return QWidget::eventFilter(watched, event);
    }

  private:
    std::shared_ptr<MonthModel> m_model;
    std::shared_ptr<OptionalDateModel> m_date_model;
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

class CalendarDatePicker::CalendarDayLabel : public QWidget {
  public:
    CalendarDayLabel(std::shared_ptr<DateModel> model,
        std::shared_ptr<MonthModel> month_model, QWidget* parent = nullptr)
        : QWidget(parent),
          m_model(std::move(model)),
          m_month_model(std::move(month_model)),
          m_is_release_ignored(false) {
      setObjectName(QString("CDL %1").arg(reinterpret_cast<std::intptr_t>(this)));
      setFixedSize(scale(24, 24));
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      m_label = make_label("", this);
      proxy_style(*this, *m_label);
      auto style = get_style(*m_label);
      style.get(Any()).
        set(BackgroundColor(QColor::fromRgb(0xFFFFFF))).
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
      layout->addWidget(m_label);
      on_current(m_model->get_current());
      m_model->connect_current_signal([=] (auto day) { on_current(day); });
      installEventFilter(this);
    }

    void set_release_ignored(bool is_ignored) {
      m_is_release_ignored = is_ignored;
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::MouseButtonRelease) {
        auto e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton) {
          if(m_is_release_ignored) {
            m_is_release_ignored = false;
            return true;
          }
        }
      }
      return QWidget::eventFilter(watched, event);
    }

  private:
    std::shared_ptr<DateModel> m_model;
    std::shared_ptr<MonthModel> m_month_model;
    TextBox* m_label;
    bool m_is_release_ignored;

    void on_current(date day) {
      m_label->get_model()->set_current(QString("%12").arg(day.day()));
      if(day == day_clock::local_day()) {
        match(*this, Today());
      } else {
        unmatch(*this, Today());
      }
      if(day.month() == m_month_model->get_current().month()) {
        unmatch(*this, OutOfMonth());
      } else {
        match(*this, OutOfMonth());
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
  setFixedSize(scale(176, 214));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(4), scale_width(8), scale_width(4),
    scale_height(4));
  layout->setSpacing(scale_height(4));
  m_month_selector = new MonthSelector(m_model, this);
  m_month_selector->get_model()->connect_current_signal(
    [=] (auto current) { on_current_month(current); });
  m_month_selector->installEventFilter(this);
  layout->addWidget(m_month_selector);
  m_day_header = make_day_header(this);
  m_day_header->installEventFilter(this);
  layout->addWidget(m_day_header);
  populate_calendar([=] (auto index, auto day) {
    m_calendar_model->push(std::make_shared<LocalDateModel>(day));
  });
  m_calendar_view = new ListView(m_calendar_model,
    [=] (const std::shared_ptr<ListModel>& model, int index) {
      auto label = new CalendarDayLabel(
        model->get<std::shared_ptr<LocalDateModel>>(index),
        m_month_selector->get_model());
      m_day_labels.push_back(label);
      return label;
    }, this);
  m_calendar_view->setFixedSize(scale(168, 144));
  setFocusProxy(m_calendar_view);
  m_month_selector->setFocusProxy(m_calendar_view);
  m_calendar_view->installEventFilter(this);
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
    set(TextColor(QColor::fromRgb(0xFFFFFF)));
  set_style(*m_calendar_view, std::move(calendar_style));
  update_calendar_model();
  m_list_current_connection =
    m_calendar_view->get_current_model()->connect_current_signal(
      [=] (auto index) { on_list_current(index); });
  layout->addWidget(m_calendar_view);
}

const std::shared_ptr<OptionalDateModel>&
    CalendarDatePicker::get_model() const {
  return m_model;
}

bool CalendarDatePicker::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_day_header && event->type() == QEvent::MouseButtonPress) {
    m_calendar_view->setFocus();
  }
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(watched == m_month_selector &&
        (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)) {
      qApp->sendEvent(m_calendar_view, e);
    } else {
      auto current_index = m_calendar_view->get_current_model()->get_current();
      if(current_index) {
        if(*current_index == 0 && e->key() == Qt::Key_Left) {
          m_model->set_current(*m_model->get_current() - days(1));
          return true;
        } else if(*current_index == DISPLAYED_DAYS - 1 &&
            e->key() == Qt::Key_Right) {
          m_model->set_current(*m_model->get_current() + days(1));
          return true;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
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
  auto current_set = false;
  auto list_has_focus =
    m_calendar_view->hasFocus() || m_calendar_view->isAncestorOf(focusWidget());
  populate_calendar([&] (auto index, auto day) {
    auto current_block = shared_connection_block(m_list_current_connection);
    m_calendar_model->
      get<std::shared_ptr<LocalDateModel>>(index)->set_current(day);
    auto minimum = m_model->get_minimum();
    auto maximum = m_model->get_maximum();
    if(minimum && maximum) {
      m_calendar_view->get_list_item(index)->setDisabled(
        day < *minimum || day > *maximum);
    } else {
      if(minimum) {
        m_calendar_view->get_list_item(index)->setDisabled(day < *minimum);
      } else if(maximum) {
        m_calendar_view->get_list_item(index)->setDisabled(day > *maximum);
      }
    }
    if(auto current = m_model->get_current();  day == current) {
      auto current_block =
        shared_connection_block(m_list_current_connection);
      m_calendar_view->get_current_model()->set_current(index);
      current_set = true;
      m_calendar_view->get_selection_model()->set_current(index);
    }
  });
  if(!current_set) {
    m_calendar_view->get_selection_model()->set_current({});
  }
  if(list_has_focus) {
    m_calendar_view->setFocus();
  }
}

void CalendarDatePicker::on_current_month(date month) {
  auto displayed_month =
    m_calendar_model->get<std::shared_ptr<LocalDateModel>>(8)->get_current();
  if(month.month() != displayed_month.month() ||
      month.year() != displayed_month.year()) {
    if(qApp->mouseButtons() == Qt::LeftButton) {
      m_day_labels[*m_calendar_view->get_current_model()->get_current()]->
        set_release_ignored(true);
    }
    update_calendar_model();
  }
}

void CalendarDatePicker::on_list_current(const boost::optional<int> index) {
  if(index) {
    m_model->set_current(m_calendar_model->
      get<std::shared_ptr<LocalDateModel>>(*index)->get_current());
  }
}
