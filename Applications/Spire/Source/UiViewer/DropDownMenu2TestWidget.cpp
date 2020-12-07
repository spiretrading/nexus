#include "Spire/UiViewer/DropDownMenu2TestWidget.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto create_item(const QString& label, QWidget* parent) {
    auto item = new DropDownItem(label, parent);
    item->setFixedHeight(scale_height(20));
    return item;
  }
}

DropDownMenu2TestWidget::DropDownMenu2TestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto layout = new QHBoxLayout(container);
  auto label = new QLabel("Parent Input", this);
  label->setFixedSize(scale(100, 26));
  label->setFocusPolicy(Qt::StrongFocus);
  label->setStyleSheet(QString(R"(
    QLabel {
      background-color: white;
      border: 1px solid #C8C8C8;
      font-family: Roboto;
      padding-left: %1px;
    }

    QLabel:hover, QLabel:focus {
      border: 1px solid #4B23A0;
    }})").arg(scale_width(5)));
  layout->addWidget(label);
  m_menu = new DropDownMenu2({
    create_item("AA", this), create_item("AB", this), create_item("AC", this),
    create_item("BA", this), create_item("BB", this), create_item("BC", this),
    create_item("CA", this), create_item("CB", this), create_item("CC", this)},
    label);
  m_status_label = new QLabel(this);
  m_status_label->setFixedSize(scale(100, 26));
  layout->addWidget(m_status_label);
  m_menu->connect_current_signal([=] (const auto& item) {
    if(item.isValid()) {
      m_status_label->setText(QString("Current: %1").arg(item.toString()));
    }
  });
  m_menu->connect_hovered_signal([=] (const auto& item) {
    m_status_label->setText(QString("Hovered: %1").arg(item.toString()));
  });
  m_menu->connect_selected_signal([=] (const auto& item) {
    m_status_label->setText(QString("Selected: %1").arg(item.toString()));
  });
  label->installEventFilter(this);
}

bool DropDownMenu2TestWidget::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    switch(e->key()) {
      case Qt::Key_Down:
        if(!m_menu->isVisible()) {
          if(auto selected_index = m_menu->get_selected(); selected_index) {
            m_menu->set_current(*selected_index);
          }
          m_menu->show();
        }
        increment_current(*m_menu);
        return true;
      case Qt::Key_Up:
        if(m_menu->isVisible()) {
          decrement_current(*m_menu);
        }
        break;
      case Qt::Key_Enter:
      case Qt::Key_Return:
        if(m_menu->isVisible() && m_menu->get_current()) {
          m_menu->select_current_index();
        }
        break;
      case Qt::Key_Escape:
        m_menu->hide();
        break;
      case Qt::Key_Space:
        // TODO: Select?
        break;
    }
  } else if(event->type() == QEvent::MouseButtonPress) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) {
      m_menu->setVisible(!m_menu->isVisible());
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}
