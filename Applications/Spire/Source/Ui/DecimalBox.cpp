#include "Spire/Ui/DecimalBox.hpp"
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;

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
    return std::clamp(value, min, max, [] (const DecimalBox::Decimal& a,
        const DecimalBox::Decimal& b) {
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
    return QString::fromStdString(value.str(DecimalBox::PRECISION,
      std::ios_base::dec));
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
    auto style = IconButton::Style();
    style.m_default_background_color = QColor("#FFFFFF");
    style.m_hover_background_color = QColor("#EBEBEB");
    style.m_hover_color = QColor("#4B23A0");
    style.m_disabled_color = QColor("#C8C8C8");
    auto button = new IconButton(imageFromSvg(icon, BUTTON_SIZE()), style,
      parent);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedSize(BUTTON_SIZE());
    return button;
  }
}

DecimalBox::DecimalBox(Decimal initial, Decimal minimum, Decimal maximum,
    QHash<Qt::KeyboardModifier, Decimal> modifiers,
    QWidget* parent)
    : QWidget(parent),
      m_submission(initial),
      m_minimum(minimum),
      m_maximum(maximum),
      m_modifiers(std::move(modifiers)),
      m_decimal_places(DEFAULT_DECIMAL_PLACES),
      m_has_trailing_zeros(false),
      m_validator(nullptr) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(to_string(initial), this);
  update_input_validator();
  auto padding = m_text_box->get_padding();
  padding.m_right_padding = scale_width(26);
  m_text_box->set_padding(padding);
  setFocusProxy(m_text_box);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_text_box);
  m_current_connection = m_text_box->connect_current_signal(
    [=] (const auto& current) { m_current_signal(to_decimal(current)); });
  m_submit_connection = m_text_box->connect_submit_signal(
    [=] (const auto& submit) { on_submit(); });
  m_text_box->installEventFilter(this);
  m_up_button = create_button(":/Icons/arrow-up.svg", this);
  connect(m_up_button, &IconButton::clicked, this, &DecimalBox::increment);
  m_down_button = create_button(":/Icons/arrow-down.svg", this);
  connect(m_down_button, &IconButton::clicked, this, &DecimalBox::decrement);
  update_button_positions();
}

bool DecimalBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
      if(e->key() == Qt::Key_Up) {
        increment();
      } else {
        decrement();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  QWidget::resizeEvent(event);
}

DecimalBox::Decimal DecimalBox::get_current() const {
  return to_decimal(m_text_box->get_text());
}

void DecimalBox::set_current(Decimal current) {
  if(m_minimum <= current && current <= m_maximum) {
    m_text_box->set_text(to_string(current));
  }
}

DecimalBox::Decimal DecimalBox::get_minimum() const {
  return m_minimum;
}

void DecimalBox::set_minimum(Decimal minimum) {
  m_minimum = minimum;
  if(to_decimal(m_text_box->get_text()) < m_minimum) {
    m_text_box->set_text(to_string(m_minimum));
  }
}

DecimalBox::Decimal DecimalBox::get_maximum() const {
  return m_maximum;
}

void DecimalBox::set_maximum(Decimal maximum) {
  m_maximum = maximum;
  if(to_decimal(m_text_box->get_text()) > m_maximum) {
    m_text_box->set_text(to_string(m_maximum));
  }
}

DecimalBox::Decimal DecimalBox::get_increment(
    Qt::KeyboardModifier modifier) const {
  static auto SENTINEL = Decimal(0);
  auto increment = m_modifiers[modifier];
  if(increment == Decimal()) {
    return SENTINEL;
  }
  return increment;
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
  // TODO: update
}

bool DecimalBox::has_trailing_zeros() const {
  return m_has_trailing_zeros;
}

void DecimalBox::set_trailing_zeros(bool has_trailing_zeros) {
  m_has_trailing_zeros = has_trailing_zeros;
  // TODO: update
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->setReadOnly(is_read_only);
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

void DecimalBox::decrement() {
  auto current = get_current();
  set_current(clamp(current -= get_increment(), m_minimum, m_maximum));
}

void DecimalBox::increment() {
  auto increment = get_increment();
  set_current(clamp(increment += get_current(), m_minimum, m_maximum));
}

DecimalBox::Decimal DecimalBox::get_increment() const {
  if(qApp->keyboardModifiers().testFlag(Qt::ShiftModifier)) {
    return m_modifiers[Qt::ShiftModifier];
  } else if(qApp->keyboardModifiers().testFlag(Qt::ControlModifier)) {
    return m_modifiers[Qt::ControlModifier];
  } else if(qApp->keyboardModifiers().testFlag(Qt::AltModifier)) {
    return m_modifiers[Qt::AltModifier];
  } else if(m_modifiers[Qt::NoModifier] != Decimal()) {
    return m_modifiers[Qt::NoModifier];
  }
  return Decimal(0);
}

void DecimalBox::update_button_positions() {
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_up_button->move(button_pos.x(), button_pos.y() - m_up_button->height());
  m_down_button->move(button_pos);
}

void DecimalBox::update_input_validator() {
  auto decimal_regex = [&] () -> QString {
    if(m_decimal_places == 0) {
      return {};
    }
    return QString("[%1]?[0-9]{0,%2}").arg(QLocale().decimalPoint()).
      arg(m_decimal_places);
  }();
  if(m_validator) {
    delete_later(m_validator);
  }
  m_validator = new QRegExpValidator(
    QRegExp(QString("^[-]?[0-9]*%1$").arg(decimal_regex)), m_text_box);
  m_text_box->setValidator(m_validator);
}

void DecimalBox::on_submit() const {
  auto current = get_current();
  if(m_minimum <= current && current <= m_maximum) {
    // TODO: why does this assignment fail?
    //m_submission = current;
    m_submit_signal(m_submission);
  } else {
    m_text_box->set_text(to_string(m_submission));
    m_text_box->play_warning();
  }
}
