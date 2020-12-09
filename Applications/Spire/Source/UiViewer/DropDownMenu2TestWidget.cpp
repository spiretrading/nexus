#include "Spire/UiViewer/DropDownMenu2TestWidget.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/LabelDropDownMenuItem.hpp"

using namespace Spire;

namespace {
  class MenuLabel : public QLabel {
    public:
      MenuLabel(const QString& label, QLabel* status_label, QWidget* parent);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QLabel* m_status_label;
      DropDownMenu2* m_menu;

      void move_menu();
      void toggle_menu_visibility();
  };

  auto create_item(const QString& label, QWidget* parent) {
    auto item = new LabelDropDownMenuItem(label, parent);
    item->setFixedHeight(scale_height(20));
    return item;
  }
}

DropDownMenu2TestWidget::DropDownMenu2TestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto layout = new QHBoxLayout(container);
  auto status_label = new QLabel(this);
  auto label = new MenuLabel("Parent Input", status_label, this);
  layout->addWidget(label);
  status_label->setFixedSize(scale(100, 26));
  layout->addWidget(status_label);
}

MenuLabel::MenuLabel(const QString& label, QLabel* status_label,
    QWidget* parent)
    : QLabel(label, parent),
      m_status_label(status_label) {
  setFixedSize(scale(100, 26));
  setFocusPolicy(Qt::StrongFocus);
  setStyleSheet(QString(R"(
    QLabel {
      background-color: white;
      border: 1px solid #C8C8C8;
      font-family: Roboto;
      padding-left: %1px;
    }

    QLabel:hover, QLabel:focus {
      border: 1px solid #4B23A0;
    }})").arg(scale_width(5)));
  m_menu = new DropDownMenu2({
    create_item("AA", this), create_item("AB", this), create_item("AC", this),
    create_item("BA", this), create_item("BB", this), create_item("BC", this),
    create_item("CA", this), create_item("CB", this), create_item("CC", this)},
    this);
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
  window()->installEventFilter(this);
}

bool MenuLabel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::WindowDeactivate) {
    m_menu->hide();
  } else if(event->type() == QEvent::Move) {
    move_menu();
  }
  return QWidget::eventFilter(watched, event);
}

void MenuLabel::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Space) {
    event->accept();
    toggle_menu_visibility();
  } else if(m_menu->isVisible()) {
    QCoreApplication::sendEvent(m_menu, event);
  }
  if(!event->isAccepted()) {
    QLabel::keyPressEvent(event);
  }
}

void MenuLabel::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    event->accept();
    toggle_menu_visibility();
  }
  if(!event->isAccepted()) {
    QLabel::mousePressEvent(event);
  }
}

void MenuLabel::moveEvent(QMoveEvent* event) {
  move_menu();
  QLabel::moveEvent(event);
}

void MenuLabel::resizeEvent(QResizeEvent* event) {
  m_menu->setFixedWidth(width());
  QLabel::resizeEvent(event);
}

void MenuLabel::move_menu() {
  m_menu->move(mapToGlobal(QPoint(0, height())));
}

void MenuLabel::toggle_menu_visibility() {
  m_menu->setVisible(!m_menu->isVisible());
  if(m_menu->isVisible()) {
    move_menu();
    m_menu->setFixedWidth(width());
  }
}
