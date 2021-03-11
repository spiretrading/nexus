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
      m_has_trailing_zeros(false),
      m_validator(nullptr),
      m_trailing_zero_regex(QString("[%1]?[0]*$").arg(
        QLocale().decimalPoint())) {
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
  m_text_box->installEventFilter(this);
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
}

DecimalBox::Decimal DecimalBox::get_maximum() const {
  return m_maximum;
}

void DecimalBox::set_maximum(Decimal maximum) {
  m_maximum = maximum;
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
  m_validator = QRegExp(QString("^[-]?[0-9]*([%1][0-9]{0,%2})?").arg(
    QLocale().decimalPoint()).arg(m_decimal_places));
  update_trailing_zeros();
}

bool DecimalBox::has_trailing_zeros() const {
  return m_has_trailing_zeros;
}

void DecimalBox::set_trailing_zeros(bool has_trailing_zeros) {
  m_has_trailing_zeros = has_trailing_zeros;
  update_trailing_zeros();
}

void DecimalBox::set_buttons_visible(bool are_visible) {
  m_up_button->setVisible(are_visible);
  m_down_button->setVisible(are_visible);
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
  m_up_button->setVisible(!is_read_only);
  m_down_button->setVisible(!is_read_only);
}

connection DecimalBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection DecimalBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
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
  setFocus();
  value += get_current();
  set_current(clamp(value, m_minimum, m_maximum));
  update_trailing_zeros();
}

void DecimalBox::update_button_positions() {
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_up_button->move(button_pos.x(), button_pos.y() - m_up_button->height());
  m_down_button->move(button_pos);
}

void DecimalBox::update_trailing_zeros() {
  auto current_text = m_text_box->get_current();
  if(!m_has_trailing_zeros || m_decimal_places == 0) {
    auto block = shared_connection_block(m_current_connection);
    m_text_box->set_current(current_text.remove(m_trailing_zero_regex));
    return;
  }
  auto point_index = current_text.indexOf(QLocale().decimalPoint());
  if(point_index == -1 && m_decimal_places > 0) {
    current_text.append(QLocale().decimalPoint());
  }
  auto zero_count = [&] {
    if(point_index == -1) {
      return m_decimal_places;
    }
    return m_decimal_places - current_text.length() + point_index + 1;
  }();
  auto blocker = shared_connection_block(m_current_connection);
  if(zero_count > 0) {
    m_text_box->set_current(current_text.leftJustified(
      current_text.length() + zero_count, '0'));
  } else {
    m_text_box->set_current(current_text.left(
      current_text.length() + zero_count));
  }
}

void DecimalBox::on_current(const QString& current) {
  if(m_validator.exactMatch(current)) {
    m_current = current;
    auto value = to_decimal(current);
    m_up_button->setDisabled(value >= m_maximum);
    m_down_button->setDisabled(value <= m_minimum);
    m_current_signal(value);
  } else {
    auto blocker = shared_connection_block(m_current_connection);
    m_text_box->set_current(m_current);
  }
}

void DecimalBox::on_submit(const QString& submission) {
  auto value = to_decimal(submission);
  if(m_minimum <= value && value <= m_maximum) {
    update_trailing_zeros();
    m_submission = value;
    m_submit_signal(m_submission);
  } else {
    m_text_box->set_current(to_string(m_submission));
    update_trailing_zeros();
    display_warning_indicator(*m_text_box);
  }
}
