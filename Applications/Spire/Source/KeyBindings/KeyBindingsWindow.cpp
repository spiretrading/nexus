#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto create_button(const QString& text, QWidget* parent) {
    auto button = new FlatButton(text, parent);
    auto button_style = button->get_style();
    button_style.m_background_color = QColor("#EBEBEB");
    auto button_hover_style = button->get_hover_style();
    button_hover_style.m_background_color = QColor("#4B23A0");
    button_hover_style.m_text_color = Qt::white;
    auto button_focus_style = button->get_focus_style();
    button_focus_style.m_background_color = QColor("#EBEBEB");
    button_focus_style.m_border_color = QColor("#4B23A0");
    auto button_font = QFont();
    button_font.setFamily("Roboto");
    button_font.setPixelSize(scale_height(12));
    button->setFont(button_font);
    button->set_style(button_style);
    button->set_hover_style(button_hover_style);
    button->set_focus_style(button_focus_style);
    return button;
  }
}

KeyBindingsWindow::KeyBindingsWindow(KeyBindings key_bindings,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_last_focus_was_key(false) {
  set_fixed_body_size(scale(871, 442));
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
  m_tab_widget = new CustomTabWidget(this);
  layout->addWidget(m_tab_widget);
  connect(m_tab_widget, &QTabWidget::currentChanged, this,
    &KeyBindingsWindow::on_tab_changed);
  m_tab_widget->tabBar()->installEventFilter(this);
  connect(m_tab_widget->tabBar(), &QTabBar::tabBarClicked, this,
    &KeyBindingsWindow::on_tab_bar_clicked);
  auto task_keys_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(task_keys_widget, tr("Task Keys"));
  auto task_keys_layout = new QVBoxLayout(task_keys_widget);
  task_keys_layout->setContentsMargins({});
  task_keys_layout->setSpacing(0);
  auto task_text = tr("Use <b>F1-F12</b> with any combination of <b>ALT</b>, <b>CTRL</b> and <b>SHIFT</b> to set a key binding.");
  auto task_keys_label = new QLabel(task_text, this);
  task_keys_label->setFixedHeight(scale_height(30));
  auto tab_label_style = QString(R"(
    font-family: Roboto;
    font-size: %1px;
    margin-left: %2px;
  )").arg(scale_height(10)).arg(scale_width(8));
  task_keys_label->setStyleSheet(tab_label_style);
  task_keys_layout->addWidget(task_keys_label);
  m_task_keys_table = new TaskKeyBindingsTableView(
    m_key_bindings.build_order_bindings(), input_model, this);
  task_keys_layout->addWidget(m_task_keys_table);
  auto cancel_keys_widget = new QWidget(m_tab_widget);
  auto cancel_keys_layout = new QVBoxLayout(cancel_keys_widget);
  cancel_keys_layout->setContentsMargins({});
  cancel_keys_layout->setSpacing(0);
  auto cancel_text = tr("Use <b>ESC</b> with any combination of <b>ALT</b>, <b>CTRL</b> and <b>SHIFT</b> to set a cancel key binding.");
  auto cancel_keys_label = new QLabel(cancel_text, this);
  cancel_keys_label->setFixedHeight(scale_height(30));
  cancel_keys_label->setStyleSheet(tab_label_style);
  cancel_keys_layout->addWidget(cancel_keys_label);
  m_cancel_keys_table = new CancelKeyBindingsTableView(
    m_key_bindings.build_cancel_bindings(), this);
  cancel_keys_layout->addWidget(m_cancel_keys_table);
  m_tab_widget->addTab(cancel_keys_widget, tr("Cancel Keys"));
  auto interactions_widget = new QWidget(m_tab_widget);
  m_tab_widget->addTab(interactions_widget, tr("Interactions"));
  auto padding_widget = new QWidget(this);
  padding_widget->setFixedHeight(scale_height(12));
  layout->addWidget(padding_widget);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(8), scale_height(18), 0, 0);
  button_layout->setSpacing(0);
  layout->addLayout(button_layout);
  auto restore_button = create_button(tr("Restore Defaults"), this);
  restore_button->connect_clicked_signal([=] {
    on_restore_button_clicked();
  });
  restore_button->setFixedSize(scale(120, 26));
  button_layout->addWidget(restore_button);
  button_layout->addStretch(1);
  auto cancel_button = create_button(tr("Cancel"), this);
  cancel_button->connect_clicked_signal([=] { close(); });
  cancel_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(cancel_button);
  button_layout->addSpacing(scale_width(8));
  auto apply_button = create_button(tr("Apply"), this);
  apply_button->connect_clicked_signal(m_apply_signal);
  apply_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(apply_button);
  button_layout->addSpacing(scale_width(8));
  auto ok_button = create_button(tr("OK"), this);
  ok_button->connect_clicked_signal([=] {
    on_ok_button_clicked();
  });
  ok_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(ok_button);
  button_layout->addSpacing(scale_width(8));
  m_tab_widget->setCurrentIndex(0);
  on_tab_bar_clicked(0);
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

void KeyBindingsWindow::on_ok_button_clicked() {
  m_apply_signal();
  close();
}

void KeyBindingsWindow::on_restore_button_clicked() {
  auto default_bindings = KeyBindings::get_default_key_bindings();
  m_task_keys_table->set_key_bindings(default_bindings.build_order_bindings());
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
