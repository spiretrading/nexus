#include "Spire/Ui/DropdownMenu.hpp"
#include <QKeyEvent>
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropdownMenuItem.hpp"
#include "Spire/Ui/DropdownMenuList.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

DropDownMenu::DropDownMenu(const std::vector<QString>& items,
    QWidget* parent)
    : QWidget(parent),
      m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
  if(!items.empty()) {
    m_current_text = items.front();
  }
  setFocusPolicy(Qt::StrongFocus);
  m_menu_list = new DropDownMenuList(items, this);
  m_menu_list->connect_selected_signal([=] (auto& t) { on_item_selected(t); });
  m_menu_list->installEventFilter(this);
  m_menu_list->hide();
  window()->installEventFilter(this);
}

void DropDownMenu::set_current_text(const QString& text) {
  m_current_text = text;
  update();
}

void DropDownMenu::set_items(const std::vector<QString>& items) {
  auto index = 0;
  auto list = static_cast<QScrollArea*>(
    m_menu_list->layout()->itemAt(0)->widget())->widget();
  for(auto i = 0; i < list->layout()->count(); ++i) {
    auto t = static_cast<DropDownMenuItem*>(
      list->layout()->itemAt(i)->widget())->text();
    if(t == m_current_text) {
      index = i;
    }
  }
  m_menu_list->set_items(items);
  if(m_menu_list->layout()->count() > 0) {
    m_current_text = items[index];
  }
  update();
}

const QString& DropDownMenu::get_text() const {
  return m_current_text;
}

connection DropDownMenu::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_menu_list->connect_highlighted_signal(slot);
}

connection DropDownMenu::connect_menu_closed_signal(
    const MenuClosedSignal::slot_type& slot) const {
  return m_menu_closed_signal.connect(slot);
}

connection DropDownMenu::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool DropDownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_menu_list->isVisible()) {
        move_menu_list();
      }
    } else if(event->type() == QEvent::WindowDeactivate &&
        !m_menu_list->isActiveWindow()) {
      hide_menu_list();
    } else if(event->type() == QEvent::MouseButtonPress) {
      hide_menu_list();
    }
  } else if(watched == m_menu_list) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Escape) {
        hide_menu_list();
      }
    }
  }
  return false;
}

void DropDownMenu::focusOutEvent(QFocusEvent* event) {
  hide_menu_list();
  update();
}

void DropDownMenu::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    on_clicked();
  } else if(event->key() == Qt::Key_Escape) {
    hide_menu_list();
  }
  event->ignore();
}

void DropDownMenu::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_menu_list->isVisible()) {
      hide_menu_list();
    } else {
      on_clicked();
    }
  }
}

void DropDownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->hasFocus()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  painter.drawText(QPoint(PADDING(),
    (height() / 2) + (metrics.ascent() / 2) - 1),
    metrics.elidedText(m_current_text, Qt::ElideRight,
    width() - (PADDING() * 3)));
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + PADDING()),
    scale_height(11)), m_dropdown_image);
}

void DropDownMenu::hide_menu_list() {
  m_menu_list->hide();
  m_menu_closed_signal();
}

void DropDownMenu::move_menu_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_menu_list->move(x_pos, y_pos + 1);
  m_menu_list->raise();
}

void DropDownMenu::on_clicked() {
  move_menu_list();
  m_menu_list->setFixedWidth(width());
  m_menu_list->show();
  m_menu_list->raise();
}

void DropDownMenu::on_item_selected(const QString& text) {
  hide_menu_list();
  m_current_text = text;
  m_selected_signal(m_current_text);
  update();
}
