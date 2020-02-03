#include "Spire/Ui/RangeInputWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/RangeInputSlider.hpp"
#include "Spire/Ui/ScalarWidget.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

RangeInputWidget::RangeInputWidget(std::shared_ptr<RangeInputModel> model,
    Scalar::Type type, ScalarWidget* min_widget,
    ScalarWidget* max_widget, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_type(type),
      m_min_widget(min_widget),
      m_max_widget(max_widget) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_min_widget->connect_change_signal([=] (auto value) {
    on_min_edited(value);
  });
  layout->addWidget(m_min_widget);
  m_slider = new RangeInputSlider(m_model->get_minimum_value(),
    m_model->get_maximum_value(), this);
  m_slider->connect_min_changed_signal([=] (auto value) {
    on_min_handle_moved(value); });
  m_slider->connect_max_changed_signal([=] (auto value) {
    on_max_handle_moved(value); });
  layout->addWidget(m_slider);
  m_max_widget->connect_change_signal([=] (auto value) {
    on_max_edited(value);
  });
  layout->addWidget(m_max_widget);
  m_min_widget->set_value(m_model->get_minimum_value());
  m_max_widget->set_value(m_model->get_maximum_value());
}

connection RangeInputWidget::connect_min_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  return m_min_changed_signal.connect(slot);
}

connection RangeInputWidget::connect_max_changed_signal(
    const ChangedSignal::slot_type& slot) const {
  return m_max_changed_signal.connect(slot);
}

void RangeInputWidget::resizeEvent(QResizeEvent* event) {
  m_slider->set_histogram(m_model->make_histogram(
    m_slider->width() / 2 - scale_width(24)));
}

void RangeInputWidget::on_min_edited(Scalar value) {
  if(value >= m_model->get_minimum_value() &&
      value <= m_max_widget->get_value()) {
    m_slider->set_min_value(value);
  } else if(value < m_model->get_minimum_value()) {
    m_slider->set_min_value(m_model->get_minimum_value());
  }
  update();
}

void RangeInputWidget::on_max_edited(Scalar value) {
  if(value >= m_min_widget->get_value() && value <=
      m_model->get_maximum_value()) {
    m_slider->set_max_value(value);
  } else if(value > m_model->get_maximum_value()) {
    m_slider->set_max_value(m_model->get_maximum_value());
  }
  update();
}

void RangeInputWidget::on_min_handle_moved(Scalar value) {
  m_min_widget->set_value(value);
}

void RangeInputWidget::on_max_handle_moved(Scalar value) {
  m_max_widget->set_value(value);
}
