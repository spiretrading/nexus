#include "Spire/Ui/ScrollableLayer.hpp"
#include <QApplication>
#include <QKeyEvent>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollableLayer::ScrollableLayer(QWidget* parent)
    : QWidget(parent),
      m_vertical_scroll_bar(new ScrollBar(Qt::Orientation::Vertical, this)),
      m_horizontal_scroll_bar(
        new ScrollBar(Qt::Orientation::Horizontal, this)),
      m_corner_box(new Box(nullptr)),
      m_horizontal_display_policy(ScrollBox::DisplayPolicy::ON_OVERFLOW),
      m_vertical_display_policy(ScrollBox::DisplayPolicy::ON_OVERFLOW) {
  m_layout = make_grid_layout(this);
  m_layout->setColumnStretch(0, 1);
  m_layout->addItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 0,
    0);
  m_layout->addWidget(m_vertical_scroll_bar, 0, 1);
  m_layout->addWidget(m_horizontal_scroll_bar, 1, 0);
  update_style(*m_corner_box, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFF, 0xFF, 0xFF)));
  });
  m_corner_box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  m_layout->addWidget(m_corner_box, 1, 1);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  m_vertical_scroll_bar->installEventFilter(this);
  m_horizontal_scroll_bar->installEventFilter(this);
  m_corner_box->installEventFilter(this);
}

ScrollBar& ScrollableLayer::get_vertical_scroll_bar() {
  return *m_vertical_scroll_bar;
}

ScrollBar& ScrollableLayer::get_horizontal_scroll_bar() {
  return *m_horizontal_scroll_bar;
}

void ScrollableLayer::update_layout(ScrollBox::DisplayPolicy horizontal_policy,
    ScrollBox::DisplayPolicy vertical_policy) {
  if(m_horizontal_display_policy == horizontal_policy && m_vertical_display_policy == vertical_policy) {
    return;
  }
  m_horizontal_display_policy = horizontal_policy;
  m_vertical_display_policy = vertical_policy;
  if(m_horizontal_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE &&
    m_vertical_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    m_layout->removeWidget(m_horizontal_scroll_bar);
    m_layout->removeWidget(m_vertical_scroll_bar);
    m_corner_box->setVisible(false);
    m_horizontal_scroll_bar->move(0, height() - m_horizontal_scroll_bar->sizeHint().height());
    m_horizontal_scroll_bar->resize(width(), m_horizontal_scroll_bar->sizeHint().height());
    static_cast<Box*>(m_horizontal_scroll_bar->layout()->itemAt(0)->widget())->get_body()->move(0, 0);
    m_vertical_scroll_bar->move(width() - m_vertical_scroll_bar->sizeHint().width(), 0);
    m_vertical_scroll_bar->resize(m_vertical_scroll_bar->sizeHint().width(), height());
    static_cast<Box*>(m_vertical_scroll_bar->layout()->itemAt(0)->widget())->get_body()->move(0, 0);
    m_horizontal_scroll_bar->raise();
  } else if(m_horizontal_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    m_layout->removeWidget(m_horizontal_scroll_bar);
    m_corner_box->setVisible(false);
    m_horizontal_scroll_bar->move(0, height() - m_horizontal_scroll_bar->sizeHint().height());
    auto padding = [&] {
      if(m_vertical_scroll_bar->isVisible()) {
        return m_vertical_scroll_bar->sizeHint().width();
      }
      return 0;
    }();
    m_horizontal_scroll_bar->resize(width() - padding, m_horizontal_scroll_bar->sizeHint().height());
    static_cast<Box*>(m_horizontal_scroll_bar->layout()->itemAt(0)->widget())->get_body()->move(0, 0);
  } else if(m_vertical_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    m_layout->removeWidget(m_vertical_scroll_bar);
    m_corner_box->setVisible(false);
    m_vertical_scroll_bar->move(width() - m_vertical_scroll_bar->sizeHint().width(), 0);
    auto padding = [&] {
      if(m_horizontal_scroll_bar->isVisible()) {
        return m_horizontal_scroll_bar->sizeHint().height();
      }
      return 0;
    }();
    m_vertical_scroll_bar->resize(m_vertical_scroll_bar->sizeHint().width(), height() - padding);
    static_cast<Box*>(m_vertical_scroll_bar->layout()->itemAt(0)->widget())->get_body()->move(0, 0);
  } else {
    m_layout->addWidget(m_vertical_scroll_bar, 0, 1);
    m_layout->addWidget(m_horizontal_scroll_bar, 1, 0);
    m_corner_box->setVisible(true);
  }
}

void ScrollableLayer::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up) {
    scroll_line_up(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Down) {
    scroll_line_down(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Right) {
    scroll_line_down(*m_horizontal_scroll_bar);
  } else if(event->key() == Qt::Key_Left) {
    scroll_line_up(*m_horizontal_scroll_bar);
  } else if(event->key() == Qt::Key_PageUp) {
    scroll_page_up(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_PageDown) {
    scroll_page_down(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Home) {
    scroll_to_start(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_End) {
    scroll_to_end(*m_vertical_scroll_bar);
  } else {
    event->ignore();
  }
}

void ScrollableLayer::wheelEvent(QWheelEvent* event) {
  auto scroll_bar = [&] {
    if(event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier)) {
      return m_horizontal_scroll_bar;
    } else {
      return m_vertical_scroll_bar;
    }
  }();
  if(event->angleDelta().y() < 0) {
    scroll_line_down(*scroll_bar, QApplication::wheelScrollLines());
  } else if(event->angleDelta().y() > 0) {
    scroll_line_up(*scroll_bar, QApplication::wheelScrollLines());
  }
  if(event->angleDelta().x() < 0) {
    scroll_line_down(*m_horizontal_scroll_bar);
  } else if(event->angleDelta().x() > 0) {
    scroll_line_up(*m_horizontal_scroll_bar);
  }
}

bool ScrollableLayer::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize || event->type() == QEvent::Move ||
      event->type() == QEvent::Show || event->type() == QEvent::Hide) {
    qDebug() << watched << " " << event;
    update_mask();
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollableLayer::resizeEvent(QResizeEvent* event) {
  if(m_horizontal_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE &&
      m_vertical_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    m_horizontal_scroll_bar->resize(width(), m_horizontal_scroll_bar->sizeHint().height());
    m_vertical_scroll_bar->resize(m_vertical_scroll_bar->sizeHint().width(), height());
  } else if(m_horizontal_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    auto padding = [&] {
      if(m_vertical_scroll_bar->isVisible()) {
        return m_vertical_scroll_bar->sizeHint().width();
      }
      return 0;
    }();
    m_horizontal_scroll_bar->resize(width() - padding, m_horizontal_scroll_bar->sizeHint().height());
  } else if(m_vertical_display_policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
    auto padding = [&] {
      if(m_horizontal_scroll_bar->isVisible()) {
        return m_horizontal_scroll_bar->sizeHint().height();
      }
      return 0;
    }();
    m_vertical_scroll_bar->resize(m_vertical_scroll_bar->sizeHint().width(), height() - padding);
  }
  update_mask();
}

void ScrollableLayer::update_mask() {
  if(m_vertical_scroll_bar->isHidden() && m_horizontal_scroll_bar->isHidden()) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    return;
  }
  auto region = [&] {
    if(m_vertical_scroll_bar->isVisible() &&
        m_horizontal_scroll_bar->isVisible()) {
      return QPolygon(m_vertical_scroll_bar->geometry()).united(
        m_horizontal_scroll_bar->geometry()).united(m_corner_box->geometry());
    } else if(m_vertical_scroll_bar->isVisible()) {
      return QPolygon(m_vertical_scroll_bar->geometry());
    }
    return QPolygon(m_horizontal_scroll_bar->geometry());
  }();
  if(region.isEmpty()) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
  } else {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMask(region);
  }
}
