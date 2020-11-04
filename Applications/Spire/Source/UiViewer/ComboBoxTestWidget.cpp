#include "Spire/UiViewer/ComboBoxTestWidget.hpp"
#include <QGridLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

ComboBoxTestWidget::ComboBoxTestWidget(ComboBoxAdapter* combo_box,
    QWidget* parent)
    : QWidget(parent) {
  auto container_widget = new QWidget(this);
  auto layout = new QGridLayout(container_widget);
  combo_box->setFixedSize(scale(100, 26));
  combo_box->layout()->itemAt(0)->widget()->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box, 0, 0);
  auto status_label = new QLabel(this);
  status_label->setMinimumWidth(scale_width(100));
  m_selection_connection = combo_box->connect_selected_signal(
    [=] (const auto& value) {
      status_label->setText(m_item_delegate.displayText(value));
    });
  layout->addWidget(status_label, 0, 1);
}
