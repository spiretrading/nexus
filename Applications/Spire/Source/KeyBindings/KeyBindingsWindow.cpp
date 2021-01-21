#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/KeyBindings/CancelKeyBindingsTableView.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

KeyBindingsWindow::KeyBindingsWindow(KeyBindings key_bindings,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_last_focus_was_key(false) {
  set_fixed_body_size(scale(871, 442));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(0, scale_height(8), 0, scale_width(8));
  layout->setSpacing(0);
  Window::layout()->addWidget(body);
  m_tab_widget = new TabWidget(TabWidget::PaddingStyle::NONE, this);
  layout->addWidget(m_tab_widget);
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
  auto restore_button = new FlatButton(tr("Restore Defaults"), this);
  connect(restore_button, &FlatButton::clicked, [=] {
    on_restore_button_clicked();
  });
  restore_button->setFixedSize(scale(120, 26));
  button_layout->addWidget(restore_button);
  button_layout->addStretch(1);
  auto cancel_button = new FlatButton(tr("Cancel"), this);
  connect(cancel_button, &FlatButton::clicked, [=] { close(); });
  cancel_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(cancel_button);
  button_layout->addSpacing(scale_width(8));
  auto apply_button = new FlatButton(tr("Apply"), this);
  connect(apply_button, &FlatButton::clicked, [=] {
    m_apply_signal();
  });
  apply_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(apply_button);
  button_layout->addSpacing(scale_width(8));
  auto ok_button = new FlatButton(tr("OK"), this);
  connect(ok_button, &FlatButton::clicked, [=] {
    on_ok_button_clicked();
  });
  ok_button->setFixedSize(scale(100, 26));
  button_layout->addWidget(ok_button);
  button_layout->addSpacing(scale_width(8));
  m_task_keys_table->setFocus();
}

const KeyBindings& KeyBindingsWindow::get_key_bindings() const {
  return m_key_bindings;
}

connection KeyBindingsWindow::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return scoped_connection();
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
