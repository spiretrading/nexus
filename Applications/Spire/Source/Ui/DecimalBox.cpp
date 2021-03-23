#include "Spire/Ui/DecimalBox.hpp"
#include <bitset>
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DEFAULT_DECIMAL_PLACES = 6;

  bool operator <(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return lhs.compare(rhs) == -1;
  }

  bool operator <=(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return lhs.compare(rhs) != 1;
  }

  bool operator ==(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return lhs.compare(rhs) == 0;
  }

  bool operator !=(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return !(lhs == rhs);
  }

  bool operator >(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return !(lhs <= rhs);
  }

  bool operator >=(DecimalBox::Decimal lhs, DecimalBox::Decimal rhs) {
    return !(lhs < rhs);
  }

  auto clamp(DecimalBox::Decimal value, DecimalBox::Decimal min,
      DecimalBox::Decimal max) {
    return std::clamp(value, min, max,
      [] (const auto& a, const auto& b) {
        return a < b;
      });
  }

  DecimalBox::Decimal to_decimal(const QString& text) {
    try {
      return DecimalBox::Decimal(text.toStdString().c_str());
    } catch (const std::runtime_error&) {
      return DecimalBox::Decimal::nan();
    }
  }

  QString to_string(DecimalBox::Decimal value) {
    return QString::fromStdString(
      value.str(DecimalBox::PRECISION, std::ios_base::dec));
  }

  auto BUTTON_RIGHT_PADDING() {
    static auto padding = scale_width(6);
    return padding;
  }

  auto BUTTON_SIZE() {
    static auto size = scale(10, 10);
    return size;
  }

  auto create_button(const QString& icon, QWidget* parent) {
    auto button = make_icon_button(imageFromSvg(icon, BUTTON_SIZE()), parent);
    auto style = button->get_style();
    style.get(Any()).
      set(BackgroundColor(QColor("#FFFFFF"))).
      set(Fill(QColor("#333333")));
    style.get(Hover()).
      set(BackgroundColor(QColor("#EBEBEB"))).
      set(Fill(QColor("#4B23A0")));
    style.get(Disabled()).
      set(BackgroundColor(QColor("#00000000"))).
      set(Fill(QColor("#C8C8C8")));
    style.get(Any() < ReadOnly()).set(Visibility(VisibilityOption::NONE));
    button->set_style(std::move(style));
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedSize(BUTTON_SIZE());
    return button;
  }
}

DecimalBox::DecimalBox(Decimal current, Decimal minimum, Decimal maximum,
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
    : StyledWidget(parent),
      m_current(to_string(current)),
      m_submission(current),
      m_minimum(minimum),
      m_maximum(maximum),
      m_modifiers(std::move(modifiers)),
      m_leading_zeros(0),
      m_trailing_zeros(0),
      m_validator(QString("^[-]?[0-9]*([%1][0-9]*)?").arg(
        QLocale().decimalPoint())),
      m_has_warning(true) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(m_current, this);
  set_decimal_places(DEFAULT_DECIMAL_PLACES);
  auto style = m_text_box->get_style();
  style.get(Any()).set(PaddingRight(scale_width(26)));
  m_text_box->set_style(std::move(style));
  setFocusProxy(m_text_box);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_text_box);
  m_current_connection = m_text_box->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
  m_submit_connection = m_text_box->connect_submit_signal(
    [=] (const auto& submit) { on_submit(submit); });
  m_text_box->findChild<QLineEdit*>()->installEventFilter(this);
  m_up_button = create_button(":/Icons/arrow-up.svg", this);
  m_up_button->connect_clicked_signal([=] { increment(); });
  m_down_button = create_button(":/Icons/arrow-down.svg", this);
  m_down_button->connect_clicked_signal([=] { decrement(); });
  update_button_positions();
}

DecimalBox::Decimal DecimalBox::get_current() const {
  return to_decimal(m_text_box->get_current());
}

void DecimalBox::set_current(Decimal current) {
  m_text_box->set_current(to_string(current));
}

DecimalBox::Decimal DecimalBox::get_minimum() const {
  return m_minimum;
}

void DecimalBox::set_minimum(Decimal minimum) {
  m_minimum = minimum;
  m_down_button->setDisabled(get_current() <= m_minimum);
}

DecimalBox::Decimal DecimalBox::get_maximum() const {
  return m_maximum;
}

void DecimalBox::set_maximum(Decimal maximum) {
  m_maximum = maximum;
  m_up_button->setDisabled(get_current() >= m_maximum);
}

DecimalBox::Decimal DecimalBox::get_increment(
    Qt::KeyboardModifier modifier) const {
  auto increment = m_modifiers.find(modifier);
  if(increment == m_modifiers.end()) {
    return get_increment(Qt::NoModifier);
  }
  return increment.value();
}

void DecimalBox::set_increment(Qt::KeyboardModifier modifier,
    Decimal increment) {
  m_modifiers[modifier] = increment;
}

int DecimalBox::get_decimal_places() const {
  return m_decimal_places;
}

void DecimalBox::set_decimal_places(int decimal_places) {
  m_decimal_places = decimal_places;
  update_padded_zeros();
}

void DecimalBox::set_leading_zeros(int leading_zeros) {
  m_leading_zeros = leading_zeros;
  update_padded_zeros();
}

void DecimalBox::set_trailing_zeros(int trailing_zeros) {
  m_trailing_zeros = trailing_zeros;
  update_padded_zeros();
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
}

void DecimalBox::set_warning(bool has_warning) {
  m_has_warning = has_warning;
}

connection DecimalBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection DecimalBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DecimalBox::test_selector(
    const Selector& element, const Selector& selector) const {
  return m_text_box->test_selector(element, selector);
}

bool DecimalBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent*>(event);
    if(keyEvent->key() == Qt::Key_Up) {
      increment();
    } else if(keyEvent->key() == Qt::Key_Down) {
      decrement();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  QWidget::resizeEvent(event);
}

void DecimalBox::wheelEvent(QWheelEvent* event) {
  if(hasFocus()) {
    auto angle_delta = [&] {
      if(event->modifiers().testFlag(Qt::AltModifier)) {
        return event->angleDelta().x();
      }
      return event->angleDelta().y();
    }();
    if(angle_delta > 0) {
      increment();
    } else {
      decrement();
    }
  }
  QWidget::wheelEvent(event);
}

void DecimalBox::decrement() {
  auto increment = get_increment();
  increment.negate();
  step_by(increment);
}

void DecimalBox::increment() {
  step_by(get_increment());
}

DecimalBox::Decimal DecimalBox::get_increment() const {
  auto modifier_flags = static_cast<int>(qApp->keyboardModifiers());
  auto modifiers =
    std::bitset<std::numeric_limits<int>::digits>(modifier_flags);
  if(modifiers.count() != 1) {
    return get_increment(Qt::NoModifier);
  }
  return get_increment(
    static_cast<Qt::KeyboardModifier>(modifiers.to_ulong()));
}

void DecimalBox::step_by(Decimal value) {
  if(m_text_box->is_read_only() || !isEnabled()) {
    return;
  }
  setFocus();
  value += get_current();
  set_current(clamp(value, m_minimum, m_maximum));
  update_padded_zeros();
}

void DecimalBox::update_button_positions() {
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_up_button->move(button_pos.x(), button_pos.y() - m_up_button->height());
  m_down_button->move(button_pos);
}

void DecimalBox::update_padded_zeros() {
  auto sign = [&] {
    if(get_current().isneg()) {
      return QString(QLocale().negativeSign());
    }
    return QString();
  }();
  auto integer = [&] {
    if(get_current().extract_integer_part().isnan()) {
      return QString(m_leading_zeros, '0');
    }
    return to_string(get_current().extract_integer_part()).remove(
      QLocale().negativeSign()).rightJustified(m_leading_zeros, '0');
  }();
  auto decimal = [&] {
    if(m_decimal_places <= 0) {
      return QString();
    }
    auto decimal = to_string(get_current()).remove(QRegExp("^-*[0-9]*"));
    if(decimal == "nan" || decimal.isEmpty()) {
      decimal.clear();
      decimal.append(QLocale().decimalPoint());
    }
    decimal = decimal.leftJustified(m_trailing_zeros + 1, '0');
    if(decimal.endsWith(QLocale().decimalPoint())) {
      return QString();
    }
    return decimal;
  }();
  auto blocker = shared_connection_block(m_current_connection);
  m_text_box->set_current(QString("%1%2%3").arg(sign, integer, decimal));
}

void DecimalBox::on_current(const QString& current) {
  auto blocker = shared_connection_block(m_current_connection);
  if(m_validator.exactMatch(current)) {
    if(auto point_index = current.indexOf(QLocale().decimalPoint());
        point_index != -1 &&
        current.length() > (point_index + m_decimal_places + 1)) {
      m_current = current.left(point_index + m_decimal_places + 1);
      auto line_edit = m_text_box->findChild<QLineEdit*>();
      auto old_cursor_index = line_edit->cursorPosition();
      m_text_box->set_current(m_current);
      line_edit->setCursorPosition(old_cursor_index);
    } else {
      m_current = current;
      m_text_box->set_current(m_current);
    }
    auto value = to_decimal(m_current);
    m_up_button->setDisabled(value >= m_maximum);
    m_down_button->setDisabled(value <= m_minimum);
    m_current_signal(value);
  } else {
    m_text_box->set_current(m_current);
  }
}

void DecimalBox::on_submit(const QString& submission) {
  auto value = to_decimal(submission);
  if(m_minimum <= value && value <= m_maximum) {
    update_padded_zeros();
    m_submission = value;
    m_submit_signal(m_submission);
  } else {
    m_text_box->set_current(to_string(m_submission));
    update_padded_zeros();
    if(m_has_warning) {
      display_warning_indicator(*m_text_box);
    }
  }
}
