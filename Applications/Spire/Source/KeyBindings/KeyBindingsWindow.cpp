#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include <QKeyEvent>
#include <QTabBar>
#include <QVBoxLayout>
#include "Spire/KeyBindings/CancelKeyBindingsTableWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

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
  layout->setContentsMargins(0, scale_height(8), 0, scale_width(8));
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
      margin: %3px %4px %3px %7px;
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
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1))
        .arg(scale_width(8)));
  layout->addWidget(m_tab_widget);
  connect(m_tab_widget, &QTabWidget::currentChanged, this,
    &KeyBindingsWindow::on_tab_changed);
  m_tab_widget->tabBar()->installEventFilter(this);
  connect(m_tab_widget->tabBar(), &QTabBar::tabBarClicked, this,
    &KeyBindingsWindow::on_tab_bar_clicked);
  auto task_keys_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(task_keys_widget, tr("Task Keys"));
  auto cancel_keys_widget = new QWidget(m_tab_widget);
  auto cancel_keys_layout = new QVBoxLayout(cancel_keys_widget);
  cancel_keys_layout->setContentsMargins({});
  cancel_keys_layout->setSpacing(0);
  auto cancel_keys_label = new QLabel(tr(R"(Use <b>ESC</b> with any combination of <b>ALT</b>, <b>CTRL</b> and <b>SHIFT</b> to set a cancel key binding.)"));
  cancel_keys_label->setFixedHeight(scale_height(30));
  cancel_keys_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;
    margin-left: %2px;
  )").arg(scale_height(10)).arg(scale_width(8)));
  cancel_keys_layout->addWidget(cancel_keys_label);
  m_cancel_keys_table = new CancelKeyBindingsTableWidget(
    key_bindings.build_cancel_bindings(), this);
  cancel_keys_layout->addWidget(m_cancel_keys_table);
  m_tab_widget->addTab(cancel_keys_widget, tr("Cancel Keys"));
  auto interactions_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(interactions_widget, tr("Interactions"));
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(8), scale_height(18), 0, 0);
  button_layout->setSpacing(0);
  layout->addLayout(button_layout);
  auto reset_button = new FlatButton(tr("Reset Default"));
  reset_button->connect_clicked_signal([=] {
    on_reset_button_clicked();
  });
  reset_button->setFixedSize(scale(120, 26));
  auto button_style = reset_button->get_style();
  button_style.m_background_color = QColor("#EBEBEB");
  auto button_hover_style = reset_button->get_hover_style();
  button_hover_style.m_background_color = QColor("#4B23A0");
  button_hover_style.m_text_color = Qt::white;
  auto button_focus_style = reset_button->get_focus_style();
  button_focus_style.m_background_color = QColor("#EBEBEB");
  button_focus_style.m_border_color = QColor("#4B23A0");
  auto button_font = QFont();
  button_font.setFamily("Roboto");
  button_font.setPixelSize(scale_height(12));
  reset_button->setFont(button_font);
  reset_button->set_style(button_style);
  reset_button->set_hover_style(button_hover_style);
  reset_button->set_focus_style(button_focus_style);
  button_layout->addWidget(reset_button);
  button_layout->addStretch(1);
  auto cancel_button = new FlatButton(tr("Cancel"));
  cancel_button->connect_clicked_signal([=] { close(); });
  cancel_button->setFixedSize(scale(100, 26));
  cancel_button->setFont(button_font);
  cancel_button->set_style(button_style);
  cancel_button->set_hover_style(button_hover_style);
  cancel_button->set_focus_style(button_focus_style);
  button_layout->addWidget(cancel_button);
  button_layout->addSpacing(scale_width(8));
  auto apply_button = new FlatButton(tr("Apply"));
  apply_button->connect_clicked_signal([=] {
    m_apply_signal();
  });
  apply_button->setFixedSize(scale(100, 26));
  apply_button->setFont(button_font);
  apply_button->set_style(button_style);
  apply_button->set_hover_style(button_hover_style);
  apply_button->set_focus_style(button_focus_style);
  button_layout->addWidget(apply_button);
  button_layout->addSpacing(scale_width(8));
  auto ok_button = new FlatButton(tr("OK"));
  ok_button->connect_clicked_signal([=] {
    m_apply_signal();
    close();
  });
  ok_button->setFixedSize(scale(100, 26));
  ok_button->setFont(button_font);
  ok_button->set_style(button_style);
  ok_button->set_hover_style(button_hover_style);
  ok_button->set_focus_style(button_focus_style);
  button_layout->addWidget(ok_button);
  button_layout->addSpacing(scale_width(8));
  m_tab_widget->setCurrentIndex(1);
  on_tab_bar_clicked(1);
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

void KeyBindingsWindow::on_reset_button_clicked() {
  auto default_bindings = KeyBindings::get_default_key_bindings();
  m_cancel_keys_table->set_key_bindings(
    default_bindings.build_cancel_bindings());
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
