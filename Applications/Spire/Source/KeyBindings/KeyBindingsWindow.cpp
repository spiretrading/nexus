#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/KeyBindings/OrderTasksPage.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/NavigationView.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void copy(AnyListModel& from, AnyListModel& to) {
    to.transact([&] {
      clear(to);
      for(auto i = 0; i < from.get_size(); ++i) {
        to.push(from.get(i));
      }
    });
  }
}

KeyBindingsWindow::KeyBindingsWindow(
    std::shared_ptr<KeyBindingsModel> key_bindings, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_order_tasks_submission(std::make_shared<ArrayListModel<OrderTask>>()),
      m_is_modified(false) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  resize(scale(926, 476));
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  m_navigation_view = new NavigationView();
  m_navigation_view->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_order_tasks_page = new OrderTasksPage(
    m_key_bindings->get_region_query_model(), m_key_bindings->get_order_tasks(),
    m_key_bindings->get_destinations(), m_key_bindings->get_markets());
  m_order_tasks_page->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_navigation_view->add_tab(*m_order_tasks_page, tr("Order Tasks"));
  body_layout->addWidget(m_navigation_view);
  auto buttons_body = new QWidget();
  auto buttons_body_layout = make_hbox_layout(buttons_body);
  buttons_body_layout->setSpacing(scale_width(8));
  auto reset_button = make_label_button(tr("Reset to Default"));
  reset_button->setFixedWidth(scale_width(120));
  reset_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_reset, this));
  buttons_body_layout->addWidget(reset_button);
  buttons_body_layout->addStretch(1);
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedWidth(scale_width(100));
  cancel_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_cancel, this));
  buttons_body_layout->addWidget(cancel_button);
  auto apply_button = make_label_button(tr("Apply"));
  apply_button->setFixedWidth(scale_width(100));
  apply_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_apply, this));
  buttons_body_layout->addWidget(apply_button);
  auto ok_button = make_label_button(tr("OK"));
  ok_button->setFixedWidth(scale_width(100));
  ok_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_ok, this));
  buttons_body_layout->addWidget(ok_button);
  auto buttons = new Box(buttons_body);
  update_style(*buttons, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  body_layout->addWidget(buttons);
  auto box = new Box(body);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xF5F5F5)));
  });
  layout()->addWidget(box);
  copy(*m_order_tasks_page->get_order_tasks(), *m_order_tasks_submission);
  m_order_tasks_connection =
    m_order_tasks_page->get_order_tasks()->connect_operation_signal(
      std::bind_front(&KeyBindingsWindow::on_order_task_operation, this));
}

connection KeyBindingsWindow::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void KeyBindingsWindow::on_apply() {
  if(m_is_modified) {
    m_is_modified = false;
    copy(*m_order_tasks_page->get_order_tasks(), *m_order_tasks_submission);
  }
  m_submit_signal(m_order_tasks_submission);
}

void KeyBindingsWindow::on_cancel() {
  if(m_is_modified) {
    m_is_modified = false;
    copy(*m_order_tasks_submission, *m_order_tasks_page->get_order_tasks());
  }
  m_submit_signal(m_order_tasks_submission);
  close();
}

void KeyBindingsWindow::on_ok() {
  m_submit_signal(m_order_tasks_page->get_order_tasks());
  close();
}

void KeyBindingsWindow::on_reset() {
  copy(*m_key_bindings->get_default_order_tasks(),
    *m_order_tasks_page->get_order_tasks());
}

void KeyBindingsWindow::on_order_task_operation(
    const ListModel<OrderTask>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<OrderTask>::StartTransaction&) {
    },
    [&] (const ListModel<OrderTask>::EndTransaction&) {
    },
    [&] (const auto&) {
      m_is_modified = true;
    });
}
