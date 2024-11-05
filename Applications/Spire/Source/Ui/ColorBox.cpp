#include "Spire/Ui/ColorBox.hpp"
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
  const auto& CHEQUERED_BOARD_IMAGE() {
    static auto image = QPixmap(":/Icons/chequered-board.png");
    return image;
  }

  auto get_color_code_panel(const ColorPicker& color_picker) {
    auto index = color_picker.layout()->count() - 1;
    return color_picker.layout()->itemAt(index)->widget();
  }

  auto get_input_boxes(const ColorPicker& color_picker) {
    auto input_boxes = std::vector<QWidget*>();
    auto color_code_panel = get_color_code_panel(color_picker);
    auto& children = color_code_panel->children();
    for(auto i = 0; i < children.count(); ++i) {
      if(children[i]->isWidgetType()) {
        if(auto stacked_widget = dynamic_cast<QStackedWidget*>(children[i])) {
          input_boxes.push_back(stacked_widget->widget(0));
          auto rgb_color_box = stacked_widget->widget(1);
          input_boxes.push_back(rgb_color_box->layout()->itemAt(0)->widget());
          input_boxes.push_back(rgb_color_box->layout()->itemAt(2)->widget());
          input_boxes.push_back(rgb_color_box->layout()->itemAt(4)->widget());
          auto hsv_color_box = stacked_widget->widget(2);
          input_boxes.push_back(hsv_color_box->layout()->itemAt(0)->widget());
          input_boxes.push_back(hsv_color_box->layout()->itemAt(2)->widget());
          input_boxes.push_back(hsv_color_box->layout()->itemAt(4)->widget());
        } else {
          input_boxes.push_back(static_cast<QWidget*>(children[i]));
        }
      }
    }
    return input_boxes;
  }
}

struct CheckerBoard : QWidget {
  using QWidget::QWidget;

  void paintEvent(QPaintEvent* event) override {
    auto painter = QPainter(this);
    painter.drawTiledPixmap(rect(), CHEQUERED_BOARD_IMAGE());
  }
};

ColorBox::ColorBox(QWidget* parent)
  : ColorBox(std::make_shared<LocalColorModel>(), parent) {}

ColorBox::ColorBox(std::shared_ptr<ColorModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_color_picker(nullptr),
      m_color_picker_panel(nullptr),
      m_is_read_only(false),
      m_is_modified(false),
      m_focus_observer(*this),
      m_press_observer(*this) {
  setFocusPolicy(Qt::StrongFocus);
  auto checker_board = new CheckerBoard();
  checker_board->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_color_display = new Box();
  m_color_display->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_color_display, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(m_current->get()));
    style.get(Disabled()).set(BackgroundColor(QColor(0xF5F5F5)));
  });
  auto body = new LayeredWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  body->add(checker_board);
  body->add(m_color_display);
  m_input_box = make_input_box(body);
  m_input_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *m_input_box);
  proxy_style(*this, *m_input_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(padding(scale_width(1)));
  });
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&ColorBox::on_current, this));
  m_focus_observer.connect_state_signal(
    std::bind_front(&ColorBox::on_focus, this));
  m_press_observer.connect_press_end_signal(
    std::bind_front(&ColorBox::on_press_end, this));
}

const std::shared_ptr<ColorModel>& ColorBox::get_current() const {
  return m_current;
}

bool ColorBox::is_read_only() const {
  return m_is_read_only;
}

void ColorBox::set_read_only(bool is_read_only) {
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    match(*m_input_box, ReadOnly());
    m_color_picker->hide();
  } else {
    unmatch(*m_input_box, ReadOnly());
  }
}

connection ColorBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ColorBox::eventFilter(QObject* watched, QEvent* event) {
  if(m_color_picker_panel == watched) {
    if(event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        return true;
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      submit();
      m_color_picker->hide();
    } else if(key_event.key() == Qt::Key_Escape) {
      if(m_submission != m_current->get()) {
        m_current->set(m_submission);
      }
      m_is_modified = false;
      m_color_picker->hide();
      return true;
    }
  } else if(event->type() == QEvent::ChildAdded) {
    auto& child = *static_cast<QChildEvent&>(*event).child();
    if(child.isWidgetType()) {
      child.installEventFilter(this);
      watched->removeEventFilter(this);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(!is_read_only()) {
      submit();
    }
  }
  QWidget::keyPressEvent(event);
}

void ColorBox::submit() {
  m_is_modified = false;
  m_submission = m_current->get();
  m_submit_signal(m_submission);
}

void ColorBox::on_current(const QColor& current) {
  m_is_modified = true;
  update_style(*m_color_display, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(current));
  });
}

void ColorBox::show_color_picker() {
  if(!m_color_picker) {
    m_color_picker = new ColorPicker(m_current, *this);
    m_color_picker->installEventFilter(this);
    link(*this, *m_color_picker);
    m_color_picker_panel = m_color_picker->window();
    m_color_picker_panel->installEventFilter(this);
    auto input_boxes = get_input_boxes(*m_color_picker);
    for(auto input_box : input_boxes) {
      find_focus_proxy(*input_box)->installEventFilter(this);
    }
  }
  if(!m_color_picker->isVisible()) {
    m_color_picker->show();
    get_color_code_panel(*m_color_picker)->nextInFocusChain()->setFocus();
  }
}

void ColorBox::on_focus(FocusObserver::State state) {
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

void ColorBox::on_press_end(PressObserver::Reason reason) {
  if(!is_read_only()) {
    show_color_picker();
  }
}
