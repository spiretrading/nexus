#include "Spire/BookView/BookViewPropertiesDialog.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/BookView/BookViewHighlightPropertiesWidget.hpp"
#include "Spire/BookView/BookViewLevelPropertiesWidget.hpp"
#include "Spire/KeyBindings/InteractionsPropertiesWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/PropertiesWindowButtonsWidget.hpp"
#include "Spire/Ui/TabWidget.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewPropertiesDialog::BookViewPropertiesDialog(
    const BookViewProperties& properties, const Security& security,
    QWidget* parent)
    : Dialog(parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint
    & ~Qt::WindowMaximizeButtonHint);
  setWindowModality(Qt::WindowModal);
  set_fixed_body_size(scale(482, 394));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5;");
  setWindowTitle(tr("Properties"));
  set_svg_icon(":/Icons/bookview-black.svg", ":/Icons/bookview-grey.svg");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
  layout->setSpacing(0);
  m_tab_widget = new TabWidget(this);
  m_levels_tab_widget = new BookViewLevelPropertiesWidget(properties,
    m_tab_widget);
  m_tab_widget->addTab(m_levels_tab_widget, tr("Price Levels"));
  m_highlights_tab_widget = new BookViewHighlightPropertiesWidget(
    properties, m_tab_widget);
  m_tab_widget->addTab(m_highlights_tab_widget, tr("Highlights"));
  if(security != Security()) {
    auto interactions_tab_widget = new InteractionsPropertiesWidget(
      m_tab_widget);
    m_tab_widget->addTab(interactions_tab_widget, tr("Interactions"));
  }
  layout->addWidget(m_tab_widget);
  auto button_group_widget = new PropertiesWindowButtonsWidget(this);
  layout->addWidget(button_group_widget);
  button_group_widget->connect_apply_signal([=] { m_apply_signal(); });
  button_group_widget->connect_apply_to_all_signal(
    [=] { m_apply_all_signal(); });
  button_group_widget->connect_cancel_signal([=] { reject(); });
  button_group_widget->connect_ok_signal([=] { accept(); });
  button_group_widget->connect_save_as_default_signal(
    [=] { m_save_default_signal(); });
  Window::layout()->addWidget(body);
}

BookViewProperties BookViewPropertiesDialog::get_properties() const {
  BookViewProperties properties;
  m_levels_tab_widget->apply(properties);
  m_highlights_tab_widget->apply(properties);
  return properties;
}

connection BookViewPropertiesDialog::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection BookViewPropertiesDialog::connect_apply_all_signal(
    const ApplyAllSignal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection BookViewPropertiesDialog::connect_save_default_signal(
    const SaveDefaultSignal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}
