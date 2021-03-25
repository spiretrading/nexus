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

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DEFAULT_DECIMAL_PLACES = 6;

  DecimalBox::Decimal to_decimal(const QString& text) {
    try {
      return DecimalBox::Decimal(text.toStdString().c_str());
    } catch (const std::runtime_error&) {
      return DecimalBox::Decimal::backend_type::nan();
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
    static const inline QRegExp m_validator =
      QRegExp(QRegExp("^[-]?[0-9]*(\\.[0-9]*)?"));
    std::shared_ptr<DecimalBox::DecimalModel> m_model;
    QString m_current;
    scoped_connection m_current_connection;
    mutable CurrentSignal m_current_signal;

    DecimalToTextModel(std::shared_ptr<DecimalBox::DecimalModel> model)
      : m_model(std::move(model)),
        m_current(to_string(m_model->get_current())),
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
      if(!m_validator.exactMatch(value)) {
        return QValidator::State::Invalid;
      }
      auto blocker = shared_connection_block(m_current_connection);
      auto state = m_model->set_current(to_decimal(value));
      if(state != QValidator::State::Invalid) {
        m_current = value;
        m_current_signal(m_current);
      }
      return state;
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
      m_modifiers(std::move(modifiers)),
      m_has_trailing_zeros(false),
      m_trailing_zero_regex(QString("[%1]?[0]*$").arg(
        QLocale().decimalPoint())) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(std::make_shared<DecimalToTextModel>(m_model), this);
//  set_decimal_places(DEFAULT_DECIMAL_PLACES);
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

bool DecimalBox::has_trailing_zeros() const {
  return m_has_trailing_zeros;
}

void DecimalBox::set_trailing_zeros(bool has_trailing_zeros) {
  m_has_trailing_zeros = has_trailing_zeros;
  update_trailing_zeros();
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
    auto keyEvent = static_cast<QKeyEvent*>(event);
    if(keyEvent->key() == Qt::Key_Up) {
      increment();
    } else if(keyEvent->key() == Qt::Key_Down) {
      decrement();
    }
  }
  return StyledWidget::eventFilter(watched, event);
}

void DecimalBox::resizeEvent(QResizeEvent* event) {
  update_button_positions();
  StyledWidget::resizeEvent(event);
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

void DecimalBox::step_by(Decimal value) {
  setFocus();
  value += m_model->get_current();
  m_model->set_current(value);
}

void DecimalBox::update_button_positions() {
  auto button_pos = QPoint(width() - BUTTON_RIGHT_PADDING() -
    BUTTON_SIZE().width(), height() / 2);
  m_up_button->move(button_pos.x(), button_pos.y() - m_up_button->height());
  m_down_button->move(button_pos);
}

void DecimalBox::update_trailing_zeros() {
#if 0 // TODO
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
// TODO    m_text_box->set_current(current_text.leftJustified(
//      current_text.length() + zero_count, '0'));
  } else {
// TODO    m_text_box->set_current(current_text.left(
//      current_text.length() + zero_count));
  }
#endif
}

void DecimalBox::on_current(const Decimal& current) {
  m_up_button->setEnabled(!m_model->get_maximum() ||
    m_model->get_current() < m_model->get_maximum());
  m_down_button->setEnabled(!m_model->get_minimum() ||
    m_model->get_current() > m_model->get_minimum());
}

void DecimalBox::on_submit(const QString& submission) {
  update_trailing_zeros();
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}
