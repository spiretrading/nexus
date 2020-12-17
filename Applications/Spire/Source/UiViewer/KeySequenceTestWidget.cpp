#include "Spire/UiViewer/KeySequenceTestWidget.hpp"
#include "Spire/KeyBindings/TestKeySequenceValidationModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Spire;

namespace {
  auto INPUT_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto get_key(const QString& text) {
    auto key_text = text.toLower();
    if(key_text == "ctrl" || key_text == "control") {
      return Qt::Key_Control;
    } else if(key_text == "alt") {
      return Qt::Key_Alt;
    } else if(key_text == "shift") {
      return Qt::Key_Shift;
    }
    auto sequence = QKeySequence(key_text);
    if(sequence.count() != 0) {
      return Qt::Key(sequence[0]);
    }
    return Qt::Key_unknown;
  }

  auto get_key_sequence_text(const QKeySequence sequence) {
    auto text = QString();
    for(auto i = 0; i < sequence.count(); ++i) {
      switch(Qt::Key(sequence[i])) {
        case Qt::Key_Shift:
          text.append(QObject::tr("Shift + "));
          break;
        case Qt::Key_Alt:
          text.append(QObject::tr("Alt + "));
          break;
        case Qt::Key_Control:
          text.append(QObject::tr("Ctrl + "));
          break;
        case Qt::Key_unknown:
          text.append(QObject::tr("Na + "));
          break;
        default:
          text.append(QKeySequence(sequence[i]).toString() +
            QObject::tr(" + "));
          break;
      }
    }
    text.remove(text.length() - 3, 3);
    return text;
  }
}

KeySequenceTestWidget::KeySequenceTestWidget(QWidget* parent)
    : QWidget(parent),
      m_input(nullptr) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 1);
  m_layout->addWidget(create_parameters_label(this), 1, 0, 1, 2);
  m_disable_check_box = make_check_box(tr("Disable"), this);
  connect(m_disable_check_box, &CheckBox::stateChanged, [=] (auto state) {
    m_input->setDisabled(m_disable_check_box->isChecked());
  });
  m_layout->addWidget(m_disable_check_box, 2, 1);
  m_set_input = new TextInputWidget(this);
  m_set_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_set_input, 3, 0);
  m_set_button = make_flat_button(tr("Set Sequence"), this);
  m_set_button->setFixedSize(INPUT_SIZE());
  m_set_button->connect_clicked_signal([=] { on_set_button(); });
  m_layout->addWidget(m_set_button, 3, 1);
  m_reset_input = new TextInputWidget("ctrl,f1", this);
  m_reset_input->setFixedSize(INPUT_SIZE());
  m_layout->addWidget(m_reset_input, 4, 0);
  m_reset_button = make_flat_button(tr("Reset Sequence"), this);
  m_reset_button->setFixedSize(INPUT_SIZE());
  m_reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(m_reset_button, 4, 1);
  on_reset_button();
}

QKeySequence KeySequenceTestWidget::parse_key_sequence(const QString& text) {
  auto keys = text.split(",", Qt::SkipEmptyParts);
  switch(keys.size()) {
    case 1:
      return QKeySequence(get_key(keys[0]));
    case 2:
      return QKeySequence(get_key(keys[0]), get_key(keys[1]));
    case 3:
      return QKeySequence(get_key(keys[0]), get_key(keys[1]),
        get_key(keys[2]));
    case 4:
      return QKeySequence(get_key(keys[0]), get_key(keys[1]), get_key(keys[2]),
        get_key(keys[3]));
    default:
      return QKeySequence();
  }
}

void KeySequenceTestWidget::on_reset_button() {
  if(auto sequence = parse_key_sequence(m_reset_input->text());
      !sequence.isEmpty()) {
    delete_later(m_input);
    m_input = new KeySequenceInputField(
      std::make_shared<TestKeySequenceValidationModel>(), this);
    m_input->setFixedSize(scale(130, 26));
    m_layout->addWidget(m_input, 0, 0);
    connect(m_input, &KeySequenceInputField::editingFinished, [=] {
      m_status_label->setText(get_key_sequence_text(
        m_input->get_key_sequence()));
    });
    m_input->set_key_sequence(sequence);
    setTabOrder(m_input, m_disable_check_box);
    setTabOrder(m_disable_check_box, m_set_input);
    setTabOrder(m_set_input, m_set_button);
    setTabOrder(m_set_button, m_reset_input);
    setTabOrder(m_reset_input, m_reset_button);
  }
}

void KeySequenceTestWidget::on_set_button() {
  if(auto sequence = parse_key_sequence(m_set_input->text());
      !sequence.isEmpty()) {
    m_input->set_key_sequence(sequence);
  }
}
