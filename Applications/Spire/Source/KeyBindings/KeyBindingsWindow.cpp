#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include <QCloseEvent>
#include <QIcon>
#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void copy(const KeyBindingsModel& source, KeyBindingsModel& destination) {
    auto& destination_tasks = *destination.get_order_task_arguments();
    auto& source_tasks = *source.get_order_task_arguments();
    auto destination_size = destination_tasks.get_size();
    auto source_size = source_tasks.get_size();
    auto min_size = std::min(destination_size, source_size);
    auto front = 0;
    while(front < min_size &&
        destination_tasks.get(front) == source_tasks.get(front)) {
      ++front;
    }
    auto back = 0;
    while(back < min_size - front &&
        destination_tasks.get(destination_size - 1 - back) ==
          source_tasks.get(source_size - 1 - back)) {
      ++back;
    }
    auto destination_div = destination_size - front - back;
    auto source_div = source_size - front - back;
    destination_tasks.transact([&] {
      auto min_div = std::min(destination_div, source_div);
      for(auto i = 0; i < min_div; ++i) {
        if(destination_tasks.get(front + i) != source_tasks.get(front + i)) {
          destination_tasks.set(front + i, source_tasks.get(front + i));
        }
      }
      for(auto i = destination_div; i > source_div; --i) {
        destination_tasks.remove(front + i - 1);
      }
      for(auto i = destination_div; i < source_div; ++i) {
        destination_tasks.insert(source_tasks.get(front + i), front + i);
      }
    });
    auto& destination_cancel = *destination.get_cancel_key_bindings();
    auto& source_cancel = *source.get_cancel_key_bindings();
    for(auto i = 0; i < CancelKeyBindingsModel::OPERATION_COUNT; ++i) {
      auto operation = static_cast<CancelKeyBindingsModel::Operation>(i);
      destination_cancel.get_binding(operation)->set(
        source_cancel.get_binding(operation)->get());
    }
    auto source_scopes = source.make_interactions_key_bindings_scopes();
    for(auto& scope : destination.make_interactions_key_bindings_scopes()) {
      if(std::find(source_scopes.begin(), source_scopes.end(), scope) ==
          source_scopes.end()) {
        destination.get_interactions_key_bindings(scope)->reset();
      }
    }
    for(auto& scope : source_scopes) {
      auto& destination_interactions =
        *destination.get_interactions_key_bindings(scope);
      auto& source_interactions = *source.get_interactions_key_bindings(scope);
      destination_interactions.get_default_quantity()->set(
        source_interactions.get_default_quantity()->get());
      destination_interactions.is_cancel_on_fill()->set(
        source_interactions.is_cancel_on_fill()->get());
      for(auto i = 0; i < InteractionsKeyBindingsModel::MODIFIER_COUNT; ++i) {
        auto modifier = to_modifier(i);
        destination_interactions.get_quantity_increment(modifier)->set(
          source_interactions.get_quantity_increment(modifier)->get());
        destination_interactions.get_price_increment(modifier)->set(
          source_interactions.get_price_increment(modifier)->get());
      }
    }
  }
}

KeyBindingsWindow::KeyBindingsWindow(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<TickerInfoQueryModel> tickers,
    const AdditionalTagDatabase& additional_tags, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_is_committed(false) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  copy(*m_key_bindings, m_snapshot);
  auto navigation_view = new NavigationView();
  navigation_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto task_keys_page =
    new TaskKeysPage(m_key_bindings, std::move(tickers), additional_tags);
  task_keys_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*task_keys_page, tr("Task Keys"));
  auto cancel_keys_page =
    new CancelKeyBindingsForm(m_key_bindings->get_cancel_key_bindings());
  cancel_keys_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*cancel_keys_page, tr("Cancel Keys"));
  auto interactions_page = new InteractionsPage(m_key_bindings);
  interactions_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*interactions_page, tr("Interactions"));
  auto buttons_body = new QWidget();
  auto buttons_body_layout = make_hbox_layout(buttons_body);
  buttons_body_layout->setSpacing(scale_width(8));
  buttons_body_layout->addStretch(1);
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedWidth(scale_width(100));
  cancel_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_cancel, this));
  buttons_body_layout->addWidget(cancel_button);
  auto done_button = make_label_button(tr("Done"));
  done_button->setFixedWidth(scale_width(100));
  done_button->connect_click_signal(
    std::bind_front(&KeyBindingsWindow::on_done, this));
  buttons_body_layout->addWidget(done_button);
  auto buttons = new Box(buttons_body);
  update_style(*buttons, [] (auto& style) {
    style.get(Any()).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(8))).
      set(PaddingTop(scale_height(29)));
  });
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(navigation_view);
  body_layout->addWidget(buttons);
  set_body(body);
  resize(scale(928, 640));
}

void KeyBindingsWindow::closeEvent(QCloseEvent* event) {
  if(!m_is_committed) {
    copy(m_snapshot, *m_key_bindings);
  }
  Window::closeEvent(event);
}

void KeyBindingsWindow::on_cancel() {
  close();
}

void KeyBindingsWindow::on_done() {
  m_is_committed = true;
  close();
}
