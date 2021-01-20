#include "Spire/Ui/PropertiesWindowButtonsWidget.hpp"
#include <QGridLayout>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace boost::signals2;
using namespace Spire;

PropertiesWindowButtonsWidget::PropertiesWindowButtonsWidget(
    QWidget* parent)
    : QWidget(parent) {
  setFixedHeight(scale_height(78));
  auto main_layout = new QHBoxLayout(this);
  main_layout->setContentsMargins(0, scale_height(10), 0, scale_height(10));
  main_layout->setSpacing(0);
  auto left_widget = new QWidget(this);
  left_widget->setFixedSize(scale(208, 60));
  auto left_layout = new QGridLayout(left_widget);
  left_layout->setContentsMargins({});
  left_layout->setHorizontalSpacing(scale_width(8));
  left_layout->setVerticalSpacing(scale_height(8));
  auto save_as_default_button = new FlatButton(tr("Save As Default"), this);
  connect(save_as_default_button, &FlatButton::clicked, [=] {
    m_save_as_default_signal();
  });
  left_layout->addWidget(save_as_default_button, 0, 0);
  auto load_default_button = new FlatButton(tr("Load Default"), this);
  connect(load_default_button, &FlatButton::clicked, [=] {
    m_load_default_signal();
  });
  left_layout->addWidget(load_default_button, 1, 0);
  auto reset_default_button = new FlatButton(tr("Reset Default"), this);
  connect(reset_default_button, &FlatButton::clicked, [=] {
    m_reset_default_signal();
  });
  left_layout->addWidget(reset_default_button, 1, 1);
  main_layout->addWidget(left_widget);
  main_layout->addStretch(1);
  auto right_widget = new QWidget(this);
  right_widget->setFixedSize(scale(208, 60));
  auto right_layout = new QGridLayout(right_widget);
  right_layout->setContentsMargins({});
  right_layout->setHorizontalSpacing(scale_width(8));
  right_layout->setVerticalSpacing(scale_height(8));
  auto apply_button = new FlatButton(tr("Apply"), this);
  connect(apply_button, &FlatButton::clicked, [=] {
    m_apply_signal();
  });
  right_layout->addWidget(apply_button, 0, 0);
  setTabOrder(reset_default_button, apply_button);
  auto apply_to_all_button = new FlatButton(tr("Apply To All"), this);
  connect(apply_to_all_button, &FlatButton::clicked, [=] {
    m_apply_to_all_signal();
  });
  right_layout->addWidget(apply_to_all_button, 0, 1);
  auto ok_button = new FlatButton(tr("OK"), this);
  connect(ok_button, &FlatButton::clicked, [=] {
    m_ok_signal();
  });
  right_layout->addWidget(ok_button, 1, 0);
  auto cancel_button = new FlatButton(tr("Cancel"), this);
  connect(cancel_button, &FlatButton::clicked, [=] {
    m_cancel_signal();
  });
  right_layout->addWidget(cancel_button, 1, 1);
  main_layout->addWidget(right_widget);
}

connection PropertiesWindowButtonsWidget::connect_save_as_default_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_save_as_default_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_load_default_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_load_default_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_reset_default_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_reset_default_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_apply_to_all_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_apply_to_all_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_cancel_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_apply_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection PropertiesWindowButtonsWidget::connect_ok_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_ok_signal.connect(slot);
}
