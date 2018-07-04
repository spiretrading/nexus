#include "spire/ui/properties_window_buttons_widget.hpp"
#include <QGridLayout>
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace spire;

properties_window_buttons_widget::properties_window_buttons_widget(
    QWidget* parent)
    : QWidget(parent) {
  auto main_layout = new QHBoxLayout(this);
  main_layout->setContentsMargins(0, scale_height(10), 0, scale_height(10));
  main_layout->setSpacing(0);
  auto left_widget = new QWidget(this);
  left_widget->setFixedSize(scale(208, 60));
  auto left_layout = new QGridLayout(left_widget);
  left_layout->setContentsMargins({});
  main_layout->addWidget(left_widget);
  main_layout->addStretch(1);
  auto right_widget = new QWidget(this);
  right_widget->setFixedSize(scale(208, 60));
  auto right_layout = new QGridLayout(right_widget);
  right_layout->setContentsMargins({});
  main_layout->addWidget(right_widget);
}
