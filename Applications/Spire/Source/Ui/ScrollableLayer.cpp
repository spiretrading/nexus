#include "Spire/Ui/ScrollableLayer.hpp"
#include <QApplication>
#include <QKeyEvent>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto get_scroll_track(const ScrollBar& scroll_bar) {
    return scroll_bar.findChild<Box*>();
  }

  auto has_range(const ScrollBar::Range& range) {
    return range.m_end - range.m_start > 1;
  }
}

ScrollableLayer::ScrollableLayer(QWidget* parent)
    : QWidget(parent),
      m_vertical_scroll_bar(new ScrollBar(Qt::Orientation::Vertical, this)),
      m_horizontal_scroll_bar(
        new ScrollBar(Qt::Orientation::Horizontal, this)),
      m_corner_box(new Box(nullptr)),
      m_is_horizontal_scroll_bar_opaque(true),
      m_is_vertical_scroll_bar_opaque(true) {
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
  m_horizontal_scroll_bar_style_connection = connect_style_signal(
    *get_scroll_track(*m_horizontal_scroll_bar), std::bind_front(
      &ScrollableLayer::on_horizontal_scroll_track_style, this));
  m_vertical_scroll_bar_style_connection =
    connect_style_signal(*get_scroll_track(*m_vertical_scroll_bar),
      std::bind_front(&ScrollableLayer::on_vertical_scroll_track_style, this));
}

ScrollBar& ScrollableLayer::get_vertical_scroll_bar() {
  return *m_vertical_scroll_bar;
}

ScrollBar& ScrollableLayer::get_horizontal_scroll_bar() {
  return *m_horizontal_scroll_bar;
}

void ScrollableLayer::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_line_up(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_Down) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_line_down(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_Right) {
    if(has_range(m_horizontal_scroll_bar->get_range())) {
      scroll_line_down(*m_horizontal_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_Left) {
    if(has_range(m_horizontal_scroll_bar->get_range())) {
      scroll_line_up(*m_horizontal_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_PageUp) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_page_up(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_PageDown) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_page_down(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_Home) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_to_start(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
  } else if(event->key() == Qt::Key_End) {
    if(has_range(m_vertical_scroll_bar->get_range())) {
      scroll_to_end(*m_vertical_scroll_bar);
    } else {
      event->ignore();
    }
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
  if(event->type() == QEvent::Resize || event->type() == QEvent::Move) {
    update_mask();
  } if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
    update_scroll_bar_position();
    update_mask();
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollableLayer::resizeEvent(QResizeEvent* event) {
  update_scroll_bar_position();
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
      auto corner_box_geometry = [&] {
        if(m_corner_box->isVisible()) {
          return m_corner_box->geometry();
        }
        return QRect(0, 0, 0, 0);
      }();
      return QPolygon(m_vertical_scroll_bar->geometry()).united(
        m_horizontal_scroll_bar->geometry()).united(corner_box_geometry);
    } else if(m_vertical_scroll_bar->isVisible()) {
      return QPolygon(m_vertical_scroll_bar->geometry());
    } else if(m_horizontal_scroll_bar->isVisible()) {
      return QPolygon(m_horizontal_scroll_bar->geometry());
    } else {
      return QPolygon();
    }
  }();
  if(region.isEmpty()) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
  } else {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMask(region);
  }
}

void ScrollableLayer::update_scroll_bar_position() {
  if(!m_is_horizontal_scroll_bar_opaque && !m_is_vertical_scroll_bar_opaque) {
    m_horizontal_scroll_bar->setGeometry(
      0, height() - m_horizontal_scroll_bar->sizeHint().height(),
      width(), m_horizontal_scroll_bar->sizeHint().height());
    m_vertical_scroll_bar->setGeometry(
      width() - m_vertical_scroll_bar->sizeHint().width(), 0,
      m_vertical_scroll_bar->sizeHint().width(), height());
  } else if(!m_is_horizontal_scroll_bar_opaque) {
    auto padding = [&] {
      if(m_vertical_scroll_bar->isVisible()) {
        return m_vertical_scroll_bar->sizeHint().width();
      }
      return 0;
    }();
    m_horizontal_scroll_bar->setGeometry(
      0, height() - m_horizontal_scroll_bar->sizeHint().height(),
      width() - padding, m_horizontal_scroll_bar->sizeHint().height());
  } else if(!m_is_vertical_scroll_bar_opaque) {
    auto padding = [&] {
      if(m_horizontal_scroll_bar->isVisible()) {
        return m_horizontal_scroll_bar->sizeHint().height();
      }
      return 0;
    }();
    m_vertical_scroll_bar->setGeometry(
      width() - m_vertical_scroll_bar->sizeHint().width(), 0,
      m_vertical_scroll_bar->sizeHint().width(), height() - padding);
  }
}

void ScrollableLayer::on_horizontal_scroll_track_style() {
  auto& stylist = find_stylist(*get_scroll_track(*m_horizontal_scroll_bar));
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          auto is_opaque = [&] {
            if(color.alpha() == 255) {
              return true;
            }
            return false;
          }();
          if(is_opaque == m_is_horizontal_scroll_bar_opaque) {
            return;
          }
          m_is_horizontal_scroll_bar_opaque = is_opaque;
          if(m_is_horizontal_scroll_bar_opaque) {
            m_layout->addWidget(m_horizontal_scroll_bar, 1, 0);
            if(m_is_vertical_scroll_bar_opaque) {
              m_corner_box->setVisible(true);
            }
          } else {
            m_layout->removeWidget(m_horizontal_scroll_bar);
            m_corner_box->setVisible(false);
          }
          update_scroll_bar_position();
        });
      });
  }
}

void ScrollableLayer::on_vertical_scroll_track_style() {
  auto& stylist = find_stylist(*get_scroll_track(*m_vertical_scroll_bar));
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          auto is_opaque = [&] {
            if(color.alpha() == 255) {
              return true;
            }
            return false;
          }();
          if(is_opaque == m_is_vertical_scroll_bar_opaque) {
            return;
          }
          m_is_vertical_scroll_bar_opaque = is_opaque;
          if(m_is_vertical_scroll_bar_opaque) {
            m_layout->addWidget(m_vertical_scroll_bar, 0, 1);
            if(m_is_horizontal_scroll_bar_opaque) {
              m_corner_box->setVisible(true);
            }
          } else {
            m_layout->removeWidget(m_vertical_scroll_bar);
            m_corner_box->setVisible(false);
          }
          update_scroll_bar_position();
        });
      });
  }
}
