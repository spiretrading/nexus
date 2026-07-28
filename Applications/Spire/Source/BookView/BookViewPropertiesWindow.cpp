#include "Spire/BookView/BookViewPropertiesWindow.hpp"
#include "Spire/BookView/BookViewHighlightPropertiesPage.hpp"
#include "Spire/BookView/BookViewInteractionPropertiesPage.hpp"
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/Button.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

BookViewPropertiesWindow::BookViewPropertiesWindow(
    std::shared_ptr<BookViewPropertiesModel> properties,
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<TickerModel> ticker, QWidget* parent)
    : Window(parent),
      m_properties(std::move(properties)),
      m_ticker(std::move(ticker)),
      m_is_submitted(false) {
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
    new BookViewInteractionPropertiesPage(std::move(key_bindings), m_ticker);
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
  m_ticker_connection = m_ticker->connect_update_signal(
    std::bind_front(&BookViewPropertiesWindow::on_ticker_update, this));
  on_ticker_update(m_ticker->get());
}

connection BookViewPropertiesWindow::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection BookViewPropertiesWindow::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void BookViewPropertiesWindow::closeEvent(QCloseEvent* event) {
  if(m_is_submitted) {
    m_submit_signal();
  } else {
    m_cancel_signal();
  }
  m_is_submitted = false;
  Window::closeEvent(event);
}

void BookViewPropertiesWindow::on_cancel_button_click() {
  close();
}

void BookViewPropertiesWindow::on_done_button_click() {
  m_is_submitted = true;
  close();
}

void BookViewPropertiesWindow::on_level_update(
    const BookViewLevelProperties& properties) {
  update_style(*m_highlights_page, [&] (auto& style) {
    style.get(Any() > is_a<HighlightBox>() > is_a<TextBox>()).
      set(Font(properties.m_font));
  });
}

void BookViewPropertiesWindow::on_ticker_update(const Ticker& ticker) {
  m_navigation_view->set_enabled(
    m_navigation_view->get_count() - 1, static_cast<bool>(ticker));
}
