#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
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

KeyBindingsWindow::KeyBindingsWindow(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<SecurityInfoQueryModel> securities,
    const AdditionalTagDatabase& additional_tags, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  auto navigation_view = new NavigationView();
  navigation_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto task_keys_page =
    new TaskKeysPage(m_key_bindings, std::move(securities), additional_tags);
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

void KeyBindingsWindow::on_cancel() {
  close();
}

void KeyBindingsWindow::on_done() {
  close();
}
