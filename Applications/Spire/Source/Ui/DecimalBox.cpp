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
#include "Spire/Ui/LocalScalarValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  optional<DecimalBox::Decimal> to_decimal(const QString& text) {
    try {
      return DecimalBox::Decimal(text.toStdString().c_str());
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

  auto create_button(const QString& icon, QWidget* parent) {
    auto button = make_icon_button(imageFromSvg(icon, BUTTON_SIZE()), parent);
    auto style = get_style(*button);
    style.get(Any() > Button::Body()).
      set(BackgroundColor(QColor("#FFFFFF"))).
      set(Fill(QColor("#333333")));
    style.get(Hover() > Button::Body()).
      set(BackgroundColor(QColor("#EBEBEB"))).
      set(Fill(QColor("#4B23A0")));
    style.get(Disabled() > Button::Body()).
      set(BackgroundColor(QColor("#00000000"))).
      set(Fill(QColor("#C8C8C8")));
    style.get(+Any() < ReadOnly()).set(Visibility(VisibilityOption::NONE));
    set_style(*button, std::move(style));
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedSize(BUTTON_SIZE());
    return button;
  }

  QValidator::State is_acceptable(const DecimalBox::Decimal& value,
      optional<DecimalBox::Decimal> min, optional<DecimalBox::Decimal> max) {
    if(max && *max >= 0 && value > *max || min && *min <= 0 && value < *min) {
      return QValidator::State::Invalid;
    }
    if(min && value < *min) {
      if(value == 0 || abs(*min - value) < 10) {
        return QValidator::State::Invalid;
      }
      auto adjustment = value;
      while(*min - adjustment > 10) {
        adjustment *= 10;
      }
      if(abs(*min - adjustment) < 10) {
        return QValidator::State::Intermediate;
      }
      return QValidator::State::Invalid;
    } else if(*max && value > *max) {
      if(min) {
        return is_acceptable(-value, optional<DecimalBox::Decimal>(-*min),
          optional<DecimalBox::Decimal>(-*max));
      }
      return is_acceptable(-value, none, optional<DecimalBox::Decimal>(-*max));
    }
    return QValidator::State::Acceptable;
  }
}

struct DecimalBox::DecimalToTextModel : TextModel {
  mutable CurrentSignal m_current_signal;
  std::shared_ptr<DecimalBox::DecimalModel> m_model;
  int m_decimal_places;
  int m_leading_zeros;
  int m_trailing_zeros;
  QString m_current;
  QRegExp m_validator;
  bool m_is_rejected;
  scoped_connection m_current_connection;

  DecimalToTextModel(std::shared_ptr<DecimalBox::DecimalModel> model,
      InitialDisplay initial_display)
      : m_model(std::move(model)),
        m_decimal_places(-log10(m_model->get_increment()).convert_to<int>()),
        m_leading_zeros(0),
        m_trailing_zeros(0),
        m_is_rejected(false),
        m_current_connection(m_model->connect_current_signal(
          [=] (const auto& current) {
            on_current(current);
          })) {
    if(initial_display == InitialDisplay::CURRENT) {
      m_current = to_string(m_model->get_current());
    }
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
    auto displayed_value = to_string(m_model->get_current());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_current_signal(m_current);
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
    auto displayed_value = to_string(m_model->get_current());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_current_signal(m_current);
    }
  }

  DecimalBox::Decimal submit() {
    auto displayed_value = to_string(m_model->get_current());
    if(displayed_value != m_current) {
      m_current = std::move(displayed_value);
      m_current_signal(m_current);
    }
    return m_model->get_current();
  }

  void reject() {
    m_is_rejected = true;
  }

  QValidator::State get_state() const override {
    return m_model->get_state();
  }

  const QString& get_current() const {
    return m_current;
  }

  QValidator::State set_current(const QString& value) override {
    auto decimal_places = 0;
    auto i = m_model->get_increment();
    while(i < 1) {
      i *= 10;
      ++decimal_places;
    }
    if(decimal_places != m_decimal_places) {
      m_decimal_places = decimal_places;
      update_validator();
    }
    if(!m_validator.exactMatch(value)) {
      m_is_rejected = false;
      return QValidator::State::Invalid;
    } else if(value.isEmpty() || value == "-" || value == "+") {
      if(value == "-" &&
          m_model->get_minimum() && *m_model->get_minimum() >= 0) {
        return QValidator::State::Invalid;
      } else if(value == "+" &&
          m_model->get_maximum() && *m_model->get_maximum() < 0) {
        return QValidator::State::Invalid;
      }
      auto origin = [&] {
        if(is_acceptable(0, m_model->get_minimum(), m_model->get_maximum()) ==
            QValidator::State::Acceptable) {
          return DecimalBox::Decimal(0);
        } else if(!m_model->get_minimum()) {
          return *m_model->get_maximum();
        }
        return *m_model->get_minimum();
      }();
      auto blocker = shared_connection_block(m_current_connection);
      m_model->set_current(origin);
      m_current = value;
      m_current_signal(m_current);
      m_is_rejected = false;
      return QValidator::State::Intermediate;
    } else if(auto decimal = to_decimal(value)) {
      auto state =
        is_acceptable(*decimal, m_model->get_minimum(), m_model->get_maximum());
      if(state == QValidator::State::Invalid) {
        return QValidator::State::Invalid;
      }
      auto blocker = shared_connection_block(m_current_connection);
      state = m_model->set_current(*decimal);
      if(state != QValidator::State::Invalid) {
        if(m_is_rejected) {
          m_current = to_string(*decimal);
        } else {
          m_current = value;
        }
        m_current_signal(m_current);
      }
      m_is_rejected = false;
      return state;
    }
    m_is_rejected = false;
    return QValidator::State::Invalid;
  }

  connection connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const override {
    return m_current_signal.connect(slot);
  }

  void update_validator() {
    if(m_decimal_places <= 0) {
      m_validator = QRegExp(QString("^[-|\\+]?[0-9]*"));
    } else if(m_trailing_zeros > m_decimal_places) {
      auto delta = m_trailing_zeros - m_decimal_places;
      m_validator =
        QRegExp(QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1}0{0,%2})?").arg(
          m_decimal_places).arg(delta));
    } else {
      m_validator = QRegExp(
        QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1})?").arg(m_decimal_places));
    }
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

  QString to_string(const DecimalBox::Decimal& value) const {
    static auto DECIMAL_PATTERN = QRegExp("^([-|\\+]?([0-9]*))(\\.([0-9]*))?");
    static const auto LEADING_DIGITS_CAPTURE_GROUP = 2;
    static const auto TRAILING_CAPTURE_GROUP = 3;
    static const auto TRAILING_DIGITS_CAPTURE_GROUP = 4;
    auto s = QString::fromStdString(
      value.str(DecimalBox::PRECISION, std::ios_base::dec));
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

  void on_current(const DecimalBox::Decimal& current) {
    m_current = to_string(current);
    m_current_signal(m_current);
  }
};

DecimalBox::DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
  QWidget* parent)
  : DecimalBox(std::move(modifiers), InitialDisplay::CURRENT, parent) {}

DecimalBox::DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
  InitialDisplay initial_display, QWidget* parent)
  : DecimalBox(std::make_shared<LocalScalarValueModel<Decimal>>(),
      std::move(modifiers), initial_display, parent) {}

DecimalBox::DecimalBox(std::shared_ptr<DecimalModel> model,
  QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
  : DecimalBox(std::move(model), std::move(modifiers), InitialDisplay::CURRENT,
      parent) {}

DecimalBox::DecimalBox(std::shared_ptr<DecimalModel> model,
    QHash<Qt::KeyboardModifier, Decimal> modifiers,
    InitialDisplay initial_display, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_adaptor_model(
        std::make_shared<DecimalToTextModel>(m_model, initial_display)),
      m_submission(m_model->get_current()),
      m_modifiers(std::move(modifiers)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(m_adaptor_model, this);
  auto style = Spire::Styles::get_style(*m_text_box);
  style.get(+Any() %
    (is_a<Button>() && !matches(Visibility(VisibilityOption::NONE)))).set(
      PaddingRight(scale_width(26)));
  set_style(*m_text_box, std::move(style));
  proxy_style(*this, *m_text_box);
  connect_style_signal(*this, [=] { on_style(); });
  setFocusProxy(m_text_box);
  layout->addWidget(m_text_box);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
  m_submit_connection = m_text_box->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_reject_connection = m_text_box->connect_reject_signal(
    [=] (const auto& value) { on_reject(value); });
  m_up_button = create_button(":/Icons/arrow-up.svg", this);
  m_up_button->connect_clicked_signal([=] { increment(); });
  m_down_button = create_button(":/Icons/arrow-down.svg", this);
  m_down_button->connect_clicked_signal([=] { decrement(); });
  update_button_positions();
}

const std::shared_ptr<DecimalBox::DecimalModel>& DecimalBox::get_model() const {
  return m_model;
}

void DecimalBox::set_placeholder(const QString& value) {
  m_text_box->set_placeholder(value);
}

bool DecimalBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
}

bool DecimalBox::is_warning_displayed() const {
  return m_text_box->is_warning_displayed();
}

void DecimalBox::set_warning_displayed(bool is_displayed) {
  m_text_box->set_warning_displayed(is_displayed);
}

connection DecimalBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection DecimalBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void DecimalBox::keyPressEvent(QKeyEvent* event) {
  if(!is_read_only()) {
    if(event->key() == Qt::Key_Up) {
      increment();
    } else if(event->key() == Qt::Key_Down) {
      decrement();
    }
  }
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  QWidget::resizeEvent(event);
}

void DecimalBox::wheelEvent(QWheelEvent* event) {
  if(hasFocus() && !is_read_only()) {
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
  step_by(-get_increment());
}

void DecimalBox::increment() {
  step_by(get_increment());
}

DecimalBox::Decimal DecimalBox::get_increment() const {
  auto modifier_flags = static_cast<int>(qApp->keyboardModifiers());
  auto modifiers =
    std::bitset<std::numeric_limits<int>::digits>(modifier_flags);
  if(modifiers.count() != 1) {
    auto increment = m_modifiers.find(
      static_cast<Qt::KeyboardModifier>(modifiers.to_ulong()));
    if(increment == m_modifiers.end()) {
      return m_modifiers.find(Qt::NoModifier).value();
    }
  }
  return m_modifiers.find(
    static_cast<Qt::KeyboardModifier>(modifiers.to_ulong())).value();
}

void DecimalBox::step_by(const Decimal& value) {
  setFocus();
  auto next = Decimal(m_model->get_current() + value);
  if(m_model->get_minimum() && next < m_model->get_minimum()) {
    next = *m_model->get_minimum();
  } else if(m_model->get_maximum() && next > m_model->get_maximum()) {
    next = *m_model->get_maximum();
  }
  if(next != m_model->get_current()) {
    m_model->set_current(next);
  }
}

void DecimalBox::update_button_positions() {
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_up_button->move(button_pos.x(), button_pos.y() - m_up_button->height());
  m_down_button->move(button_pos);
}

void DecimalBox::on_current(const Decimal& current) {
  m_up_button->setEnabled(!is_read_only() && (!m_model->get_maximum() ||
    m_model->get_current() < m_model->get_maximum()));
  m_down_button->setEnabled(!is_read_only() && (!m_model->get_minimum() ||
    m_model->get_current() > m_model->get_minimum()));
}

void DecimalBox::on_submit(const QString& submission) {
  static auto VALIDATOR = QRegExp("^([-|\\+]?[0-9]+(\\.[0-9]*)?)|(-\\.)");
  if(VALIDATOR.exactMatch(submission)) {
    m_submission = m_adaptor_model->submit();
    m_submit_signal(m_submission);
  }
}

void DecimalBox::on_reject(const QString& value) {
  m_reject_signal(to_decimal(value).value_or(Decimal(0)));
  m_adaptor_model->reject();
}

void DecimalBox::on_style() {
  auto style = compute_style(*this);
  if(auto leading_zeros = Styles::find<LeadingZeros>(style)) {
    m_adaptor_model->set_leading_zeros(
      leading_zeros->get_expression().as<int>());
  }
  if(auto trailing_zeros = Styles::find<TrailingZeros>(style)) {
    m_adaptor_model->set_trailing_zeros(
      trailing_zeros->get_expression().as<int>());
  }
}
