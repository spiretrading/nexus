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
      m_is_warning_displayed(true) {
  auto center_widget = new QWidget(this);
  center_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto center_layout = new QHBoxLayout(center_widget);
  center_layout->setContentsMargins({});
  center_layout->setSpacing(0);
  auto hour_model = std::make_shared<LocalIntegerModel>();
  hour_model->set_minimum(0);
  m_hour_field = new IntegerBox(hour_model, QHash<Qt::KeyboardModifier, int>());
  m_hour_field->set_warning_displayed(false);
  m_hour_field->set_placeholder("hh");
  auto hour_style = m_hour_field->get_style();
  hour_style.get(is_a<TextBox>() > is_a<Box>()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(border_size(0));
  hour_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_hour_field->set_style(std::move(hour_style));
  auto minute_model = std::make_shared<LocalIntegerModel>();
  minute_model->set_minimum(0);
  minute_model->set_maximum(59);
  m_minute_field = new IntegerBox(minute_model,
    QHash<Qt::KeyboardModifier, int>());
  m_minute_field->set_warning_displayed(false);
  m_minute_field->set_placeholder("mm");
  auto minute_style = m_minute_field->get_style();
  minute_style.get(Any()).set(LeadingZeros(2));
  minute_style.get(is_a<TextBox>() > is_a<Box>()).set(border_size(0));
  minute_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_minute_field->set_style(std::move(minute_style));
  auto second_model = std::make_shared<LocalScalarValueModel<DecimalBox::Decimal>>();
  second_model->set_minimum(DecimalBox::Decimal(0));
  second_model->set_maximum(DecimalBox::Decimal(59.999));
  second_model->set_increment(pow(DecimalBox::Decimal(10), -3));
  m_second_field = new DecimalBox(second_model,
    QHash<Qt::KeyboardModifier, DecimalBox::Decimal>());
  m_second_field->set_warning_displayed(false);
  m_second_field->set_placeholder("ss.sss");
  auto second_style = m_second_field->get_style();
  second_style.get(Any()).
    set(border_size(0)).
    set(LeadingZeros(2)).set(TrailingZeros(3));
  second_style.get(is_a<Button>()).set(Visibility(VisibilityOption::NONE));
  m_second_field->set_style(std::move(second_style));
  auto m_colon1 = new TextBox(":");
  m_colon1->setEnabled(false);
  m_colon1->set_read_only(true);
  m_colon1->setFixedWidth(scale_width(10));
  auto colon_style = m_colon1->get_style();
  colon_style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  colon_style.get(ReadOnly() && Disabled()).
    set(TextColor(QColor::fromRgb(0, 0, 0))).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  m_colon1->set_style(std::move(colon_style));
  auto m_colon2 = new TextBox(":");
  m_colon2->setEnabled(false);
  m_colon2->set_read_only(true);
  m_colon2->setFixedWidth(scale_width(10));
  m_colon2->set_style(std::move(colon_style));
  center_layout->addWidget(m_hour_field, 1);
  center_layout->addWidget(m_colon1, 0);
  center_layout->addWidget(m_minute_field, 1);
  center_layout->addWidget(m_colon2, 0);
  center_layout->addWidget(m_second_field, 2);
  m_box = new Box(center_widget);
  auto box_style = m_box->get_style();
  box_style.get(Any()).
    set(BodyAlign(Qt::AlignCenter)).
    set(PaddingLeft(scale_width(4))).
    set(PaddingRight(scale_width(4)));
  m_box->set_style(box_style);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(m_box);
  m_hour_field->get_model()->connect_current_signal([=] (const auto& current) {
    if(m_model->get_state() == QValidator::State::Invalid) {
      m_model->set_current(hours(current));
    } else {
      auto changes = hours(current) - hours(m_model->get_current().hours());
      if(changes != hours(0)) {
        m_model->set_current(m_model->get_current() + changes);
      }
    }
  });
  m_minute_field->get_model()->connect_current_signal([=] (const auto& current) {
    if(m_model->get_state() == QValidator::State::Invalid) {
      m_model->set_current(minutes(current));
    } else {
      auto changes = minutes(current) - minutes(m_model->get_current().minutes());
      if(changes != minutes(0)) {
        m_model->set_current(m_model->get_current() + changes);
      }
    }
  });
  m_second_field->get_model()->connect_current_signal([=] (const auto& current) {
    auto seconds_value = current.convert_to<float>();
    if(m_model->get_state() == QValidator::State::Invalid) {
      m_model->set_current(milliseconds(static_cast<long>(seconds_value * 1000)));
    } else {
      auto duration = m_model->get_current();
      auto current_seconds = duration - hours(duration.hours()) -
        minutes(duration.minutes());
      auto changes = static_cast<long>(seconds_value * 1000) -
        current_seconds.total_milliseconds();
      if(changes != 0) {
        m_model->set_current(m_model->get_current() + milliseconds(changes));
      }
    }
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
    if(m_model->get_state() == QValidator::State::Invalid) {
      clear_leading_trailing_zeros();
      m_hour_field->findChild<QLineEdit*>()->setText("");
      m_minute_field->findChild<QLineEdit*>()->setText("");
      m_second_field->findChild<QLineEdit*>()->setText("");
    } else {
      set_leading_trailing_zeros();
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

void DurationBox::on_submit() {
  auto minimum = m_model->get_minimum();
  auto maximum = m_model->get_maximum();
  if(minimum && m_model->get_current() < *minimum ||
    maximum && m_model->get_current() > *maximum) {
    m_reject_signal(m_model->get_current());
    m_model->set_current(m_submission);
    if(m_is_warning_displayed) {
      display_warning_indicator(*m_box);
    }
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

void DurationBox::clear_leading_trailing_zeros() {
  auto minute_style = m_minute_field->get_style();
  minute_style.get(Any()).set(LeadingZeros(0));
  m_minute_field->set_style(std::move(minute_style));
  auto second_style = m_second_field->get_style();
  second_style.get(Any()).set(LeadingZeros(0)).set(TrailingZeros(0));
  m_second_field->set_style(std::move(second_style));
}

void DurationBox::set_leading_trailing_zeros() {
  auto minute_style = m_minute_field->get_style();
  minute_style.get(Any()).set(LeadingZeros(2));
  m_minute_field->set_style(std::move(minute_style));
  auto second_style = m_second_field->get_style();
  second_style.get(Any()).set(LeadingZeros(2)).set(TrailingZeros(3));
  m_second_field->set_style(std::move(second_style));
}
