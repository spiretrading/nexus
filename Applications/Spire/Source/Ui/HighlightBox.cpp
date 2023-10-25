#include "Spire/Ui/HighlightBox.hpp"
#include <QChildEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStackedWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto get_highlight_palette(HighlightPicker& picker) {
    return static_cast<ListView*>(
      picker.layout()->itemAt(0)->layout()->itemAt(0)->widget());
  }
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
  m_label = make_label("123.45");
  m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_label, [&] (auto& style) {
    style.get(Any()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(m_current->get().m_background_color)).
      set(TextColor(m_current->get().m_text_color));
  });
  m_input_box = make_input_box(m_label);
  update_style(*m_input_box, [] (auto& style) {
    style.get(Disabled() > is_a<TextBox>()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(TextColor(QColor(0xC8C8C8)));
  });
  enclose(*this, *m_input_box);
  proxy_style(*this, *m_input_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(padding(scale_width(1)));
  });
  m_highlight_picker = new HighlightPicker(m_current, *this);
  m_highlight_picker_panel = m_highlight_picker->window();
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&HighlightBox::on_current, this));
  m_focus_observer.connect_state_signal(
    std::bind_front(&HighlightBox::on_focus, this));
  m_press_observer.connect_press_end_signal(
    std::bind_front(&HighlightBox::on_press_end, this));
  m_highlight_picker_panel->installEventFilter(this);
  auto palette = get_highlight_palette(*m_highlight_picker);
  for(auto i = 0; i < palette->get_list()->get_size(); ++i) {
    palette->get_list_item(i)->layout()->itemAt(0)->widget()->installEventFilter(this);
  }
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
    match(*m_input_box, ReadOnly());
    m_highlight_picker->hide();
  } else {
    unmatch(*m_input_box, ReadOnly());
  }
}

connection HighlightBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool HighlightBox::eventFilter(QObject* watched, QEvent* event) {
  if(m_highlight_picker_panel == watched) {
    if(event->type() == QEvent::MouseButtonPress ||
      event->type() == QEvent::MouseButtonRelease ||
      event->type() == QEvent::MouseButtonDblClick) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        return true;
      }
    } else if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Escape) {
        if(m_submission != m_current->get()) {
          m_current->set(m_submission);
        }
        m_is_modified = false;
        m_highlight_picker->hide();
        return true;
      }
    }
  } else {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
        submit();
        m_highlight_picker->hide();
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void HighlightBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(!is_read_only()) {
      show_highlight_picker();
    }
  } else if(event->key() == Qt::Key_Escape) {
    if(m_submission != m_current->get()) {
      m_current->set(m_submission);
    }
    m_is_modified = false;
  }
  QWidget::keyPressEvent(event);
}

void HighlightBox::submit() {
  m_is_modified = false;
  m_submission = m_current->get();
  m_submit_signal(m_submission);
}

void HighlightBox::on_current(const HighlightColor& current) {
  m_is_modified = true;
  update_style(*m_label, [&] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(current.m_background_color)).
      set(TextColor(current.m_text_color));
  });
}

void HighlightBox::show_highlight_picker() {
  if(!m_highlight_picker->isVisible()) {
    m_highlight_picker->show();
    get_highlight_palette(*m_highlight_picker)->setFocus();
  }
}

void HighlightBox::on_focus(FocusObserver::State state) {
  if(is_read_only()) {
    return;
  }
  if(state == FocusObserver::State::NONE) {
    unmatch(*m_input_box, FocusIn());
    if(m_is_modified) {
      submit();
    }
  } else {
    match(*m_input_box, FocusIn());
  }
}

void HighlightBox::on_press_end(PressObserver::Reason reason) {
  if(!is_read_only()) {
    show_highlight_picker();
  }
}
