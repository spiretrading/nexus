#include "Spire/Charting/TrendLineStyleDropDownMenu.hpp"
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Charting/StyleDropDownMenuList.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::signals2;
using namespace Spire;

TrendLineStyleDropDownMenu::TrendLineStyleDropDownMenu(
    QWidget* parent)
    : QWidget(parent),
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
  m_current_style = TrendLineStyle::SOLID;
  setFocusPolicy(Qt::StrongFocus);
  m_menu_list = new StyleDropDownMenuList(this);
  m_menu_list->connect_selected_signal([=] (auto t) { on_item_selected(t); });
  m_menu_list->hide();
  m_drop_shadow = std::make_unique<DropShadow>(false, m_menu_list);
  window()->installEventFilter(this);
}

TrendLineStyle TrendLineStyleDropDownMenu::get_style() const {
  return m_current_style;
}

connection TrendLineStyleDropDownMenu::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

bool TrendLineStyleDropDownMenu::eventFilter(QObject* watched, QEvent* event) {
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

void TrendLineStyleDropDownMenu::focusOutEvent(QFocusEvent* event) {
  m_menu_list->hide();
  update();
}

void TrendLineStyleDropDownMenu::keyPressEvent(QKeyEvent* event) {
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

void TrendLineStyleDropDownMenu::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_menu_list->isVisible()) {
      m_menu_list->hide();
    } else {
      on_clicked();
    }
  }
}

void TrendLineStyleDropDownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->hasFocus()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto line_y = event->rect().height() / 2;
  draw_trend_line(painter, m_current_style, Qt::black, scale_width(8),
    line_y, scale_width(8) + scale_width(30), line_y);
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + scale_width(8)),
    scale_height(8)), m_dropdown_image);
}

void TrendLineStyleDropDownMenu::move_menu_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_menu_list->move(x_pos, y_pos + 1);
  m_menu_list->raise();
}

void TrendLineStyleDropDownMenu::on_clicked() {
  move_menu_list();
  m_menu_list->setFixedWidth(width());
  m_menu_list->show();
  m_menu_list->raise();
}

void TrendLineStyleDropDownMenu::on_item_selected(TrendLineStyle style) {
  m_menu_list->hide();
  m_current_style = style;
  m_style_signal(m_current_style);
  update();
}
