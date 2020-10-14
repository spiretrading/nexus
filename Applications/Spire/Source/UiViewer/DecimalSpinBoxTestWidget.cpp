#include "Spire/UiViewer/DecimalSpinBoxTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace boost;
using namespace Spire;

namespace {
  const auto NO_MODIFIER = QString(QObject::tr("NoModifier"));
  const auto SHIFT_MODIFIER = QString(QObject::tr("Shift"));
  const auto CTRL_MODIFIER = QString(QObject::tr("Control"));
  const auto CTRL_SHIFT_MODIFIER = QString(QObject::tr("Shift + Control"));

  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  Qt::KeyboardModifiers get_modifier(const QString& text) {
    if(text == NO_MODIFIER) {
      return Qt::NoModifier;
    } else if(text == SHIFT_MODIFIER) {
      return Qt::ShiftModifier;
    } else if(text == CTRL_MODIFIER) {
      return Qt::ControlModifier;
    } else if(text == CTRL_SHIFT_MODIFIER) {
      return Qt::ControlModifier | Qt::ShiftModifier;
    }
    return Qt::NoModifier;
  }
  
  optional<double> get_value(const QString& text) {
    auto ok = false;
    auto value = text.toDouble(&ok);
    if(ok) {
      return value;
    }
    return none;
  }
}

DecimalSpinBoxTestWidget::DecimalSpinBoxTestWidget(QWidget* parent)
    : QWidget(parent),
      m_spin_box(nullptr) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_value_label = new QLabel(this);
  m_layout->addWidget(m_value_label, 0, 1, Qt::AlignCenter);
  auto initial_label = new QLabel(tr("Initial"), this);
  m_layout->addWidget(initial_label, 1, 0);
  m_initial_input = new TextInputWidget("0", this);
  m_initial_input->setFixedSize(CONTROL_SIZE());
  connect(m_initial_input, &TextInputWidget::editingFinished, this,
    &DecimalSpinBoxTestWidget::on_initial_set);
  m_layout->addWidget(m_initial_input, 1, 1);
  auto min_label = new QLabel(tr("Minimum"), this);
  m_layout->addWidget(min_label, 2, 0);
  m_min_input = new TextInputWidget("-1000", this);
  m_min_input->setFixedSize(CONTROL_SIZE());
  connect(m_min_input, &TextInputWidget::editingFinished, this,
    &DecimalSpinBoxTestWidget::on_min_set);
  m_layout->addWidget(m_min_input, 2, 1);
  auto max_label = new QLabel(tr("Maximum"), this);
  m_layout->addWidget(max_label, 3, 0);
  m_max_input = new TextInputWidget("1000", this);
  m_max_input->setFixedSize(CONTROL_SIZE());
  connect(m_max_input, &TextInputWidget::editingFinished, this,
    &DecimalSpinBoxTestWidget::on_max_set);
  m_layout->addWidget(m_max_input, 3, 1);
  m_modifier_menu = new StaticDropDownMenu({NO_MODIFIER, SHIFT_MODIFIER,
    CTRL_MODIFIER, CTRL_SHIFT_MODIFIER}, this);
  m_modifier_menu->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_modifier_menu, 4, 0);
  m_increment_input = new TextInputWidget("1", this);
  m_increment_input->setFixedSize(CONTROL_SIZE());
  connect(m_increment_input, &TextInputWidget::editingFinished, this,
    &DecimalSpinBoxTestWidget::on_increment_set);
  m_layout->addWidget(m_increment_input, 4, 1);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedHeight(scale_height(26));
  reset_button->connect_clicked_signal([=] { reset_spin_box(); });
  m_layout->addWidget(reset_button, 5, 0, 1, 2);
  auto no_label = new QLabel(NO_MODIFIER, this);
  m_layout->addWidget(no_label, 6, 0);
  m_no_increment_label = new QLabel(this);
  m_layout->addWidget(m_no_increment_label, 6, 1);
  auto shift_label = new QLabel(SHIFT_MODIFIER, this);
  m_layout->addWidget(shift_label, 7, 0);
  m_shift_increment_label = new QLabel(this);
  m_layout->addWidget(m_shift_increment_label, 7, 1);
  auto ctrl_label = new QLabel(CTRL_MODIFIER, this);
  m_layout->addWidget(ctrl_label, 8, 0);
  m_ctrl_increment_label = new QLabel(this);
  m_layout->addWidget(m_ctrl_increment_label, 8, 1);
  auto shift_ctrl_label = new QLabel(CTRL_SHIFT_MODIFIER, this);
  m_layout->addWidget(shift_ctrl_label, 9, 0);
  m_ctrl_shift_increment_label = new QLabel(this);
  m_layout->addWidget(m_ctrl_shift_increment_label, 9, 1);
  reset_spin_box();
}

void DecimalSpinBoxTestWidget::reset_spin_box() {
  auto initial = get_value(m_initial_input->text());
  auto min = get_value(m_min_input->text());
  auto max = get_value(m_max_input->text());
  auto increment = get_value(m_increment_input->text());
  if(initial && min && max && increment && min < max && min <= initial &&
      initial <= max) {
    m_model = std::make_shared<DecimalSpinBoxModel>(*initial, *min, *max,
      *increment);
    delete_later(m_spin_box);
    m_spin_box = new DecimalSpinBox(m_model, this);
    m_spin_box->setFixedSize(CONTROL_SIZE());
    m_spin_box->connect_change_signal([=] (auto value) {
      m_value_label->setText(QString::number(value));
    });
    m_layout->addWidget(m_spin_box, 0, 0);
    update_increment_labels();
    m_spin_box->setFocus();
  } else {
    m_value_label->setText(tr("Failed"));
  }
}

void DecimalSpinBoxTestWidget::update_increment_labels() {
  m_no_increment_label->setText(QString::number(m_model->get_increment(
    Qt::NoModifier)));
  m_shift_increment_label->setText(QString::number(m_model->get_increment(
    Qt::ShiftModifier)));
  m_ctrl_increment_label->setText(QString::number(m_model->get_increment(
    Qt::ControlModifier)));
  m_ctrl_shift_increment_label->setText(QString::number(m_model->get_increment(
    Qt::ControlModifier | Qt::ShiftModifier)));
}

void DecimalSpinBoxTestWidget::on_initial_set() {
  if(auto initial = get_value(m_initial_input->text()); initial) {
    m_model->set_initial(*initial);
    m_value_label->setText("Initial set");
  }
}

void DecimalSpinBoxTestWidget::on_min_set() {
  if(auto min = get_value(m_min_input->text()); min) {
    m_model->set_minimum(*min);
    m_value_label->setText("Min set");
  }
}

void DecimalSpinBoxTestWidget::on_max_set() {
  if(auto max = get_value(m_max_input->text()); max) {
    m_model->set_maximum(*max);
    m_value_label->setText("Max set");
  }
}

void DecimalSpinBoxTestWidget::on_increment_set() {
  if(auto modifier = get_value(m_increment_input->text()); modifier) {
    m_model->set_increment(get_modifier(
      m_modifier_menu->get_current_item().value<QString>()), *modifier);
    m_value_label->setText("Increment set");
    update_increment_labels();
  }
}
