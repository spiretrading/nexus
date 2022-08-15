#include "Spire/Ui/KeyInputBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/Layouts.hpp"
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
      if(item->widget()) {
        item->widget()->deleteLater();
      }
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
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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
      m_status(Status::UNINITIALIZED),
      m_is_read_only(false),
      m_is_modified(false) {
  setFocusPolicy(Qt::StrongFocus);
  m_body = new QWidget();
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_body->setLayout(make_hbox_layout());
  auto layers = new LayeredWidget();
  layers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  layers->add(m_body);
  layers->add(new Caret(this));
  m_input_box = make_input_box(layers);
  proxy_style(*this, *m_input_box);
  enclose(*this, *m_input_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(vertical_padding(scale_height(3)));
  });
  set_status(Status::NONE);
  m_current_connection = m_current->connect_update_signal(
    [=] (const auto& current) { on_current(current); });
}

KeyInputBox::KeyInputBox(QWidget* parent)
  : KeyInputBox(std::make_shared<LocalKeySequenceValueModel>(), parent) {}

const std::shared_ptr<KeySequenceValueModel>& KeyInputBox::get_current() const {
  return m_current;
}

bool KeyInputBox::is_read_only() const {
  return m_is_read_only;
}

void KeyInputBox::set_read_only(bool is_read_only) {
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    setFocusPolicy(Qt::NoFocus);
    match(*m_input_box, ReadOnly());
  } else {
    setFocusPolicy(Qt::StrongFocus);
    unmatch(*m_input_box, ReadOnly());
  }
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize KeyInputBox::sizeHint() const {
  static auto size = QSize(0, scale_height(26));
  auto base_size_hint = QWidget::sizeHint();
  if(base_size_hint.isValid()) {
    base_size_hint.setHeight(std::max(base_size_hint.height(), size.height()));
    return base_size_hint;
  }
  return size;
}

void KeyInputBox::focusInEvent(QFocusEvent* event) {
  match(*m_input_box, FocusIn());
  transition_status();
}

void KeyInputBox::focusOutEvent(QFocusEvent* event) {
  unmatch(*m_input_box, FocusIn());
  set_status(Status::NONE);
  transition_submission();
}

void KeyInputBox::keyPressEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_Shift ||
      key == Qt::Key_Meta || key == Qt::Key_Control || key == Qt::Key_Alt) {
    return;
  } else if(event->modifiers() == 0) {
    if(key == Qt::Key_Delete || key == Qt::Key_Backspace) {
      m_submission = m_current->get();
      m_current->set(QKeySequence());
    } else if(key == Qt::Key_Escape &&
        m_current->set(key) == QValidator::Invalid && m_is_modified) {
      {
        auto blocker = shared_connection_block(m_current_connection);
        m_current->set(m_submission);
      }
      m_is_modified = false;
      transition_status();
      layout_key_sequence();
    } else if(key == Qt::Key_Enter || key == Qt::Key_Return) {
      m_is_modified = true;
      transition_submission();
    } else {
      m_current->set(key);
    }
  } else {
    m_current->set(event->modifiers() + key);
  }
}

void KeyInputBox::layout_key_sequence() {
  if(m_status == Status::NONE) {
    auto& layout = *m_body->layout();
    clear(layout);
    layout.setSpacing(scale_width(4));
    for(auto key : split(m_current->get())) {
      auto tag = new KeyTag(make_constant_value_model(key));
      tag->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      layout.addWidget(tag);
    }
  }
}

void KeyInputBox::transition_status() {
  if(m_current->get().count() == 0) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}

void KeyInputBox::transition_submission() {
  if(m_status == Status::PROMPT || !m_is_modified) {
    return;
  }
  if(m_current->get_state() == QValidator::State::Intermediate) {
    return;
  }
  m_is_modified = false;
  m_submission = m_current->get();
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
    auto prompt = make_label(tr("Enter Keys"));
    prompt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    prompt->set_read_only(true);
    prompt->setDisabled(true);
    update_style(*prompt, [&] (auto& style) {
      style.get(Disabled() && ReadOnly()).set(TextColor(QColor(0xA0A0A0)));
    });
    layout.addWidget(prompt);
  }
}

void KeyInputBox::on_current(const QKeySequence& current) {
  m_is_modified = true;
  transition_status();
  layout_key_sequence();
  if(current.count() == 0) {
    transition_submission();
  }
}
