#include "Spire/Ui/KeyInputBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::vector<Qt::Key> split(const QKeySequence& sequence) {
    auto keys = std::vector<Qt::Key>();
    for(auto& key: sequence.toString().split('+')) {
      if(key == "Shift") {
        keys.push_back(Qt::Key_Shift);
      } else if(key == "Meta") {
        keys.push_back(Qt::Key_Meta);
      } else if(key == "Ctrl") {
        keys.push_back(Qt::Key_Control);
      } else if(key == "Alt") {
        keys.push_back(Qt::Key_Alt);
      } else {
        auto name = QKeySequence(key);
        if(name.count() != 0) {
          keys.push_back(static_cast<Qt::Key>(name[0]));
        }
      }
    }
    return keys;
  }

  void clear(QLayout& layout) {
    while(auto item = layout.takeAt(0)) {
      item->widget()->deleteLater();
      delete item;
    }
  }

  class Caret : public QWidget {
    public:
      Caret(QWidget* focus_widget)
          : m_focus_widget(focus_widget),
            m_is_visible(false) {
        auto blink_timer = new QTimer(this);
        connect(blink_timer, &QTimer::timeout, this, [=] { on_blink(); });
        const auto BLINK_RATE = 500;
        blink_timer->setInterval(BLINK_RATE);
        blink_timer->start();
        setFixedWidth(scale_width(1));
      }

    protected:
      void paintEvent(QPaintEvent* event) override {
        if(m_is_visible && m_focus_widget->hasFocus()) {
          auto painter = QPainter(this);
          painter.fillRect(QRect(0, 0, width(), height()), QColor(0x0u));
        }
      }

    private:
      QWidget* m_focus_widget;
      bool m_is_visible;

      void on_blink() {
        m_is_visible = !m_is_visible;
        update();
      }
  };
}

KeyInputBox::KeyInputBox(
    std::shared_ptr<KeySequenceValueModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_status(Status::UNINITIALIZED) {
  setFocusPolicy(Qt::StrongFocus);
  m_layers = new LayeredWidget();
  m_layers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  m_body = new QWidget();
  m_layers->add(m_body);
  m_layers->add(new Caret(this));
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  layout->addWidget(make_input_box(m_layers, this));
  setLayout(layout);
  auto body_layout = new QHBoxLayout();
  body_layout->setContentsMargins({});
  m_body->setLayout(body_layout);
  set_status(Status::NONE);
  m_current_connection = m_current->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
}

KeyInputBox::KeyInputBox(QWidget* parent)
  : KeyInputBox(std::make_shared<LocalKeySequenceValueModel>(), parent) {}

const std::shared_ptr<KeySequenceValueModel>& KeyInputBox::get_current() const {
  return m_current;
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::focusInEvent(QFocusEvent* event) {
  transition_status();
}

void KeyInputBox::focusOutEvent(QFocusEvent* event) {
  set_status(Status::NONE);
  transition_submission();
}

void KeyInputBox::keyPressEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_Shift || key == Qt::Key_Meta || key == Qt::Key_Control ||
      key == Qt::Key_Alt) {
    return;
  } else if(event->modifiers() == 0) {
    if(key == Qt::Key_Delete || key == Qt::Key_Backspace) {
      m_submission = m_current->get_current();
      m_current->set_current(QKeySequence());
    } else if(key == Qt::Key_Escape &&
        m_current->set_current(key) == QValidator::Invalid) {
      m_current->set_current(m_submission);
    } else if(key == Qt::Key_Enter || key == Qt::Key_Tab) {
      transition_submission();
    } else {
      m_current->set_current(key);
    }
  } else {
    m_current->set_current(event->modifiers() + key);
  }
}

void KeyInputBox::layout_key_sequence() {
  if(m_status == Status::NONE) {
    auto& layout = *m_body->layout();
    clear(layout);
    layout.setSpacing(scale_width(4));
    for(auto key : split(m_current->get_current())) {
      layout.addWidget(new KeyTag(make_constant_value_model(key)));
    }
    m_layers->updateGeometry();
  }
}

void KeyInputBox::transition_status() {
  if(m_current->get_current().count() == 0) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}

void KeyInputBox::transition_submission() {
  m_submission = m_current->get_current();
  m_submit_signal(m_submission);
}

void KeyInputBox::set_status(Status status) {
  if(m_status == status) {
    return;
  }
  m_status = status;
  if(m_status == Status::NONE) {
    layout_key_sequence();
  } else if(m_status == Status::PROMPT) {
    auto& layout = *m_body->layout();
    clear(layout);
    layout.setSpacing(0);
    auto prompt = new TextBox(tr("Enter Keys"));
    prompt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    prompt->set_read_only(true);
    prompt->setDisabled(true);
    auto style = get_style(*prompt);
    style.get(Any()).set(vertical_padding(0));
    set_style(*prompt, std::move(style));
    layout.addWidget(prompt);
    m_layers->updateGeometry();
  }
}

void KeyInputBox::on_current(const QKeySequence& current) {
  transition_status();
  layout_key_sequence();
  if(current.count() == 0) {
    transition_submission();
  }
}
