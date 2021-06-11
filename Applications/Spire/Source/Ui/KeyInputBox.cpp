#include "Spire/Ui/KeyInputBox.hpp"
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

  auto KEY_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }

  auto KEY_SPACING() {
    static auto spacing = scale_width(4);
    return spacing;
  }

  auto make_key_sequence(const std::vector<Qt::Key>& keys) {
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
      m_model(std::move(model)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_layers = new LayeredWidget(this);
  layout->addWidget(m_layers);
  m_text_box = new TextBox(this);
  m_layers->add(m_text_box);
  setFocusProxy(m_text_box);
  m_text_box->installEventFilter(this);
  m_text_box->findChild<QLineEdit*>()->installEventFilter(this);
  m_key_spacer = new QWidget(this);
  m_key_spacer->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_key_layout = new QHBoxLayout(m_key_spacer);
  m_key_layout->setContentsMargins(scale_width(8), 0, 0, 0);
  m_key_layout->setSpacing(scale_width(4));
  auto key_box = new Box(m_key_spacer, this);
  set_style(*key_box, KEY_BOX_STYLE());
  key_box->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_layers->add(key_box);
  m_model->connect_current_signal([=] (const auto& sequence) {
    on_current_sequence(sequence);
  });
  on_current_sequence(m_model->get_current());
}

bool KeyInputBox::eventFilter(QObject* watched, QEvent* event) {
  switch(event->type()) {
    case QEvent::MouseButtonPress:
      if(isEnabled()) {
        auto e = static_cast<QMouseEvent*>(event);
        if(e->button() == Qt::LeftButton) {
          set_status(Status::PROMPT);
        }
      }
      break;
    case QEvent::FocusIn:
      if(m_model->get_current().isEmpty()) {
        set_status(Status::PROMPT);
      }
      break;
    case QEvent::FocusOut:
      set_status(Status::NONE);
      break;
    case QEvent::KeyPress:
    {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->isAutoRepeat()) {
        return true;
      }
      switch(e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
          set_status(Status::PROMPT);
          break;
        case Qt::Key_Delete:
          m_model->set_current(QKeySequence());
          m_submission = {};
          m_submit_signal(m_submission);
          break;
        case Qt::Key_Alt:
        case Qt::Key_Control:
        case Qt::Key_Shift:
          break;
        default:
          auto sequence = [&] {
            auto keys = std::vector<Qt::Key>();
            if(e->modifiers().testFlag(Qt::AltModifier)) {
              keys.push_back(Qt::Key_Alt);
            }
            if(e->modifiers().testFlag(Qt::ControlModifier)) {
              keys.push_back(Qt::Key_Control);
            }
            if(e->modifiers().testFlag(Qt::ShiftModifier)) {
              keys.push_back(Qt::Key_Shift);
            }
            keys.push_back(Qt::Key(e->key()));
            return make_key_sequence(keys);
          }();
          if(m_model->set_current(sequence) == QValidator::Acceptable) {
            m_submission = sequence;
            m_submit_signal(m_submission);
          }
      }
    }
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

void KeyInputBox::resizeEvent(QResizeEvent* event) {
  if(auto border_size =
      Styles::find<BorderLeftSize>(get_computed_block(*m_text_box))) {
    m_key_spacer->setMask({0, 0, width() - border_size->get_expression().
      as<ConstantExpression<int>>().get_constant(), height()});
  } else {
    m_key_spacer->clearMask();
  }
  QWidget::resizeEvent(event);
}

const QKeySequence& KeyInputBox::get_submission() const {
  return m_submission;
}

const std::shared_ptr<KeySequenceModel>& KeyInputBox::get_model() const {
  return m_model;
}

QSize KeyInputBox::sizeHint() const {
  return m_layers->sizeHint();
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::set_status(Status status) {
  clear_layout(m_key_layout);
  if(status == Status::PROMPT) {
    m_text_box->set_placeholder(tr("Enter Keys"));
    auto style = get_style(*m_text_box);
    style.get(Any()).set(PaddingLeft(scale_width(8)));
    set_style(*m_text_box, style);
    return;
  }
  m_text_box->set_placeholder("");
  auto sequence = m_model->get_current();
  if(!sequence.isEmpty()) {
    auto sequence_size = QSize(KEY_PADDING(), 0);
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
    auto style = get_style(*m_text_box);
    style.get(Any()).set(PaddingLeft(sequence_size.width() + CARET_PADDING()));
    set_style(*m_text_box, style);
    m_key_spacer->setFixedSize(sequence_size);
  }
}

void KeyInputBox::on_current_sequence(const QKeySequence& sequence) {
  if(sequence.isEmpty() && hasFocus()) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}
