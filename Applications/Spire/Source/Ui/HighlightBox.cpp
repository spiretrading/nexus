#include "Spire/Ui/HighlightBox.hpp"
#include <QChildEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStackedWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ColorPicker.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
}

HighlightBox::HighlightBox(QWidget* parent)
  : HighlightBox(std::make_shared<LocalHighlightColorModel>(), parent) {}

HighlightBox::HighlightBox(std::shared_ptr<HighlightColorModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_read_only(false),
      m_is_modified(false),
      m_focus_observer(*this),
      m_press_observer(*this) {
  setFocusPolicy(Qt::StrongFocus);
}

const std::shared_ptr<HighlightColorModel>& HighlightBox::get_current() const {
  return m_current;
}

bool HighlightBox::is_read_only() const {
  return m_is_read_only;
}

void HighlightBox::set_read_only(bool is_read_only) {
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    //match(*m_input_box, ReadOnly());
    //m_color_picker->hide();
  } else {
    //unmatch(*m_input_box, ReadOnly());
  }
}

connection HighlightBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

//bool HighlightBox::eventFilter(QObject* watched, QEvent* event) {
//  if(m_color_picker_panel == watched) {
//    if(event->type() == QEvent::MouseButtonPress ||
//        event->type() == QEvent::MouseButtonRelease ||
//        event->type() == QEvent::MouseButtonDblClick) {
//      auto& mouse_event = *static_cast<QMouseEvent*>(event);
//      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
//        return true;
//      }
//    }
//  } else if(event->type() == QEvent::KeyPress) {
//    auto& key_event = *static_cast<QKeyEvent*>(event);
//    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
//      submit();
//      m_color_picker->hide();
//    } else if(key_event.key() == Qt::Key_Escape) {
//      if(m_submission != m_current->get()) {
//        m_current->set(m_submission);
//      }
//      m_is_modified = false;
//      m_color_picker->hide();
//      return true;
//    }
//  } else if(event->type() == QEvent::ChildAdded) {
//    auto& child_event = *static_cast<QChildEvent*>(event);
//    if(child_event.child()->isWidgetType()) {
//      child_event.child()->installEventFilter(this);
//      watched->removeEventFilter(this);
//    }
//  }
//  return QWidget::eventFilter(watched, event);
//}
//
//void HighlightBox::keyPressEvent(QKeyEvent* event) {
//  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
//      event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
//    if(!is_read_only()) {
//      show_color_picker();
//    }
//  }
//  QWidget::keyPressEvent(event);
//}
//
//void HighlightBox::submit() {
//  m_is_modified = false;
//  m_submission = m_current->get();
//  m_submit_signal(m_submission);
//}
//
//void HighlightBox::on_current(const QColor& current) {
//  m_is_modified = true;
//  update_style(*m_color_display, [&] (auto& style) {
//    style.get(Any()).set(BackgroundColor(current));
//  });
//}
//
//void HighlightBox::show_color_picker() {
//  if(!m_color_picker->isVisible()) {
//    m_color_picker->show();
//    get_color_code_panel(*m_color_picker)->nextInFocusChain()->setFocus();
//  }
//}
//
//void HighlightBox::on_focus(FocusObserver::State state) {
//  if(is_read_only()) {
//    return;
//  }
//  if(state == FocusObserver::State::NONE) {
//    unmatch(*m_input_box, FocusIn());
//    if(m_is_modified) {
//      submit();
//    }
//  } else {
//    match(*m_input_box, FocusIn());
//  }
//}
//
//void HighlightBox::on_press_end(PressObserver::Reason reason) {
//  if(!is_read_only()) {
//    show_color_picker();
//  }
//}
