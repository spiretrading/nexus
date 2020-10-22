#include "Spire/UiViewer/ScrollAreaTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }
}

ScrollAreaTestWidget::ScrollAreaTestWidget(QWidget* parent)
    : QWidget(parent),
      m_scroll_area(nullptr) {
  m_layout = new QGridLayout(this);
  auto width_label = new QLabel(tr("Width"), this);
  m_layout->addWidget(width_label, 0, 0);
  m_width_input = new TextInputWidget("500", this);
  m_width_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_width_input, 0, 1);
  auto height_label = new QLabel(tr("Height"), this);
  m_layout->addWidget(height_label, 1, 0);
  m_height_input = new TextInputWidget("500", this);
  m_height_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_height_input, 1, 1);
  auto resize_button = make_flat_button(tr("Resize Inner Widget"), this);
  resize_button->setFixedHeight(scale_height(26));
  resize_button->connect_clicked_signal([=] { on_resize_button(); });
  m_layout->addWidget(resize_button, 2, 0, 1, 2);
  m_dynamic_check_box = make_check_box(tr("Dynamic"), this);
  m_dynamic_check_box->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_dynamic_check_box, 3, 0);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedSize(CONTROL_SIZE());
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 3, 1);
  on_reset_button();
}

bool ScrollAreaTestWidget::set_widget_size() {
  auto width_ok = false;
  auto width = m_width_input->text().toInt(&width_ok);
  auto height_ok = false;
  auto height = m_height_input->text().toInt(&height_ok);
  if(width_ok && height_ok) {
    m_widget->setFixedSize(width, height);
    return true;
  }
  return false;
}

void ScrollAreaTestWidget::on_reset_button() {
  delete_later(m_scroll_area);
  m_scroll_area = new ScrollArea(m_dynamic_check_box->isChecked(), this);
  m_layout->addWidget(m_scroll_area, 4, 0, 1, 2);
  m_widget = new QLabel(this);
  m_widget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  m_widget->setStyleSheet(
    "border-image: url(:/Icons/124.png) 0 0 0 0 stretch stretch;");
  m_scroll_area->setWidget(m_widget);
  if(!set_widget_size()) {
    m_widget->setFixedSize(m_scroll_area->size());
  }
}

void ScrollAreaTestWidget::on_resize_button() {
  set_widget_size();
}
