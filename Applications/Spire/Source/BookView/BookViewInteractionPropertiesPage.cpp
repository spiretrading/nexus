#include "Spire/BookView/BookViewInteractionPropertiesPage.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

BookViewInteractionPropertiesPage::BookViewInteractionPropertiesPage(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<SecurityModel> security, QWidget* parent)
    : m_key_bindings(std::move(key_bindings)) {
  m_current_region = make_transform_value_model(security,
    [=] (const auto& security) {
      return Region(security);
    });
  auto key_bindings_form =
    new InteractionsKeyBindingsForm(m_key_bindings, m_current_region);
  key_bindings_form->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto reset_container_body = new QWidget();
  auto reset_container_layout = make_hbox_layout(reset_container_body);
  reset_container_layout->addStretch(1);
  auto reset_button = make_label_button(tr("Reset"));
  reset_button->setFixedWidth(scale_width(120));
  reset_container_layout->addWidget(reset_button);
  auto reset_container = new Box(reset_container_body);
  update_style(*reset_container, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(8)));
  });
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(key_bindings_form);
  layout->addWidget(reset_container);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  enclose(*this, *box);
  reset_button->connect_click_signal(
    std::bind_front(&BookViewInteractionPropertiesPage::on_reset_click, this));
}

void BookViewInteractionPropertiesPage::on_reset_click() {
  m_key_bindings->get_interactions_key_bindings(
    m_current_region->get())->reset();
}
