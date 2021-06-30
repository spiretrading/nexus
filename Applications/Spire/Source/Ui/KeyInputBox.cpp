#include "Spire/Ui/KeyInputBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto CARET_PADDING() {
    static auto padding = scale_width(2);
    return padding;
  }

  auto KEY_BOX_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0, 0, 0, 0))).
      set(BodyAlign(Qt::AlignLeft | Qt::AlignVCenter));
    return style;
  }

  auto KEY_SPACING() {
    static auto spacing = scale_width(4);
    return spacing;
  }

  auto make_key_sequence(QKeyEvent* event) {
    auto keys = std::vector<Qt::Key>();
    if(event->modifiers().testFlag(Qt::AltModifier)) {
      keys.push_back(Qt::Key_Alt);
    }
    if(event->modifiers().testFlag(Qt::ControlModifier)) {
      keys.push_back(Qt::Key_Control);
    }
    if(event->modifiers().testFlag(Qt::ShiftModifier)) {
      keys.push_back(Qt::Key_Shift);
    }
    keys.push_back(Qt::Key(event->key()));
    switch(keys.size()) {
      case 1:
        return QKeySequence(keys[0]);
      case 2:
        return QKeySequence(keys[0], keys[1]);
      case 3:
        return QKeySequence(keys[0], keys[1], keys[2]);
      case 4:
        return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
      default:
        return QKeySequence();
    }
  }
}

KeyInputBox::KeyInputBox(std::shared_ptr<KeySequenceModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_previous_current(m_model->get_current()),
      m_is_rejected(false),
      m_text_box_right_margin(0) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_layers = new LayeredWidget(this);
  layout->addWidget(m_layers);
  m_text_box = new TextBox(this);
  m_layers->add(m_text_box);
  setFocusProxy(m_text_box);
  connect_style_signal(*m_text_box, [=] { on_text_box_style(); });
  m_text_box->findChild<QLineEdit*>()->installEventFilter(this);
  m_key_sequence = new QWidget(this);
  m_key_sequence->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_key_sequence->setFixedSize(0, 0);
  m_key_layout = new QHBoxLayout(m_key_sequence);
  m_key_layout->setSpacing(scale_width(4));
  auto key_box = new Box(m_key_sequence, this);
  set_style(*key_box, KEY_BOX_STYLE());
  key_box->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_layers->add(key_box);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& sequence) { on_current_sequence(sequence); });
  set_status(Status::NONE);
}

bool KeyInputBox::eventFilter(QObject* watched, QEvent* event) {
  switch(event->type()) {
    case QEvent::FocusIn:
      if(m_model->get_current().isEmpty()) {
        set_status(Status::PROMPT);
      }
      break;
    case QEvent::FocusOut:
      submit_current();
      set_status(Status::NONE);
      break;
    case QEvent::KeyPress:
    {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->isAutoRepeat()) {
        return true;
      }
      switch(e->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
          m_submission = {};
          m_model->set_current(QKeySequence());
          break;
        case Qt::Key_Escape:
          if(m_model->set_current(make_key_sequence(e)) ==
              QValidator::Invalid) {
            m_model->set_current(m_submission);
          }
          break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
          submit_current();
          break;
        case Qt::Key_Alt:
        case Qt::Key_Control:
        case Qt::Key_Shift:
          break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
          return false;
        default:
          m_model->set_current(make_key_sequence(e));
      }
    }
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

void KeyInputBox::resizeEvent(QResizeEvent* event) {
  m_key_sequence->setMask(QRegion(0, 0, width() - m_text_box_right_margin,
    height()));
  QWidget::resizeEvent(event);
}

const QKeySequence& KeyInputBox::get_submission() const {
  return m_submission;
}

const std::shared_ptr<KeySequenceModel>& KeyInputBox::get_model() const {
  return m_model;
}

QSize KeyInputBox::sizeHint() const {
  return {m_layers->sizeHint().width() + m_key_sequence->width() -
    m_key_layout->contentsMargins().left(), m_layers->sizeHint().height()};
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::set_status(Status status) {
  clear_layout(m_key_layout);
  if(status == Status::PROMPT) {
    m_text_box->set_placeholder(tr("Enter Keys"));
    m_key_sequence->setFixedSize(0, 0);
    return;
  }
  m_text_box->set_placeholder("");
  auto sequence = m_model->get_current();
  if(!sequence.isEmpty()) {
    auto sequence_size = QSize(m_key_layout->contentsMargins().left(), 0);
    for(auto i = 0; i < sequence.count(); ++i) {
      auto tag = new KeyTag(
        std::make_shared<LocalKeyModel>(Qt::Key(sequence[i])), this);
      tag->setAttribute(Qt::WA_TransparentForMouseEvents);
      m_key_layout->addWidget(tag);
      sequence_size.rwidth() += tag->sizeHint().width();
      if(i < sequence.count() - 1) {
        sequence_size.rwidth() += KEY_SPACING();
      }
      sequence_size.rheight() = std::max(tag->sizeHint().height(),
        sequence_size.height());
    }
    m_key_sequence->setFixedSize(sequence_size);
  }
}

void KeyInputBox::submit_current() {
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}

void KeyInputBox::on_current_sequence(const QKeySequence& sequence) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_text_box, Rejected());
  }
  auto blocker = shared_connection_block(m_current_connection);
  if(sequence.isEmpty()) {
    m_previous_current = m_submission;
    m_model->set_current(m_submission);
    m_submit_signal(m_submission);
    set_status(Status::PROMPT);
  } else if(m_model->get_state() == QValidator::Invalid) {
    m_model->set_current(m_previous_current);
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*m_text_box, Rejected());
    }
  } else {
    m_previous_current = sequence;
    set_status(Status::NONE);
  }
}

void KeyInputBox::on_text_box_style() {
  m_text_box_right_margin = 0;
  auto text_box_left_margin = CARET_PADDING();
  auto& stylist = find_stylist(*m_text_box);
  auto block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [&] (auto size) {
          text_box_left_margin += size;
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_text_box_right_margin += size;
        });
      },
      [&] (const PaddingLeft& padding) {
        stylist.evaluate(padding, [&] (auto padding) {
          text_box_left_margin += padding;
        });
      },
      [&] (const PaddingRight& padding) {
        stylist.evaluate(padding, [=] (auto padding) {
          m_text_box_right_margin += padding;
        });
      });
  }
  m_key_layout->setContentsMargins(text_box_left_margin, 0, 0, 0);
}
