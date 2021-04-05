#include "Spire/Ui/DurationBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

DurationBox::DurationBox(QWidget* parent)
  : DurationBox(std::make_shared<LocalDurationModel>(), parent) {}

DurationBox::DurationBox(std::shared_ptr<LocalDurationModel> model, QWidget* parent)
    : StyledWidget(parent),
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
  container_layout->addWidget(m_colon1);
  container_layout->addWidget(m_minute_field, 7);
  container_layout->addWidget(m_colon2);
  container_layout->addWidget(m_second_field, 11);
  m_box = new Box(container);
  auto box_style = m_box->get_style();
  box_style.get(Any()).
    set(BodyAlign(Qt::AlignCenter)).
    set(horizontal_padding(scale_width(0)));
  m_box->set_style(box_style);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(m_box);
  m_hour_field->get_model()->connect_current_signal([=] (const auto& current) {
    on_hour_field_current(current);
  });
  m_minute_field->get_model()->connect_current_signal(
    [=] (const auto& current) {
      on_minute_field_current(current);
    });
  m_second_field->get_model()->connect_current_signal(
    [=] (const auto& current) {
      on_second_field_current(current);
    });
  m_hour_field->connect_submit_signal([=] (const auto& submission) {
    on_submit();
  });
  m_minute_field->connect_submit_signal([=] (const auto& submission) {
    on_submit();
  });
  m_second_field->connect_submit_signal([=] (const auto& submission) {
    on_submit();
  });
  m_hour_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_minute_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_second_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_model->connect_current_signal([=] (const auto& current) {
    on_current(current);
  });
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

void DurationBox::create_hour_field() {
  auto hour_model = std::make_shared<LocalIntegerModel>();
  hour_model->set_minimum(0);
  auto hour_modifiers = QHash<Qt::KeyboardModifier, int>({{Qt::NoModifier, 1}});
  m_hour_field = new IntegerBox(hour_model, hour_modifiers);
  m_hour_field->setMinimumWidth(scale_width(24));
  m_hour_field->set_placeholder("hh");
  m_hour_field->set_warning_displayed(false);
  auto hour_style = m_hour_field->get_style();
  hour_style.get(Any()).
    set(border_size(0)).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set_override(Rule::Override::EXCLUSIVE);
  hour_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_hour_field->set_style(std::move(hour_style));
}

void DurationBox::create_minute_field() {
  auto minute_model = std::make_shared<LocalIntegerModel>();
  minute_model->set_minimum(0);
  minute_model->set_maximum(59);
  auto minute_modifiers = QHash<Qt::KeyboardModifier, int>(
    {{Qt::NoModifier, 1}});
  m_minute_field = new IntegerBox(minute_model, minute_modifiers);
  m_minute_field->setMinimumWidth(scale_width(28));
  m_minute_field->set_placeholder("mm");
  m_minute_field->set_warning_displayed(false);
  auto minute_style = m_minute_field->get_style();
  minute_style.get(Any()).
    set(border_size(0)).
    set(LeadingZeros(2)).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set_override(Rule::Override::EXCLUSIVE);
  minute_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_minute_field->set_style(std::move(minute_style));
}

void DurationBox::create_second_field() {
  auto second_model =
    std::make_shared<LocalScalarValueModel<DecimalBox::Decimal>>();
  second_model->set_minimum(DecimalBox::Decimal(0));
  second_model->set_maximum(DecimalBox::Decimal(59.999));
  second_model->set_increment(pow(DecimalBox::Decimal(10), -3));
  auto second_modifiers = QHash<Qt::KeyboardModifier, DecimalBox::Decimal>(
    {{Qt::NoModifier, 1.0}});
  m_second_field = new DecimalBox(second_model, second_modifiers);
  m_second_field->setMinimumWidth(scale_width(44));
  m_second_field->set_placeholder("ss.sss");
  m_second_field->set_warning_displayed(false);
  auto second_style = m_second_field->get_style();
  second_style.get(Any()).
    set(border_size(0)).
    set(LeadingZeros(2)).
    set(TrailingZeros(3)).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set_override(Rule::Override::EXCLUSIVE);
  second_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_second_field->set_style(std::move(second_style));
}

void DurationBox::create_colon_fields() {
  m_colon1 = new TextBox(":");
  m_colon1->setFixedWidth(scale_width(10));
  m_colon1->setEnabled(false);
  m_colon1->set_read_only(true);
  auto colon_style = m_colon1->get_style();
  colon_style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  colon_style.get(ReadOnly() && Disabled()).
    set(TextColor(QColor::fromRgb(0, 0, 0))).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  m_colon1->set_style(std::move(colon_style));
  m_colon2 = new TextBox(":");
  m_colon2->setFixedWidth(scale_width(10));
  m_colon2->setEnabled(false);
  m_colon2->set_read_only(true);
  m_colon2->set_style(std::move(colon_style));
}

void DurationBox::on_hour_field_current(int current) {
  m_is_hour_field_inputting = true;
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_model->set_current(hours(current));
  } else {
    auto duration = m_model->get_current();
    if(duration.hours() != current) {
      auto current_minutes = minutes(m_minute_field->get_model()->get_current());
      auto current_seconds = milliseconds(static_cast<Duration::sec_type>(
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
    auto duration = m_model->get_current();
    if(duration.minutes() != current) {
      auto current_hours = hours(m_hour_field->get_model()->get_current());
      auto current_seconds = milliseconds(static_cast<Duration::sec_type>(
        m_second_field->get_model()->get_current().convert_to<double>() * 1000));
      m_model->set_current(current_hours + minutes(current) + current_seconds);
    }
  }
  m_is_minute_field_inputting = false;
}

void DurationBox::on_second_field_current(const DecimalBox::Decimal& current) {
  m_is_second_field_inputting = true;
  auto seconds_value =
    static_cast<Duration::sec_type>(current.convert_to<double>() * 1000);
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_model->set_current(milliseconds(seconds_value));
  } else {
    auto duration = m_model->get_current();
    if(duration.total_microseconds() != seconds_value) {
      auto current_hours = hours(m_hour_field->get_model()->get_current());
      auto current_minutes = minutes(m_minute_field->get_model()->get_current());
      m_model->set_current(current_hours + current_minutes +
        milliseconds(seconds_value));
    }
  }
  m_is_second_field_inputting = false;
}

void DurationBox::on_current(const Duration& current) {
  if(m_model->get_state() == QValidator::State::Invalid) {
    m_hour_field->findChild<QLineEdit*>()->setText("");
    m_minute_field->findChild<QLineEdit*>()->setText("");
    m_second_field->findChild<QLineEdit*>()->setText("");
  } else {
    if(!m_is_hour_field_inputting && !m_is_minute_field_inputting &&
        !m_is_second_field_inputting) {
      auto current_hours = static_cast<int>(current.hours());
      m_hour_field->get_model()->set_current(current_hours);
      auto current_minutes = static_cast<int>(current.minutes());
      m_minute_field->get_model()->set_current(current_minutes);
      auto current_seconds = current - hours(current_hours) -
        minutes(current_minutes);
      auto current_milliseconds = DecimalBox::Decimal(
        current_seconds.total_milliseconds());
      m_second_field->get_model()->set_current(current_milliseconds / 1000);
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
    m_submission = m_model->get_current();
    m_submit_signal(m_submission);
  }
}

void DurationBox::on_reject() {
  m_reject_signal(m_model->get_current());
  m_model->set_current(m_submission);
  if(m_is_warning_displayed) {
    display_warning_indicator(*m_box);
  }
}
