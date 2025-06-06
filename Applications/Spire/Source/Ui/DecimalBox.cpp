#include "Spire/Ui/DecimalBox.hpp"
#include <bitset>
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QKeyEvent>
#include <qt_windows.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto REPEAT_DELAY_TIMEOUT_MS = 500;
  const auto REPEAT_TIMEOUT_MS = 150;

  auto validate_fractional_part(const Decimal& value, int decimal_places) {
    if(value < pow(Decimal(10), -decimal_places)) {
      return QValidator::State::Intermediate;
    }
    return QValidator::State::Invalid;
  }

  optional<Decimal> text_to_decimal(const QString& text) {
    auto trimmed_text = text.trimmed().toStdString();
    if(trimmed_text.empty()) {
      return none;
    }
    try {
      return Decimal(trimmed_text.c_str());
    } catch (const std::runtime_error&) {
      return none;
    }
  }

  auto BUTTON_RIGHT_PADDING() {
    static auto padding = scale_width(6);
    return padding;
  }

  auto BUTTON_SIZE() {
    static auto size = scale(10, 10);
    return size;
  }

  auto make_modifiers(const OptionalDecimalModel& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, Decimal>();
    modifiers[Qt::NoModifier] = current.get_increment().get_value_or(1);
    return modifiers;
  }
}

struct DecimalBox::DecimalToTextModel : TextModel {
  mutable UpdateSignal m_update_signal;
  std::shared_ptr<OptionalDecimalModel> m_model;
  int m_decimal_places;
  int m_leading_zeros;
  int m_trailing_zeros;
  QString m_current;
  QRegExp m_validator;
  bool m_is_rejected;
  scoped_connection m_current_connection;

  DecimalToTextModel(std::shared_ptr<OptionalDecimalModel> model)
      : m_model(std::move(model)),
        m_decimal_places(
          -log10(m_model->get_increment().get_value_or(1)).convert_to<int>()),
        m_leading_zeros(0),
        m_trailing_zeros(0),
        m_current(to_string(m_model->get())),
        m_is_rejected(false),
        m_current_connection(m_model->connect_update_signal(
          std::bind_front(&DecimalToTextModel::on_current, this))) {
    update_validator();
  }

  void set_leading_zeros(int leading_zeros) {
    if(leading_zeros == m_leading_zeros) {
      return;
    }
    m_leading_zeros = leading_zeros;
    update_validator();
    if(m_current.isEmpty()) {
      return;
    }
    auto displayed_value = to_string(m_model->get());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_update_signal(m_current);
    }
  }

  void set_trailing_zeros(int trailing_zeros) {
    if(trailing_zeros == m_trailing_zeros) {
      return;
    }
    m_trailing_zeros = trailing_zeros;
    update_validator();
    if(m_current.isEmpty()) {
      return;
    }
    auto displayed_value = to_string(m_model->get());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_update_signal(m_current);
    }
  }

  const optional<Decimal>& submit() {
    auto displayed_value = to_string(m_model->get());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_update_signal(m_current);
    }
    return m_model->get();
  }

  void reject() {
    m_is_rejected = true;
  }

  QValidator::State get_state() const override {
    return m_model->get_state();
  }

  const QString& get() const {
    return m_current;
  }

  QValidator::State test(const QString& value) const override {
    auto decimal_places = 0;
    auto i = m_model->get_increment().get_value_or(1);
    while(i < 1) {
      i *= 10;
      ++decimal_places;
    }
    auto local_validator = optional<const QRegExp>();
    auto& validator = [&] () -> auto& {
      if(decimal_places != m_decimal_places) {
        local_validator.emplace(make_validator(decimal_places));
        return *local_validator;
      } else {
        return m_validator;
      }
    }();
    auto min = m_model->get_minimum();
    auto max = m_model->get_maximum();
    if(!validator.exactMatch(value)) {
      return QValidator::State::Invalid;
    } else if(value.isEmpty()) {
      return QValidator::State::Intermediate;
    } else if(value == "-") {
      if(min && *min < 0 || !min) {
        return QValidator::State::Intermediate;
      }
    } else if(value == "+") {
      if(max && *max > 0 || !max) {
        return QValidator::State::Intermediate;
      }
    } else if(auto decimal = text_to_decimal(value)) {
      if(value.front() != '-' && max && *max < 0 ||
          value.front() == '-' && min && *min > 0) {
        return QValidator::State::Invalid;
      }
      if(value.contains('.')) {
        if(min) {
          if(trunc(*decimal) != trunc(*min)) {
            if(*decimal < *min) {
              return QValidator::State::Invalid;
            }
          } else if(*decimal >= 0) {
            auto d = *min - *decimal;
            if(d >= 0) {
              return validate_fractional_part(d,
                value.length() - value.indexOf('.') - 1);
            }
          }
        }
        if(max) {
          if(trunc(*decimal) != trunc(*max)) {
            if(*decimal > *max) {
              return QValidator::State::Invalid;
            }
          } else if(*decimal <= 0) {
            auto d = *decimal - *max;
            if(d >= 0) {
              return validate_fractional_part(d,
                value.length() - value.indexOf('.') - 1);
            }
          }
        }
      }
      auto state = validate(*decimal, min, max);
      if(state == QValidator::State::Invalid) {
        return QValidator::State::Invalid;
      }
      return m_model->test(*decimal);
    }
    return QValidator::State::Invalid;
  }

  QValidator::State set(const QString& value) override {
    update_decimal_places();
    if(!m_validator.exactMatch(value)) {
      m_is_rejected = false;
      return QValidator::State::Invalid;
    } else if(value.isEmpty() || value == "-" || value == "+") {
      auto blocker = shared_connection_block(m_current_connection);
      m_model->set(none);
      m_current = value;
      m_update_signal(m_current);
      m_is_rejected = false;
      return QValidator::State::Intermediate;
    } else if(auto decimal = text_to_decimal(value)) {
      auto state =
        validate(*decimal, m_model->get_minimum(), m_model->get_maximum());
      if(state == QValidator::State::Invalid) {
        return QValidator::State::Invalid;
      }
      auto blocker = shared_connection_block(m_current_connection);
      state = m_model->set(*decimal);
      if(state != QValidator::State::Invalid) {
        if(m_is_rejected) {
          m_current = to_string(*decimal);
        } else {
          m_current = value;
        }
        m_update_signal(m_current);
      }
      m_is_rejected = false;
      return state;
    }
    m_is_rejected = false;
    return QValidator::State::Invalid;
  }

  connection connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const override {
    return m_update_signal.connect(slot);
  }

  QRegExp make_validator(int decimal_places) const {
    if(decimal_places <= 0) {
      return QRegExp(QString("^[-|\\+]?[0-9]*"));
    } else if(m_trailing_zeros > decimal_places) {
      auto delta = m_trailing_zeros - decimal_places;
      return QRegExp(QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1}0{0,%2})?").arg(
        decimal_places).arg(delta));
    } else {
      return QRegExp(
        QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1})?").arg(decimal_places));
    }
  }

  void update_decimal_places() {
    auto decimal_places = 0;
    auto i = m_model->get_increment().get_value_or(1);
    while(i < 1) {
      i *= 10;
      ++decimal_places;
    }
    if(decimal_places != m_decimal_places) {
      m_decimal_places = decimal_places;
      update_validator();
    }
  }

  void update_validator() {
    m_validator = make_validator(m_decimal_places);
  }

  bool update_leading_zeros(QString& source, const QString& digits) const {
    auto leading_zeros = std::max(1, m_leading_zeros);
    if(digits.length() < leading_zeros) {
      auto padding = QString("0");
      auto insert_index = [&] {
        if(!source.isEmpty() && (source[0] == '-' || source[0] == '+')) {
          return 1;
        }
        return 0;
      }();
      for(auto i = digits.length() + 1; i < leading_zeros; ++i) {
        padding += '0';
      }
      source.insert(insert_index, padding);
      return true;
    }
    return false;
  }

  bool update_trailing_zeros(QString& source, const QString& digits) const {
    if(digits.length() < m_trailing_zeros) {
      for(auto i = digits.length(); i < m_trailing_zeros; ++i) {
        source += "0";
      }
      return true;
    }
    auto delta = digits.length() - std::max(m_trailing_zeros, m_decimal_places);
    if(delta > 0) {
      source.chop(delta);
      return true;
    }
    return false;
  }

  QString to_string(const optional<Decimal>& value) const {
    static auto DECIMAL_PATTERN = QRegExp("^([-|\\+]?([0-9]*))(\\.([0-9]*))?");
    static const auto LEADING_DIGITS_CAPTURE_GROUP = 2;
    static const auto TRAILING_CAPTURE_GROUP = 3;
    static const auto TRAILING_DIGITS_CAPTURE_GROUP = 4;
    if(!value) {
      return {};
    }
    auto s = QString::fromStdString(value->str(
      Decimal::backend_type::cpp_dec_float_digits10, std::ios_base::fixed));
    auto decimal_point_pos = s.indexOf(".");
    if(decimal_point_pos >= 0) {
      auto trailing_zeros = 0;
      for(auto pos = s.rbegin(); pos != s.rend() - decimal_point_pos; ++pos) {
        if(*pos == '0' || *pos == '.') {
          ++trailing_zeros;
        } else {
          break;
        }
      }
      s.chop(trailing_zeros);
    }
    if(DECIMAL_PATTERN.indexIn(s, 0) != -1) {
      auto captures = DECIMAL_PATTERN.capturedTexts();
      if(m_trailing_zeros != 0) {
        if(captures[TRAILING_CAPTURE_GROUP].isEmpty()) {
          s += ".";
        }
      }
      auto has_update =
        update_leading_zeros(s, captures[LEADING_DIGITS_CAPTURE_GROUP]);
      has_update |=
        update_trailing_zeros(s, captures[TRAILING_DIGITS_CAPTURE_GROUP]);
    }
    return s;
  }

  void on_current(const optional<Decimal>& current) {
    update_decimal_places();
    m_current = to_string(current);
    m_update_signal(m_current);
  }
};

struct DecimalBox::StepButton : QWidget {
  using PressSignal = Signal<void()>;
  mutable PressSignal m_press_signal;
  FocusObserver m_focus_observer;
  int m_repeat_delay_timer_id;
  int m_repeat_timer_id;

  StepButton(QImage icon, QWidget& parent)
      : QWidget(&parent),
        m_focus_observer(parent),
        m_repeat_delay_timer_id(-1),
        m_repeat_timer_id(-1) {
    auto button_icon = new Icon(std::move(icon));
    auto box = new Box(button_icon);
    proxy_style(*this, *box);
    link(*this, *button_icon);
    enclose(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any() > is_a<Icon>()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(Fill(QColor(0x333333)));
      style.get(Hover() > is_a<Icon>()).
        set(BackgroundColor(QColor(0xEBEBEB))).
        set(Fill(QColor(0x4B23A0)));
      style.get(Press() > is_a<Icon>()).
        set(BackgroundColor(QColor(0xEBEBEB))).
        set(Fill(QColor(0x7E71B8)));
      style.get(Disabled() > is_a<Icon>()).
        set(BackgroundColor(QColor(Qt::transparent))).
        set(Fill(QColor(0xC8C8C8)));
    });
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(BUTTON_SIZE());
    m_focus_observer.connect_state_signal(
      std::bind_front(&StepButton::on_focus, this));
  }

  connection connect_press_signal(const PressSignal::slot_type& slot) const {
    return m_press_signal.connect(slot);
  }

  void mousePressEvent(QMouseEvent* event) override {
    if(QWidget* parent = parentWidget()) {
      parent->setFocus();
    }
    if(event->button() == Qt::LeftButton) {
      event->accept();
      match(*this, Press());
      m_press_signal();
      m_repeat_delay_timer_id = startTimer(REPEAT_DELAY_TIMEOUT_MS);
    } else {
      QWidget::mousePressEvent(event);
    }
  }

  void mouseReleaseEvent(QMouseEvent* event) override {
    if(event->button() == Qt::LeftButton) {
      event->accept();
      reset();
    } else {
      QWidget::mouseReleaseEvent(event);
    }
  }

  void timerEvent(QTimerEvent* event) override {
    if(event->timerId() == m_repeat_delay_timer_id) {
      killTimer(m_repeat_delay_timer_id);
      m_repeat_delay_timer_id = -1;
      m_repeat_timer_id = startTimer(REPEAT_TIMEOUT_MS);
    } else if(event->timerId() == m_repeat_timer_id) {
      if(isEnabled() && isVisible() &&
          rect().contains(mapFromGlobal(QCursor::pos()))) {
        m_press_signal();
      } else {
        reset();
      }
    } else {
      QWidget::timerEvent(event);
    }
  }

  void reset() {
    unmatch(*this, Press());
    if(m_repeat_delay_timer_id != -1) {
      killTimer(m_repeat_delay_timer_id);
      m_repeat_delay_timer_id = -1;
    }
    if(m_repeat_timer_id != -1) {
      killTimer(m_repeat_timer_id);
      m_repeat_timer_id = -1;
    }
  }

  void on_focus(const FocusObserver::State& state) {
    if(state == FocusObserver::State::NONE) {
      reset();
    }
  }
};

QValidator::State DecimalBox::validate(const Decimal& value,
    const optional<Decimal>& min, const optional<Decimal>& max) {
  auto validate_fractional_part = [] (const Decimal& value,
      const Decimal& fractional_part) {
    auto decimal_places = 0;
    auto v = Decimal(value - trunc(value));
    while(v != 0) {
      v *= 10;
      v = v - trunc(v);
      ++decimal_places;
    }
    return ::validate_fractional_part(fractional_part, decimal_places);
  };
  if(min && max && value >= min && value <= max ||
      min && !max && value >= min || !min && max && value <= max ||
      !min && !max) {
    return QValidator::State::Acceptable;
  } else if(max && (*max >= 0 && value > *max || *max < 0 && value > 0) ||
      min && (*min <= 0 && value < *min || *min > 0 && value < 0)) {
    return QValidator::State::Invalid;
  } else if(min && trunc(value) == trunc(*min)) {
    auto fractional_part = value - trunc(value);
    if(fractional_part == 0) {
      return QValidator::State::Intermediate;
    } else if(fractional_part < 0) {
      if(max && trunc(value) == trunc(*max)) {
        return validate_fractional_part(fractional_part, value - *max);
      }
    } else {
      return validate_fractional_part(fractional_part, *min - value);
    }
  } else if(max && trunc(value) == trunc(*max)) {
    auto fractional_part = value - trunc(value);
    if(fractional_part == 0) {
      return QValidator::State::Intermediate;
    } else if(fractional_part > 0) {
      if(min && trunc(value) == trunc(*min)) {
        return validate_fractional_part(fractional_part, *min - value);
      }
    } else {
      return validate_fractional_part(fractional_part, value - *max);
    }
  } else if(!max && validate(value, Decimal(trunc(*min / 10)), none) ||
      !min && validate(value, none, Decimal(trunc(*max / 10))) ||
      validate(value, Decimal(trunc(*min / 10)), Decimal(trunc(*max / 10)))) {
    return QValidator::State::Intermediate;
  }
  return QValidator::State::Invalid;
}

DecimalBox::DecimalBox(QWidget* parent)
  : DecimalBox(std::make_shared<LocalOptionalDecimalModel>(), parent) {}

DecimalBox::DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
  QWidget* parent)
  : DecimalBox(std::make_shared<LocalOptionalDecimalModel>(),
      std::move(modifiers), parent) {}

DecimalBox::DecimalBox(
  std::shared_ptr<OptionalDecimalModel> current, QWidget* parent)
  : DecimalBox(current, make_modifiers(*current), parent) {}

DecimalBox::DecimalBox(std::shared_ptr<OptionalDecimalModel> current,
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_adaptor_model(std::make_shared<DecimalToTextModel>(m_current)),
      m_modifiers(std::move(modifiers)),
      m_text_box(m_adaptor_model, this),
      m_tick(TickIndicator::NONE) {
  enclose(*this, m_text_box);
  proxy_style(*this, m_text_box);
  update_style(*this, [] (auto& style) {
    style.get(ReadOnly() > is_a<StepButton>()).set(Visibility::NONE);
    style.get(+Any() > (is_a<StepButton>() && !matches(Visibility::NONE))).
      set(PaddingRight(scale_width(24)));
  });
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&DecimalBox::on_style, this));
  setFocusProxy(&m_text_box);
  if(auto current = m_current->get()) {
    if(*current > 0) {
      m_sign = SignIndicator::POSITIVE;
      match(*this, IsPositive());
    } else if(*current < 0) {
      m_sign = SignIndicator::NEGATIVE;
      match(*this, IsNegative());
    } else {
      m_sign = SignIndicator::NONE;
    }
  }
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&DecimalBox::on_current, this));
}

const std::shared_ptr<OptionalDecimalModel>& DecimalBox::get_current() const {
  return m_current;
}

std::shared_ptr<const TextModel> DecimalBox::get_text() const {
  return m_text_box.get_current();
}

void DecimalBox::set_placeholder(const QString& value) {
  m_text_box.set_placeholder(value);
}

bool DecimalBox::is_read_only() const {
  return m_text_box.is_read_only();
}

void DecimalBox::set_read_only(bool is_read_only) {
  if(is_read_only == this->is_read_only()) {
    return;
  }
  m_text_box.set_read_only(is_read_only);
  if(is_read_only) {
    match(*this, ReadOnly());
  } else {
    initialize_editable_data();
    unmatch(*this, ReadOnly());
  }
}

connection DecimalBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  initialize_editable_data();
  return m_data->m_submit_signal.connect(slot);
}

connection DecimalBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  initialize_editable_data();
  return m_data->m_reject_signal.connect(slot);
}

void DecimalBox::keyPressEvent(QKeyEvent* event) {
  if(!is_read_only()) {
    if(event->key() == Qt::Key_Up) {
      increment();
      return;
    } else if(event->key() == Qt::Key_Down) {
      decrement();
      return;
    }
  }
  QWidget::keyPressEvent(event);
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  QWidget::resizeEvent(event);
}

void DecimalBox::showEvent(QShowEvent* event) {
  if(!is_read_only()) {
    initialize_editable_data();
  }
  QWidget::showEvent(event);
}

void DecimalBox::wheelEvent(QWheelEvent* event) {
  if(hasFocus() && !is_read_only()) {
    auto angle_delta = [&] {
      if(event->modifiers().testFlag(Qt::AltModifier)) {
        return event->angleDelta().x();
      } else {
        return event->angleDelta().y();
      }
    }();
    if(angle_delta > 0) {
      increment();
    } else if(angle_delta < 0) {
      decrement();
    }
  }
  QWidget::wheelEvent(event);
}

void DecimalBox::initialize_editable_data() const {
  if(m_data) {
    return;
  }
  auto self = const_cast<DecimalBox*>(this);
  self->m_data = std::make_unique<EditableData>();
  m_data->m_submission = m_current->get();
  static auto up_icon = imageFromSvg(":/Icons/arrow-up.svg", BUTTON_SIZE());
  m_data->m_up_button = new StepButton(up_icon, self->m_text_box);
  m_data->m_up_button->show();
  m_data->m_up_button->connect_press_signal(
    std::bind_front(&DecimalBox::increment, self));
  link(*self, *m_data->m_up_button);
  match(*m_data->m_up_button, UpButton());
  static auto down_icon = imageFromSvg(":/Icons/arrow-down.svg", BUTTON_SIZE());
  m_data->m_down_button = new StepButton(down_icon, self->m_text_box);
  m_data->m_down_button->show();
  m_data->m_down_button->connect_press_signal(
    std::bind_front(&DecimalBox::decrement, self));
  link(*self, *m_data->m_down_button);
  match(*m_data->m_down_button, DownButton());
  self->update_button_positions();
  self->update_button_states();
  m_text_box.connect_submit_signal(
    std::bind_front(&DecimalBox::on_submit, self));
  m_text_box.connect_reject_signal(
    std::bind_front(&DecimalBox::on_reject, self));
}

void DecimalBox::decrement() {
  step_by(-get_increment());
}

void DecimalBox::increment() {
  step_by(get_increment());
}

Decimal DecimalBox::get_increment() const {
  auto modifier_flags = static_cast<int>(qApp->keyboardModifiers());
  if(auto increment =
      m_modifiers.find(static_cast<Qt::KeyboardModifier>(modifier_flags));
      increment != m_modifiers.end()) {
    return increment.value();
  }
  return m_modifiers[Qt::NoModifier];
}

void DecimalBox::step_by(const Decimal& value) {
  auto current = [&] {
    if(m_current->get()) {
      return *m_current->get();
    } else if(!m_current->get_minimum() && !m_current->get_maximum() ||
        !m_current->get_minimum() && *m_current->get_maximum() >= 0 ||
        !m_current->get_maximum() && *m_current->get_minimum() <= 0 ||
        *m_current->get_minimum() <= 0 && *m_current->get_maximum() >= 0) {
      return Decimal(0);
    } else if(abs(*m_current->get_minimum()) < abs(*m_current->get_maximum())) {
      return *m_current->get_minimum();
    } else {
      return *m_current->get_maximum();
    }
  }();
  auto next = Decimal(current + value);
  if(m_current->get_minimum() && next < m_current->get_minimum()) {
    next = *m_current->get_minimum();
  } else if(m_current->get_maximum() && next > m_current->get_maximum()) {
    next = *m_current->get_maximum();
  }
  if(next != m_current->get()) {
    m_current->set(next);
  }
}

void DecimalBox::update_button_positions() {
  if(!m_data) {
    return;
  }
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_data->m_up_button->move(
    button_pos.x(), button_pos.y() - m_data->m_up_button->height());
  m_data->m_down_button->move(button_pos);
}

void DecimalBox::update_button_states() {
  if(!m_data) {
    return;
  }
  m_data->m_up_button->setEnabled(!is_read_only() &&
    (!m_current->get_maximum() ||
      !m_current->get() || m_current->get() < m_current->get_maximum()));
  m_data->m_down_button->setEnabled(!is_read_only() &&
    (!m_current->get_minimum() ||
      !m_current->get() || m_current->get() > m_current->get_minimum()));
}

void DecimalBox::on_current(const optional<Decimal>& current) {
  if(m_last_current && current) {
    if(m_tick == TickIndicator::DOWN) {
      unmatch(*this, Downtick());
    } else if(m_tick == TickIndicator::UP) {
      unmatch(*this, Uptick());
    }
    if(*current > *m_last_current) {
      m_tick = TickIndicator::UP;
      match(*this, Uptick());
    } else if(*current < *m_last_current) {
      m_tick = TickIndicator::DOWN;
      match(*this, Downtick());
    }
  }
  if(current) {
    m_last_current = current;
    if(*current > 0 && m_sign != SignIndicator::POSITIVE) {
      if(m_sign == SignIndicator::NEGATIVE) {
        unmatch(*this, IsNegative());
      }
      match(*this, IsPositive());
      m_sign = SignIndicator::POSITIVE;
    } else if(*current == 0 && m_sign != SignIndicator::NONE) {
      if(m_sign == SignIndicator::NEGATIVE) {
        unmatch(*this, IsNegative());
      } else if(m_sign == SignIndicator::POSITIVE) {
        unmatch(*this, IsPositive());
      }
      m_sign = SignIndicator::NONE;
    } else if(*current < 0 && m_sign != SignIndicator::NEGATIVE) {
      if(m_sign == SignIndicator::POSITIVE) {
        unmatch(*this, IsPositive());
      }
      match(*this, IsNegative());
      m_sign = SignIndicator::NEGATIVE;
    }
  }
  update_button_states();
}

void DecimalBox::on_submit(const QString& submission) {
  static auto VALIDATOR = QRegExp("^([-|\\+]?[0-9]+(\\.[0-9]*)?)|(-\\.)");
  if(VALIDATOR.exactMatch(submission)) {
    m_data->m_submission = m_adaptor_model->submit();
    m_data->m_submit_signal(m_data->m_submission);
  }
}

void DecimalBox::on_reject(const QString& value) {
  m_data->m_reject_signal(text_to_decimal(value).value_or(Decimal(0)));
  m_adaptor_model->reject();
}

void DecimalBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  if(auto leading_zeros = Styles::find<LeadingZeros>(block)) {
    stylist.evaluate(*leading_zeros, [=] (auto leading_zeros) {
      m_adaptor_model->set_leading_zeros(leading_zeros);
    });
  }
  if(auto trailing_zeros = Styles::find<TrailingZeros>(block)) {
    stylist.evaluate(*trailing_zeros, [=] (auto trailing_zeros) {
      m_adaptor_model->set_trailing_zeros(trailing_zeros);
    });
  }
}
