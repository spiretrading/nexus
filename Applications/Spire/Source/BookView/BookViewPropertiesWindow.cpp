#include "Spire/BookView/BookViewPropertiesWindow.hpp"
#include <ranges>
#include "Spire/BookView/BookViewHighlightPropertiesPage.hpp"
#include "Spire/BookView/BookViewInteractionPropertiesPage.hpp"
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/Button.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void copy_interactions(const InteractionsKeyBindingsModel& from,
      InteractionsKeyBindingsModel& to) {
    to.get_default_quantity()->set(from.get_default_quantity()->get());
    for(auto i :
        std::views::iota(0, InteractionsKeyBindingsModel::MODIFIER_COUNT)) {
      auto modifier = to_modifier(i);
      to.get_quantity_increment(modifier)->set(
        from.get_quantity_increment(modifier)->get());
      to.get_price_increment(modifier)->set(
        from.get_price_increment(modifier)->get());
    }
    to.is_cancel_on_fill()->set(from.is_cancel_on_fill()->get());
  }
}

BookViewPropertiesWindow::BookViewPropertiesWindow(
    std::shared_ptr<BookViewPropertiesModel> properties,
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<SecurityModel> security, QWidget* parent)
    : Window(parent),
      m_properties(std::move(properties)),
      m_key_bindings(std::move(key_bindings)),
      m_security(std::move(security)),
      m_initial_properties(m_properties->get()) {
  set_svg_icon(":/Icons/bookview.svg");
  setWindowTitle(tr("Book View Properties"));
  setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
  m_navigation_view = new NavigationView();
  m_navigation_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto levels_page = new BookViewLevelPropertiesPage(make_field_value_model(
    m_properties, &BookViewProperties::m_level_properties));
  levels_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_level_connection = levels_page->get_current()->connect_update_signal(
    std::bind_front(&BookViewPropertiesWindow::on_level_update, this));
  m_navigation_view->add_tab(*levels_page, tr("Levels"));
  m_highlights_page = new BookViewHighlightPropertiesPage(
    make_field_value_model(
      m_properties, &BookViewProperties::m_highlight_properties));
  m_highlights_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_navigation_view->add_tab(*m_highlights_page, tr("Highlights"));
  auto interactions_page =
    new BookViewInteractionPropertiesPage(m_key_bindings, m_security);
  interactions_page->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_navigation_view->add_tab(*interactions_page, tr("Interactions"));
  auto actions_body = new QWidget();
  auto actions_body_layout = make_hbox_layout(actions_body);
  actions_body_layout->addStretch(1);
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setFixedWidth(scale_width(100));
  cancel_button->connect_click_signal(
    std::bind_front(&BookViewPropertiesWindow::on_cancel_button_click, this));
  actions_body_layout->addWidget(cancel_button);
  actions_body_layout->addSpacing(scale_width(8));
  auto done_button = make_label_button(QObject::tr("Done"));
  done_button->setFixedWidth(scale_width(100));
  done_button->connect_click_signal(
    std::bind_front(&BookViewPropertiesWindow::on_done_button_click, this));
  actions_body_layout->addWidget(done_button);
  auto actions_box = new Box(actions_body);
  update_style(*actions_box, [] (auto& style) {
    style.get(Any()).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(padding(scale_width(8)));
  });
  auto body = new QWidget();
  body->setFixedSize(scale(360, 608));
  auto layout = make_vbox_layout(body);
  layout->addWidget(m_navigation_view);
  layout->addWidget(actions_box);
  set_body(body);
  m_security_connection = m_security->connect_update_signal(
    std::bind_front(&BookViewPropertiesWindow::on_security_update, this));
  on_security_update(m_security->get());
}

void BookViewPropertiesWindow::on_cancel_button_click() {
  m_properties->set(m_initial_properties);
  auto& current_interactions =
    m_key_bindings->get_interactions_key_bindings(m_security->get());
  if(current_interactions && current_interactions->is_detached()) {
    if(m_are_interactions_detached) {
      copy_interactions(m_initial_interactions, *current_interactions);
    } else {
      current_interactions->reset();
    }
  }
  close();
}

void BookViewPropertiesWindow::on_done_button_click() {
  m_initial_properties = m_properties->get();
  if(auto& current_interactions =
      m_key_bindings->get_interactions_key_bindings(m_security->get())) {
    copy_interactions(*current_interactions, m_initial_interactions);
    m_are_interactions_detached = current_interactions->is_detached();
  }
  close();
}

void BookViewPropertiesWindow::on_level_update(
    const BookViewLevelProperties& properties) {
  update_style(*m_highlights_page, [&] (auto& style) {
    style.get(Any() > is_a<HighlightBox>() > is_a<TextBox>()).
      set(Font(properties.m_font));
  });
}

void BookViewPropertiesWindow::on_security_update(const Security& security) {
  auto is_empty_security = !security;
  m_navigation_view->set_enabled(m_navigation_view->get_count() - 1, !is_empty_security);
  if(!is_empty_security) {
    if(auto& current_interactions =
      m_key_bindings->get_interactions_key_bindings(security)) {
      m_are_interactions_detached = current_interactions->is_detached();
      copy_interactions(*current_interactions, m_initial_interactions);
    } else {
      m_are_interactions_detached = true;
    }
  }
}
