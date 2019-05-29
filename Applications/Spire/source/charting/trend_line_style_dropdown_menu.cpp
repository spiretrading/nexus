#include "spire/charting/trend_line_style_dropdown_menu.hpp"
#include <QPainter>
#include <QPaintEvent>
#include "spire/charting/style_dropdown_menu_list.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

TrendLineStyleDropdownMenu::TrendLineStyleDropdownMenu(QWidget* parent)
    : QWidget(parent),
      m_current_style(TrendLineStyle::SOLID),
      m_dropdown_image(imageFromSvg(":/icons/arrow-down.svg", scale(6, 4))) {
  setFocusPolicy(Qt::StrongFocus);
  m_menu_list = new StyleDropdownMenuList(this);
  m_menu_list->connect_selected_signal(
    [=] (auto style) { on_style_selected(style); });
  m_menu_list->hide();
  window()->installEventFilter(this);
}

TrendLineStyle TrendLineStyleDropdownMenu::get_style() const {
  return m_current_style;
}

bool TrendLineStyleDropdownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_menu_list->isVisible()) {
        move_menu_list();
      }
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_menu_list->hide();
    }
  }
  return false;
}

void TrendLineStyleDropdownMenu::focusOutEvent(QFocusEvent* event) {
  m_menu_list->hide();
  update();
}

void TrendLineStyleDropdownMenu::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    on_clicked();
  } else if(event->key() == Qt::Key_Down) {
    m_current_style = m_menu_list->get_next(m_current_style);
    update();
  } else if(event->key() == Qt::Key_Up) {
    m_current_style = m_menu_list->get_previous(m_current_style);
    update();
  }
  event->ignore();
}

void TrendLineStyleDropdownMenu::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_menu_list->isVisible()) {
      m_menu_list->hide();
    } else {
      on_clicked();
    }
  }
}

void TrendLineStyleDropdownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->hasFocus()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  // TODO: draw lines
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + scale_width(8)),
    scale_height(7)), m_dropdown_image);
}

void TrendLineStyleDropdownMenu::move_menu_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_menu_list->move(x_pos, y_pos);
  m_menu_list->raise();
}

void TrendLineStyleDropdownMenu::on_clicked() {
  move_menu_list();
  m_menu_list->setFixedWidth(width());
  m_menu_list->show();
  m_menu_list->raise();
}

void TrendLineStyleDropdownMenu::on_style_selected(TrendLineStyle style) {
  m_menu_list->hide();
  m_current_style = style;
  update();
}
