#include "Spire/Ui/KeyInputBox.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

#include <QApplication>

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
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

KeyInputBox::KeyInputBox(QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<LocalKeySequenceModel>()) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(this);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_box->findChild<QLineEdit*>()->installEventFilter(this);
  layout->addWidget(m_text_box);
  m_key_container = new QWidget(this);
  m_key_container->setObjectName("key_container");
  m_key_container->setStyleSheet("#key_container { background-color: transparent; }");
  m_key_container->setAttribute(Qt::WA_TransparentForMouseEvents);
  //m_key_container->setSizePolicy(QSizePolicy::, QSizePolicy::Maximum);
  m_key_layout = new QHBoxLayout(m_key_container);
  m_key_layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(2),
    scale_height(6));
  m_key_layout->setSpacing(scale_width(4));
  m_key_layout->setAlignment(Qt::AlignLeft);
  m_key_layout->setSizeConstraint(QLayout::SetFixedSize);
  m_model->connect_current_signal([=] (const auto& sequence) {
    on_current_sequence(sequence);
  });
  on_current_sequence(m_model->get_current());
}

bool KeyInputBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    if(m_model->get_current().isEmpty()) {
      set_status(Status::PROMPT);
    }
  } else if(event->type() == QEvent::FocusOut) {
    set_status(Status::NONE);
  } else if(event->type() == QEvent::MouseButtonPress) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) {
      set_status(Status::PROMPT);
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    switch(e->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
        set_status(Status::PROMPT);
        break;
      case Qt::Key_Delete:
        m_model->set_current({});
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
        m_model->set_current(sequence);
        m_submission = sequence;
        m_submit_signal(m_submission);
    }
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

//void KeyInputBox::keyPressEvent(QKeyEvent* event) {
//
//  QWidget::keyPressEvent(event);
//}

//void KeyInputBox::mousePressEvent(QMouseEvent* event) {
//  if(event->button() == Qt::LeftButton) {
//    set_status(Status::PROMPT);
//  }
//  QWidget::mousePressEvent(event);
//}

const QKeySequence& KeyInputBox::get_submission() const {
  return m_submission;
}

const std::shared_ptr<KeySequenceModel>& KeyInputBox::get_model() const {
  return m_model;
}

QSize KeyInputBox::sizeHint() const {
  return scale(220, 30);
}

connection KeyInputBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyInputBox::set_status(Status status) {
  clear_layout(m_key_layout);
  if(status == Status::PROMPT) {
    //auto enter_keys_label = new TextBox(tr("Enter Keys"), this);
    //enter_keys_label->setSizePolicy(QSizePolicy::Expanding,
    //  QSizePolicy::Expanding);
    //enter_keys_label->setDisabled(true);
    //enter_keys_label->set_read_only(true);
    //m_box_layout->addWidget(enter_keys_label);
    m_text_box->set_placeholder(tr("Enter Keys"));
    auto style = get_style(*m_text_box);
    style.get(Any()).set(PaddingLeft(scale_width(8)));
    set_style(*m_text_box, style);
  } else {
    auto sequence = m_model->get_current();
    auto c_size = QSize();
    for(auto i = 0; i < sequence.count(); ++i) {
      auto tag = new KeyTag(Qt::Key(sequence[i]), this);
      tag->setAttribute(Qt::WA_TransparentForMouseEvents);
      tag->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      m_key_layout->addWidget(tag);
      tag->adjustSize();
      c_size.rwidth() += tag->width();
    }
    c_size.rwidth() += 2 * scale_width(8);
    m_text_box->set_placeholder("");
    //qDebug() << "****************************";
    //qDebug() << m_key_container->geometry();
    //m_key_container->updateGeometry();
    //m_key_container->adjustSize();
    //m_key_layout->invalidate();
    //m_key_layout->update();
    m_key_container->show();
    //qDebug() << m_key_container->geometry();
    if(!sequence.isEmpty()) {
      auto style = get_style(*m_text_box);
      auto left_padding = m_key_container->rect().right();
      //qDebug() << left_padding;
      style.get(Any()).set(PaddingLeft(c_size.width() - scale_width(2)));
      set_style(*m_text_box, style);
    }
  }
}

void KeyInputBox::on_current_sequence(const QKeySequence& sequence) {
  if(sequence.isEmpty() && hasFocus()) {
    set_status(Status::PROMPT);
  } else {
    set_status(Status::NONE);
  }
}
