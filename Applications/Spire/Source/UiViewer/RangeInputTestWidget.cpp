#include "Spire/UiViewer/RangeInputTestWidget.hpp"
#include <algorithm>
#include <random>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/LocalRangeInputModel.hpp"
#include "Spire/Ui/QuantitySpinBox.hpp"
#include "Spire/Ui/ScalarWidget.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto generate_data(double min, double max, int count) {
    static auto device = std::random_device();
    static auto engine = std::mt19937(device());
    auto distribution = std::uniform_real_distribution(min, max);
    auto generator = [&] { return Scalar(Quantity(distribution(engine))); };
    auto data = std::vector<Scalar>();
    data.reserve(count);
    std::generate_n(std::back_inserter(data), count, generator);
    return data;
  }

  auto to_quantity(Scalar value) {
    return static_cast<Quantity>(value);
  }
}

RangeInputTestWidget::RangeInputTestWidget(QWidget* parent)
    : QWidget(parent),
      m_range_input(nullptr) {
  auto container = new QWidget(this);
  container->setMinimumWidth(scale_width(500));
  m_layout = new QGridLayout(container);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 1, 0, 1, 2);
  auto min_label = new QLabel(tr("Minimum"), this);
  m_layout->addWidget(min_label, 2, 0);
  m_min_input = new TextInputWidget("0", this);
  m_min_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_min_input, 2, 1);
  auto max_label = new QLabel(tr("Maximum"), this);
  m_layout->addWidget(max_label, 3, 0);
  m_max_input = new TextInputWidget("100", this);
  m_max_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_max_input, 3, 1);
  auto count_label = new QLabel(tr("Data Count"), this);
  m_layout->addWidget(count_label, 4, 0);
  m_count_input = new TextInputWidget("1000", this);
  m_count_input->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_count_input, 4, 1);
  m_reset_button = make_flat_button(tr("Reset"), this);
  m_reset_button->setFixedHeight(scale_height(26));
  m_reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(m_reset_button, 5, 0, 1, 2);
  on_reset_button();
}

void RangeInputTestWidget::on_reset_button() {
  delete_later(m_range_input);
  auto min_ok = false;
  auto min = m_min_input->text().toDouble(&min_ok);
  auto max_ok = false;
  auto max = m_max_input->text().toDouble(&max_ok);
  auto count_ok = false;
  auto count = m_count_input->text().toInt(&count_ok);
  if(min_ok && max_ok && count_ok) {
    auto range_model = std::make_shared<LocalRangeInputModel>(
      std::move(generate_data(min, max, count)));
    auto min_input_model = std::make_shared<QuantitySpinBoxModel>(
      to_quantity(range_model->get_minimum_value()),
      to_quantity(range_model->get_minimum_value()),
      to_quantity(range_model->get_maximum_value()), Quantity(1));
    auto min_input = new QuantitySpinBox(min_input_model, this);
    min_input->setFixedSize(CONTROL_SIZE());
    auto min_widget = new ScalarWidget(min_input,
      &QuantitySpinBox::connect_change_signal, &QuantitySpinBox::set_value);
    auto max_input_model = std::make_shared<QuantitySpinBoxModel>(
      to_quantity(range_model->get_maximum_value()),
      to_quantity(range_model->get_minimum_value()),
      to_quantity(range_model->get_maximum_value()), Quantity(1));
    auto max_input = new QuantitySpinBox(max_input_model, this);
    max_input->setFixedSize(CONTROL_SIZE());
    auto max_widget = new ScalarWidget(max_input,
      &QuantitySpinBox::connect_change_signal, &QuantitySpinBox::set_value);
    m_range_input = new RangeInputWidget(range_model, min_widget, max_widget,
      Scalar(Quantity(1)), this);
    m_layout->addWidget(m_range_input, 0, 0, 1, 2);
    setTabOrder(min_input, max_input);
    setTabOrder(max_input, m_min_input);
    setTabOrder(m_min_input, m_max_input);
    setTabOrder(m_max_input, m_count_input);
    setTabOrder(m_count_input, m_reset_button);
  } else {
    m_status_label->setText(tr("Invalid Input"));
  }
}
