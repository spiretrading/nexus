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

  struct DecimalToTextModel : TextModel {
    std::shared_ptr<DecimalBox::DecimalModel> m_model;
    QString m_current;
    int m_decimal_places;
    QRegExp m_validator;
    scoped_connection m_current_connection;
    mutable CurrentSignal m_current_signal;

    DecimalToTextModel(std::shared_ptr<DecimalBox::DecimalModel> model)
      : m_model(std::move(model)),
        m_current(to_string(m_model->get_current())),
        m_decimal_places(-log10(m_model->get_increment()).convert_to<int>()),
        m_validator(
          QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1})?").arg(m_decimal_places)),
        m_current_connection(m_model->connect_current_signal(
          [=] (const auto& current) {
            on_current(current);
          })) {}

    QValidator::State get_state() const override {
      return m_model->get_state();
    }

    const QString& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const QString& value) override {
      auto decimal_places = -log10(m_model->get_increment()).convert_to<int>();
      if(decimal_places != m_decimal_places) {
        m_validator = QRegExp(
          QString("^[-|\\+]?[0-9]*(\\.[0-9]{0,%1})?").arg(m_decimal_places));
        m_decimal_places = decimal_places;
      }
      if(!m_validator.exactMatch(value)) {
        return QValidator::State::Invalid;
      } else if(value.isEmpty() || value == "-" || value == "+") {
        m_current = value;
        m_current_signal(m_current);
        return QValidator::State::Intermediate;
      } else if(auto decimal = to_decimal(value)) {
        auto blocker = shared_connection_block(m_current_connection);
        auto state = m_model->set_current(*decimal);
        if(state != QValidator::State::Invalid) {
          m_current = value;
          m_current_signal(m_current);
        }
        return state;
      }
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override {
      return m_current_signal.connect(slot);
    }

    void on_current(const DecimalBox::Decimal& current) {
      m_current = to_string(current);
      m_current_signal(m_current);
    }
  };
}

DecimalBox::DecimalBox(QHash<Qt::KeyboardModifier, Decimal> modifiers,
  QWidget* parent)
  : DecimalBox(std::make_shared<LocalScalarValueModel<Decimal>>(),
      std::move(modifiers), parent) {}

DecimalBox::DecimalBox(std::shared_ptr<DecimalModel> model,
    QHash<Qt::KeyboardModifier, Decimal> modifiers, QWidget* parent)
    : StyledWidget(parent),
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_modifiers(std::move(modifiers)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(std::make_shared<DecimalToTextModel>(m_model), this);
  auto style = m_text_box->get_style();
  style.get((is_a<Button>() && !matches(Visibility(VisibilityOption::NONE))) %
    is_a<TextBox>() > is_a<Box>()).set(PaddingRight(scale_width(26)));
  m_text_box->set_style(std::move(style));
  setFocusProxy(m_text_box);
  layout->addWidget(m_text_box);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
  m_submit_connection = m_text_box->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_text_box->installEventFilter(this);
  m_up_button = create_button(":/Icons/arrow-up.svg", this);
  m_up_button->connect_clicked_signal([=] { increment(); });
  m_down_button = create_button(":/Icons/arrow-down.svg", this);
  m_down_button->connect_clicked_signal([=] { decrement(); });
  update_button_positions();
}

const std::shared_ptr<DecimalBox::DecimalModel>& DecimalBox::get_model() const {
  return m_model;
}

bool DecimalBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
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
    if(!is_read_only()) {
      auto keyEvent = static_cast<QKeyEvent*>(event);
      if(keyEvent->key() == Qt::Key_Up) {
        increment();
      } else if(keyEvent->key() == Qt::Key_Down) {
        decrement();
      }
    }
  }
  return StyledWidget::eventFilter(watched, event);
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  StyledWidget::resizeEvent(event);
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
  StyledWidget::wheelEvent(event);
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
  auto next = std::clamp(Decimal(m_model->get_current() + value),
    m_model->get_minimum().value_or(value),
    m_model->get_maximum().value_or(value));
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
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}
