#include "Spire/Ui/DurationBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QHBoxLayout>
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct HourModel : ScalarValueModel<optional<int>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_minutes;
    std::weak_ptr<ScalarValueModel<optional<DecimalBox::Decimal>>> m_seconds;
    optional<int> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    HourModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_current_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get_current());
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      return none;
    }

    int get_increment() const {
      return 1;
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<int>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      auto update = [&] () -> optional<time_duration> {
        if(!value) {
          auto minutes = m_minutes.lock();
          auto seconds = m_seconds.lock();
          if((!minutes || !minutes->get_current()) &&
              (!seconds || !seconds->get_current())) {
            return none;
          }
        }
        return m_source->get_current().get_value_or(hours(0)) +
          hours(value.get_value_or(0)) - hours(m_current.get_value_or(0));
      }();
      if(m_source->set_current(update) != QValidator::State::Invalid) {
        return m_state;
      }
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        auto hours = static_cast<int>(current->hours());
        if(hours != 0 || m_current) {
          m_current = hours;
        }
        return;
      } else {
        m_current = none;
      }
      m_state = QValidator::State::Acceptable;
      m_current_signal(m_current);
    }
  };

  struct MinuteModel : ScalarValueModel<optional<int>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_hours;
    std::weak_ptr<ScalarValueModel<optional<DecimalBox::Decimal>>> m_seconds;
    optional<int> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    MinuteModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_current_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get_current());
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      return 59;
    }

    int get_increment() const {
      return 1;
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<int>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      auto update = [&] () -> optional<time_duration> {
        if(!value) {
          auto hours = m_hours.lock();
          auto seconds = m_seconds.lock();
          if((!hours || !hours->get_current()) &&
              (!seconds || !seconds->get_current())) {
            return none;
          }
        }
        return m_source->get_current().get_value_or(minutes(0)) +
          minutes(value.get_value_or(0)) - minutes(m_current.get_value_or(0));
      }();
      if(m_source->set_current(update) != QValidator::State::Invalid) {
        return m_state;
      }
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        auto minutes = static_cast<int>(current->minutes());
        if(minutes != 0 || m_current) {
          m_current = minutes;
        }
        return;
      } else {
        m_current = none;
      }
      m_state = QValidator::State::Acceptable;
      m_current_signal(m_current);
    }
  };

  struct SecondModel : ScalarValueModel<optional<DecimalBox::Decimal>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_hours;
    std::weak_ptr<OptionalIntegerModel> m_minutes;
    optional<DecimalBox::Decimal> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    static auto to_seconds(const DecimalBox::Decimal& decimal) {
      return milliseconds(static_cast<time_duration::sec_type>(
        (1000 * decimal).convert_to<double>()));
    }

    SecondModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_current_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get_current());
    }

    optional<DecimalBox::Decimal> get_minimum() const {
      return 0;
    }

    optional<DecimalBox::Decimal> get_maximum() const {
      return DecimalBox::Decimal("59.999");
    }

    DecimalBox::Decimal get_increment() const {
      return DecimalBox::Decimal("0.001");
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<DecimalBox::Decimal>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      auto update = [&] () -> optional<time_duration> {
        if(!value) {
          auto hours = m_hours.lock();
          auto minutes = m_minutes.lock();
          if((!hours || !hours->get_current()) &&
              (!minutes || !minutes->get_current())) {
            return none;
          }
        }
        return m_source->get_current().get_value_or(seconds(0)) +
          to_seconds(value.get_value_or(0)) -
            to_seconds(m_current.get_value_or(0));
      }();
      if(m_source->set_current(update) != QValidator::State::Invalid) {
        return m_state;
      }
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        auto seconds = DecimalBox::Decimal((*current - hours(current->hours()) -
          minutes(current->minutes())).total_milliseconds()) / 1000;
        if(seconds != 0 || m_current) {
          m_current = seconds;
        }
        return;
      } else {
        m_current = none;
      }
      m_state = QValidator::State::Acceptable;
      m_current_signal(m_current);
    }
  };

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(BodyAlign(Qt::AlignCenter)).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(scale_width(4)));
    style.get(Hover()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto FOCUS_IN_STYLE(StyleSheet style) {
    style.get(Any()).set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    return style;
  }

  auto FOCUS_OUT_STYLE(StyleSheet style) {
    style.get(Any()).set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto HOUR_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto MINUTE_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto SECOND_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).set(TrailingZeros(3)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto COLON_FIELD_STYLE(StyleSheet style) {
    style.get(Any() > Colon()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(TextColor(QColor::fromRgb(0, 0, 0)));
    style.get(Disabled() > Colon()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  template<typename T>
  auto create_modifiers() {
    return QHash<Qt::KeyboardModifier, T>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }

  auto make_hour_field(std::shared_ptr<OptionalIntegerModel> model,
      QWidget& event_filter) {
    auto field = new IntegerBox(std::move(model), create_modifiers<int>());
    field->setMinimumWidth(scale_width(24));
    field->set_placeholder("hh");
    field->set_warning_displayed(false);
    set_style(*field, HOUR_FIELD_STYLE(get_style(*field)));
    field->findChild<QLineEdit*>()->installEventFilter(&event_filter);
    return field;
  }

  auto make_minute_field(std::shared_ptr<OptionalIntegerModel> model,
      QWidget& event_filter) {
    auto field = new IntegerBox(std::move(model), create_modifiers<int>());
    field->setMinimumWidth(scale_width(28));
    field->set_placeholder("mm");
    field->set_warning_displayed(false);
    set_style(*field, MINUTE_FIELD_STYLE(get_style(*field)));
    field->findChild<QLineEdit*>()->installEventFilter(&event_filter);
    return field;
  }

  auto make_second_field(
      std::shared_ptr<ScalarValueModel<optional<DecimalBox::Decimal>>> model,
      QWidget& event_filter) {
    auto field =
      new DecimalBox(std::move(model), create_modifiers<DecimalBox::Decimal>());
    field->setMinimumWidth(scale_width(44));
    field->set_placeholder("ss.sss");
    field->set_warning_displayed(false);
    set_style(*field, SECOND_FIELD_STYLE(get_style(*field)));
    field->findChild<QLineEdit*>()->installEventFilter(&event_filter);
    return field;
  }

  auto make_colon() {
    auto colon = new TextBox(":");
    colon->setFixedWidth(scale_width(10));
    colon->setEnabled(false);
    colon->set_read_only(true);
    find_stylist(*colon).match(Colon());
    return colon;
  }
}

DurationBox::DurationBox(QWidget* parent)
  : DurationBox(std::make_shared<LocalOptionalDurationModel>(), parent) {}

DurationBox::DurationBox(std::shared_ptr<OptionalDurationModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_warning_displayed(true) {
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto hour_model = std::make_shared<HourModel>(m_model);
  auto minute_model = std::make_shared<MinuteModel>(m_model);
  auto second_model = std::make_shared<SecondModel>(m_model);
  hour_model->m_minutes = minute_model;
  hour_model->m_seconds = second_model;
  minute_model->m_hours = hour_model;
  minute_model->m_seconds = second_model;
  second_model->m_hours = hour_model;
  second_model->m_minutes = minute_model;
  m_hour_field = make_hour_field(std::move(hour_model), *this);
  m_minute_field = make_minute_field(std::move(minute_model), *this);
  m_second_field = make_second_field(std::move(second_model), *this);
  auto hour_minute_colon = make_colon();
  auto minute_second_colon = make_colon();
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  container_layout->addWidget(m_hour_field, 6);
  container_layout->addWidget(hour_minute_colon);
  container_layout->addWidget(m_minute_field, 7);
  container_layout->addWidget(minute_second_colon);
  container_layout->addWidget(m_second_field, 11);
  set_style(*container, COLON_FIELD_STYLE(get_style(*container)));
  auto box = new Box(container);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  m_hour_field->connect_submit_signal([=] (const auto& submission) {
    if(m_hour_field->hasFocus()) {
      on_submit();
    }
  });
  m_minute_field->connect_submit_signal([=] (const auto& submission) {
    if(m_minute_field->hasFocus()) {
      on_submit();
    }
  });
  m_second_field->connect_submit_signal([=] (const auto& submission) {
    if(m_second_field->hasFocus()) {
      on_submit();
    }
  });
  m_hour_field->connect_reject_signal([=] (const auto& value) { on_reject(); });
  m_minute_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_second_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
}

const std::shared_ptr<OptionalDurationModel>& DurationBox::get_model() const {
  return m_model;
}

bool DurationBox::is_warning_displayed() const {
  return m_is_warning_displayed;
}

void DurationBox::set_warning_displayed(bool is_displayed) {
  m_is_warning_displayed = is_displayed;
}

connection DurationBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

connection DurationBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize DurationBox::sizeHint() const {
  return scale(126, 26);
}

bool DurationBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    set_style(*this, FOCUS_IN_STYLE(get_style(*this)));
  } else if(event->type() == QEvent::FocusOut) {
    if(!m_hour_field->hasFocus() && !m_minute_field->hasFocus() &&
        !m_second_field->hasFocus()) {
      set_style(*this, FOCUS_OUT_STYLE(get_style(*this)));
      if(m_hour_field->get_model()->get_state() == QValidator::Acceptable &&
          m_minute_field->get_model()->get_state() == QValidator::Acceptable &&
          m_second_field->get_model()->get_state() == QValidator::Acceptable) {
        on_submit();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DurationBox::on_submit() {
  auto minimum = m_model->get_minimum();
  auto maximum = m_model->get_maximum();
  if(minimum && m_model->get_current() < *minimum ||
      maximum && m_model->get_current() > *maximum) {
    on_reject();
  } else {
    m_submission = m_model->get_current();
    m_submit_signal(m_submission);
  }
}

void DurationBox::on_reject() {
  m_reject_signal(m_model->get_current());
  auto minimum = m_model->get_minimum();
  if(minimum && m_submission < *minimum) {
    m_submission = *minimum;
  }
  auto maximum = m_model->get_maximum();
  if(maximum && m_submission > *maximum) {
    m_submission = *maximum;
  }
  m_model->set_current(m_submission);
  if(m_is_warning_displayed) {
    display_warning_indicator(*this);
  }
}
