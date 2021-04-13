#include "Spire/Ui/DurationBox.hpp"
#include <QHBoxLayout>
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(BodyAlign(Qt::AlignCenter)).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(scale_width(4)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto get_milliseconds(time_duration duration) {
    return (duration - hours(duration.hours()) - minutes(duration.minutes())).
      total_milliseconds();
  }

  template<typename T>
  auto create_modifiers() {
    return QHash<Qt::KeyboardModifier, T>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }
}

DurationBox::DurationBox(QWidget* parent)
  : DurationBox(std::make_shared<LocalDurationModel>(), parent) {}

DurationBox::DurationBox(std::shared_ptr<LocalDurationModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_warning_displayed(true),
      m_is_hour_field_inputting(false),
      m_is_minute_field_inputting(false),
      m_is_second_field_inputting(false) {
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  create_hour_field();
  create_minute_field();
  create_second_field();
  create_colon_fields();
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  container_layout->addWidget(m_hour_field, 6);
  container_layout->addWidget(m_hour_minute_colon);
  container_layout->addWidget(m_minute_field, 7);
  container_layout->addWidget(m_minute_second_colon);
  container_layout->addWidget(m_second_field, 11);
  auto container_style = get_style(*container);
  container_style.get(Any() > Colon()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(TextColor(QColor::fromRgb(0, 0, 0)));
  container_style.get(Disabled() > Colon()).
    set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(*container, std::move(container_style));
  m_box = new Box(container);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(m_box);
  proxy_style(*this, *m_box);
  set_style(*this, DEFAULT_STYLE());
  m_hour_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_hour_field_current(current); });
  m_minute_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_minute_field_current(current); });
  m_second_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_second_field_current(current); });
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
  m_hour_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_minute_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_second_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_model->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
}

const std::shared_ptr<LocalDurationModel>& DurationBox::get_model() const {
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
    auto style = get_style(*this);
    style.get(Any()).set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    set_style(*this, std::move(style));
  } else if(event->type() == QEvent::FocusOut) {
    if(!m_hour_field->hasFocus() && !m_minute_field->hasFocus() &&
        !m_second_field->hasFocus()) {
      auto style = get_style(*this);
      style.get(Any()).set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
      set_style(*this, std::move(style));
      if(m_hour_field->get_model()->get_state() == QValidator::Acceptable &&
          m_minute_field->get_model()->get_state() == QValidator::Acceptable &&
          m_second_field->get_model()->get_state() == QValidator::Acceptable) {
        on_submit();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DurationBox::create_hour_field() {
  m_hour_field = new IntegerBox(m_model->get_hour_model(),
    create_modifiers<int>());
  m_hour_field->setMinimumWidth(scale_width(24));
  m_hour_field->set_placeholder("hh");
  m_hour_field->set_warning_displayed(false);
  auto hour_style = get_style(*m_hour_field);
  hour_style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(border_size(0)).
    set(horizontal_padding(scale_width(0))).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  hour_style.get(Any() > is_a<Button>()).set(
    Visibility(VisibilityOption::NONE));
  set_style(*m_hour_field, std::move(hour_style));
  m_hour_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_minute_field() {
  m_minute_field = new IntegerBox(m_model->get_minute_model(),
    create_modifiers<int>());
  m_minute_field->setMinimumWidth(scale_width(28));
  m_minute_field->set_placeholder("mm");
  m_minute_field->set_warning_displayed(false);
  auto minute_style = get_style(*m_minute_field);
  minute_style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(border_size(0)).
    set(horizontal_padding(scale_width(0))).
    set(LeadingZeros(2)).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  minute_style.get(Any() > is_a<Button>()).set(
    Visibility(VisibilityOption::NONE));
  set_style(*m_minute_field, std::move(minute_style));
  m_minute_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_second_field() {
  m_second_field = new DecimalBox(m_model->get_second_model(),
    create_modifiers<DecimalBox::Decimal>());
  m_second_field->setMinimumWidth(scale_width(44));
  m_second_field->set_placeholder("ss.sss");
  m_second_field->set_warning_displayed(false);
  auto second_style = get_style(*m_second_field);
  second_style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(border_size(0)).
    set(horizontal_padding(scale_width(0))).
    set(LeadingZeros(2)).set(TrailingZeros(3)).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  second_style.get(Any() > is_a<Button>()).set(
    Visibility(VisibilityOption::NONE));
  set_style(*m_second_field, std::move(second_style));
  m_second_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_colon_fields() {
  m_hour_minute_colon = new TextBox(":");
  m_hour_minute_colon->setFixedWidth(scale_width(10));
  m_hour_minute_colon->setEnabled(false);
  m_hour_minute_colon->set_read_only(true);
  m_minute_second_colon = new TextBox(":");
  m_minute_second_colon->setFixedWidth(scale_width(10));
  m_minute_second_colon->setEnabled(false);
  m_minute_second_colon->set_read_only(true);
  find_stylist(*m_hour_minute_colon).match(Colon());
  find_stylist(*m_minute_second_colon).match(Colon());
}

void DurationBox::on_hour_field_current(int current) {
  m_is_hour_field_inputting = true;
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_model->set_current(hours(current));
  } else {
    if(m_model->get_current().hours() != current) {
      auto current_minutes =
        minutes(m_minute_field->get_model()->get_current());
      auto current_seconds = milliseconds(static_cast<time_duration::sec_type>(
        m_second_field->get_model()->get_current().convert_to<double>() * 1000));
      m_model->set_current(hours(current) + current_minutes + current_seconds);
    }
  }
  m_is_hour_field_inputting = false;
}

void DurationBox::on_minute_field_current(int current) {
  m_is_minute_field_inputting = true;
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_model->set_current(minutes(current));
  } else {
    if(m_model->get_current().minutes() != current) {
      auto current_hours = hours(m_hour_field->get_model()->get_current());
      auto current_seconds = milliseconds(static_cast<time_duration::sec_type>(
        m_second_field->get_model()->get_current().convert_to<double>() * 1000));
      m_model->set_current(current_hours + minutes(current) + current_seconds);
    }
  }
  m_is_minute_field_inputting = false;
}

void DurationBox::on_second_field_current(const DecimalBox::Decimal& current) {
  m_is_second_field_inputting = true;
  auto milliseconds_value =
    static_cast<time_duration::sec_type>(current.convert_to<double>() * 1000);
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_model->set_current(milliseconds(milliseconds_value));
  } else {
    if(get_milliseconds(m_model->get_current()) != milliseconds_value) {
      m_model->set_current(hours(m_hour_field->get_model()->get_current()) +
        minutes(m_minute_field->get_model()->get_current()) +
        milliseconds(milliseconds_value));
    }
  }
  m_is_second_field_inputting = false;
}

void DurationBox::on_current(time_duration current) {
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_hour_field->findChild<QLineEdit*>()->setText("");
    m_minute_field->findChild<QLineEdit*>()->setText("");
    m_second_field->findChild<QLineEdit*>()->setText("");
  } else {
    if(!m_is_hour_field_inputting && !m_is_minute_field_inputting &&
        !m_is_second_field_inputting) {
      m_hour_field->get_model()->set_current(static_cast<int>(current.hours()));
      m_minute_field->get_model()->set_current(
        static_cast<int>(current.minutes()));
      m_second_field->get_model()->set_current(
        DecimalBox::Decimal(get_milliseconds(current)) / 1000);
    }
  }
}

void DurationBox::on_submit() {
  auto minimum = m_model->get_minimum();
  auto maximum = m_model->get_maximum();
  if(minimum && m_model->get_current() < *minimum ||
    maximum && m_model->get_current() > *maximum) {
    on_reject();
  } else {
    if(m_submission != m_model->get_current()) {
      m_submission = m_model->get_current();
      m_submit_signal(m_submission);
    }
  }
}

void DurationBox::on_reject() {
  m_reject_signal(m_model->get_current());
  m_model->set_current(m_submission);
  if(m_is_warning_displayed) {
    display_warning_indicator(*this);
  }
}
