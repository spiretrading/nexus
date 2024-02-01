#include "Spire/Ui/HighlightBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto get_highlight_palette(const HighlightPicker& picker) {
    return static_cast<ListView*>(
      picker.layout()->itemAt(0)->layout()->itemAt(0)->widget());
  }

  auto get_color_layout(const HighlightPicker& picker) {
    return picker.layout()->itemAt(0)->layout()->itemAt(2)->layout();
  }

  auto get_background_color_box(const HighlightPicker& picker) {
    return get_color_layout(picker)->itemAt(0)->widget();
  }

  auto get_text_color_box(const HighlightPicker& picker) {
    return get_color_layout(picker)->itemAt(2)->widget();
  }
}

HighlightBox::HighlightBox(QWidget* parent)
  : HighlightBox(std::make_shared<LocalHighlightColorModel>(), parent) {}

HighlightBox::HighlightBox(std::shared_ptr<HighlightColorModel> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_read_only(false),
      m_is_modified(false),
      m_focus_observer(*this),
      m_press_observer(*this) {
  setFocusPolicy(Qt::StrongFocus);
  auto label = make_label("123.45");
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  label->setFocusPolicy(Qt::NoFocus);
  update_style(*label, [] (auto& style) {
    style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  });
  m_input_box = make_input_box(label);
  enclose(*this, *m_input_box);
  proxy_style(*this, *m_input_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(padding(scale_width(1)));
    style.get(Disabled() > is_a<TextBox>()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(TextColor(QColor(0xC8C8C8)));
  });
  update_label_color(m_current->get());
  m_highlight_picker = new HighlightPicker(m_current, *this);
  m_highlight_picker_panel = m_highlight_picker->window();
  m_highlight_picker_panel->installEventFilter(this);
  get_highlight_palette(*m_highlight_picker)->connect_submit_signal(
    std::bind_front(&HighlightBox::on_palette_submit, this));
  get_background_color_box(*m_highlight_picker)->installEventFilter(this);
  get_text_color_box(*m_highlight_picker)->installEventFilter(this);
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&HighlightBox::on_current, this));
  m_focus_observer.connect_state_signal(
    std::bind_front(&HighlightBox::on_focus, this));
  m_press_observer.connect_press_end_signal(
    std::bind_front(&HighlightBox::on_press_end, this));
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
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      m_highlight_picker->hide();
      submit();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void HighlightBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(!is_read_only()) {
      submit();
    }
  }
  QWidget::keyPressEvent(event);
}

void HighlightBox::submit() {
  m_is_modified = false;
  m_submission = m_current->get();
  m_submit_signal(m_submission);
}

void HighlightBox::update_label_color(const HighlightColor& highlight) {
  update_style(*m_input_box->get_body(), [&] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(highlight.m_background_color)).
      set(TextColor(highlight.m_text_color));
  });
}

void HighlightBox::on_palette_submit(const std::any& submission) {
  m_highlight_picker->hide();
  submit();
}

void HighlightBox::on_current(const HighlightColor& current) {
  m_is_modified = true;
  update_label_color(current);
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
