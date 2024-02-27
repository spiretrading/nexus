#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
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
  auto make_interactions_page() {
    auto interactions_form = new InteractionsKeyBindingsForm(Region::Global(),
      std::make_shared<InteractionsKeyBindingsModel>());
    interactions_form->setMinimumWidth(scale_width(384));
    interactions_form->setMaximumWidth(scale_width(480));
    auto scroll_box_body = new QWidget();
    scroll_box_body->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    auto center_layout = make_vbox_layout();
    center_layout->addWidget(interactions_form);
    center_layout->addStretch(1);
    auto scroll_box_body_layout = make_hbox_layout(scroll_box_body);
    scroll_box_body_layout->addStretch(0);
    scroll_box_body_layout->addLayout(center_layout, 1);
    scroll_box_body_layout->addStretch(0);
    auto scroll_box = new ScrollBox(scroll_box_body);
    scroll_box->setFocusPolicy(Qt::NoFocus);
    scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*scroll_box, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
    });
    auto interactions_page = new QWidget();
    enclose(*interactions_page, *scroll_box);
    interactions_page->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    return interactions_page;
  }
}

KeyBindingsWindow::KeyBindingsWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle(tr("Key Bindings"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  auto navigation_view = new NavigationView();
  navigation_view->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  auto task_keys_page = new QWidget();
  task_keys_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  navigation_view->add_tab(*task_keys_page, tr("Task Keys"));
  auto cancel_keys_page =
    new CancelKeyBindingsForm(std::make_shared<CancelKeyBindingsModel>());
  cancel_keys_page->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  navigation_view->add_tab(*cancel_keys_page, tr("Cancel Keys"));
  navigation_view->add_tab(*make_interactions_page(), tr("Interactions"));
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
