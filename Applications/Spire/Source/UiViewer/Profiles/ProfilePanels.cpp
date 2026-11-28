#include "Spire/UiViewer/Profiles/ProfilePanels.hpp"
#include <QGridLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;

FilterPanel* Spire::make_filter_panel() {
  auto component = new QWidget();
  auto component_layout = new QGridLayout(component);
  component_layout->setSpacing(scale_width(5));
  component_layout->setContentsMargins({});
  auto min_label = make_label("Min");
  min_label->setMinimumWidth(scale_width(40));
  component_layout->addWidget(min_label, 0, 0);
  auto min_text = new TextBox();
  min_text->setMinimumWidth(scale_width(120));
  component_layout->addWidget(min_text, 0, 1);
  auto max_label = make_label("Max");
  max_label->setMinimumWidth(scale_width(40));
  component_layout->addWidget(max_label, 0, 2);
  auto max_text = new TextBox();
  max_text->setMinimumWidth(scale_width(120));
  component_layout->addWidget(max_text, 0, 3);
  return new FilterPanel(*component);
}

QWidget* Spire::create_panel_body(const QString& button_name) {
  auto body = new QWidget();
  body->setFixedSize(scale(200, 200));
  auto container_layout = new QVBoxLayout(body);
  container_layout->setSpacing(0);
  container_layout->setContentsMargins(scale_width(1),
    scale_height(1), scale_width(1), scale_height(1));
  auto create_button = make_label_button(button_name, body);
  container_layout->addWidget(create_button);
  auto close_button = make_label_button("Close", body);
  close_button->connect_click_signal([=] { body->window()->close(); });
  container_layout->addWidget(close_button);
  return body;
}
