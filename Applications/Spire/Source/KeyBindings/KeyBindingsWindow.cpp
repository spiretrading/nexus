#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include <QKeyEvent>
#include <QTabBar>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

KeyBindingsWindow::KeyBindingsWindow(const KeyBindings& key_bindings,
    QWidget* parent)
    : Window(parent),
      m_key_bindings(key_bindings),
      m_last_focus_was_key(false){
  set_fixed_body_size(scale(853, 442));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setWindowIcon(QIcon(":/Icons/key-bindings-icon-256x256.png"));
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings-black.svg",
    ":/Icons/key-bindings-grey.svg");
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(scale_width(8), scale_height(8), scale_width(8),
    scale_height(8));
  layout->setSpacing(0);
  Window::layout()->addWidget(body);
  m_tab_widget = new QTabWidget(this);
  m_tab_widget->tabBar()->setFixedHeight(scale_height(40));
  m_tab_widget->setStyleSheet(QString(R"(
    QWidget {
      outline: none;
    }

    QTabWidget::pane {
      border: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:focus {
      border: %6px solid #4B23A0;
      padding: -%6px 0px 0px -%6px;
    }

    QTabBar::tab:hover {
      color: #4B23A0;
    }

    QTabBar::tab:selected {
      background-color: #F5F5F5;
      color: #4B23A0;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1)));
  layout->addWidget(m_tab_widget);
  connect(m_tab_widget, &QTabWidget::currentChanged, this,
    &KeyBindingsWindow::on_tab_changed);
  m_tab_widget->tabBar()->installEventFilter(this);
  connect(m_tab_widget->tabBar(), &QTabBar::tabBarClicked, this,
    &KeyBindingsWindow::on_tab_bar_clicked);
  auto task_keys_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(task_keys_widget, tr("Task Keys"));
  auto cancel_keys_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(cancel_keys_widget, tr("Cancel Keys"));
  auto interactions_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(interactions_widget, tr("Interactions"));
}

const KeyBindings& KeyBindingsWindow::get_key_bindings() const {
  return m_key_bindings;
}

connection KeyBindingsWindow::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return scoped_connection();
}

bool KeyBindingsWindow::eventFilter(QObject* watched,
    QEvent* event) {
  if(watched == m_tab_widget->tabBar()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
        m_last_focus_was_key = true;
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      m_last_focus_was_key = false;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void KeyBindingsWindow::on_tab_bar_clicked(int index) {
  if(index > -1) {
    m_tab_widget->widget(index)->setFocus();
  }
}

void KeyBindingsWindow::on_tab_changed() {
  if(m_last_focus_was_key) {
    m_tab_widget->tabBar()->setFocus();
  }
}
