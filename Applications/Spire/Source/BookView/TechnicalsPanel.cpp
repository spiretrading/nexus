#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

TechnicalsPanel::TechnicalsPanel(QWidget* parent)
    : QWidget(parent) {
  setObjectName("technicals_panel");
  setStyleSheet("#technicals_panel { background-color: #F5F5F5; }");
  setFixedHeight(scale_height(36));
  m_layout = new QGridLayout(this);
  m_layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
  m_layout->setSpacing(0);
  m_high_label_widget = new LabeledDataWidget(tr("H"), this);
  m_layout->addWidget(m_high_label_widget, 0, 0);
  m_layout->setColumnStretch(0, 100);
  m_open_label_widget = new LabeledDataWidget(tr("O"), this);
  m_layout->addWidget(m_open_label_widget, 0, 1);
  m_layout->setColumnStretch(1, 100);
  m_defaults_label_widget = new LabeledDataWidget(tr("D"),
    QString("100%1%2").arg(tr("x")).arg("100"), this);
  m_layout->addWidget(m_defaults_label_widget, 0, 2);
  m_layout->setColumnStretch(2, 1);
  m_low_label_widget = new LabeledDataWidget(tr("L"), this);
  m_layout->addWidget(m_low_label_widget, 1, 0);
  m_close_label_widget = new LabeledDataWidget(tr("C"), this);
  m_layout->addWidget(m_close_label_widget, 1, 1);
  m_volume_label_widget = new LabeledDataWidget(tr("V"), this);
  m_layout->addWidget(m_volume_label_widget, 1, 2);
  m_item_delegate = new CustomVariantItemDelegate(this);
}

void TechnicalsPanel::set_model(std::shared_ptr<BookViewModel> model) {
  m_model = model;
  if(m_model->get_close().is_initialized()) {
    m_close_label_widget->set_data_text(m_item_delegate->displayText(
      QVariant::fromValue(*m_model->get_close()), QLocale()));
  } else {
    m_close_label_widget->set_data_text(tr("N/A"));
  }
  if(m_model->get_high().is_initialized()) {
    m_high_label_widget->set_data_text(m_item_delegate->displayText(
      QVariant::fromValue(*m_model->get_high()), QLocale()));
  } else {
    m_high_label_widget->set_data_text(tr("N/A"));
  }
  if(m_model->get_low().is_initialized()) {
    m_low_label_widget->set_data_text(m_item_delegate->displayText(
      QVariant::fromValue(*m_model->get_low()), QLocale()));
  } else {
    m_low_label_widget->set_data_text(tr("N/A"));
  }
  if(m_model->get_open().is_initialized()) {
    m_open_label_widget->set_data_text(m_item_delegate->displayText(
      QVariant::fromValue(*m_model->get_open()), QLocale()));
  } else {
    m_open_label_widget->set_data_text(tr("N/A"));
  }
  m_volume_label_widget->set_data_text(m_item_delegate->displayText(
    QVariant::fromValue(m_model->get_volume()), QLocale()));
  m_close_connection = m_model->connect_close_slot(
    [=] (const auto& c) { on_close_signal(c); });
  m_high_connection = m_model->connect_high_slot(
    [=] (const auto& h) { on_high_signal(h); });
  m_low_connection = m_model->connect_low_slot(
    [=] (const auto& l) { on_low_signal(l); });
  m_open_connection = m_model->connect_open_slot(
    [=] (const auto& o) { on_open_signal(o); });
  m_volume_connection = m_model->connect_volume_slot(
    [=] (const auto& v) { on_volume_signal(v); });
}

void TechnicalsPanel::reset_model() {
  m_model.reset();
  m_close_label_widget->set_data_text("");
  m_high_label_widget->set_data_text("");
  m_low_label_widget->set_data_text("");
  m_open_label_widget->set_data_text("");
  m_volume_label_widget->set_data_text("");
}

void TechnicalsPanel::resizeEvent(QResizeEvent* event) {
  auto header_width = width();
  if(header_width <= scale_width(255)) {
    set_labeled_data_short_form_text();
  } else if(header_width >= scale_width(256) &&
      header_width <= scale_width(411)) {
    set_labeled_data_long_form_text();
  } else if(header_width >= scale_width(412) &&
      header_width <= scale_width(503)) {
    set_labeled_data_short_form_text();
  } else if(header_width >= scale_width(504)) {
    set_labeled_data_long_form_text();
  }
  if(header_width <= scale_width(412)) {
    if(m_layout->itemAtPosition(0, 3) != nullptr) {
      setFixedHeight(scale_height(36));
      m_layout->addWidget(m_open_label_widget, 0, 1);
      m_layout->addWidget(m_defaults_label_widget, 0, 2);
      m_layout->addWidget(m_low_label_widget, 1, 0);
      m_layout->addWidget(m_close_label_widget, 1, 1);
      m_layout->addWidget(m_volume_label_widget, 1, 2);
      m_layout->setColumnStretch(2, 1);
      m_layout->setColumnStretch(3, 0);
      m_layout->setColumnStretch(4, 0);
      m_layout->setColumnStretch(5, 0);
    }
  } else {
    if(m_layout->itemAtPosition(1, 0) != nullptr) {
      setFixedHeight(scale_height(20));
      m_layout->addWidget(m_low_label_widget, 0, 1);
      m_layout->addWidget(m_open_label_widget, 0, 2);
      m_layout->addWidget(m_close_label_widget, 0, 3);
      m_layout->addWidget(m_volume_label_widget, 0, 4);
      m_layout->addWidget(m_defaults_label_widget, 0, 5);
      m_layout->setColumnStretch(2, 100);
      m_layout->setColumnStretch(3, 100);
      m_layout->setColumnStretch(4, 100);
      m_layout->setColumnStretch(5, 1);
    }
  }
}

void TechnicalsPanel::set_labeled_data_long_form_text() {
  m_high_label_widget->set_label_text(tr("High"));
  m_high_label_widget->set_label_width(scale_width(24));
  m_open_label_widget->set_label_text(tr("Open"));
  m_open_label_widget->set_label_width(scale_width(28));
  m_defaults_label_widget->set_label_text(tr("Def"));
  m_defaults_label_widget->set_label_width(scale_width(20));
  m_low_label_widget->set_label_text(tr("Low"));
  m_low_label_widget->set_label_width(scale_width(24));
  m_close_label_widget->set_label_text(tr("Close"));
  m_close_label_widget->set_label_width(scale_width(28));
  m_volume_label_widget->set_label_text(tr("Vol"));
  m_volume_label_widget->set_label_width(scale_width(20));
}

void TechnicalsPanel::set_labeled_data_short_form_text() {
  auto width = scale_width(10);
  m_high_label_widget->set_label_text(tr("H"));
  m_high_label_widget->set_label_width(width);
  m_open_label_widget->set_label_text(tr("O"));
  m_open_label_widget->set_label_width(width);
  m_defaults_label_widget->set_label_text(tr("D"));
  m_defaults_label_widget->set_label_width(width);
  m_low_label_widget->set_label_text(tr("L"));
  m_low_label_widget->set_label_width(width);
  m_close_label_widget->set_label_text(tr("C"));
  m_close_label_widget->set_label_width(width);
  m_volume_label_widget->set_label_text(tr("V"));
  m_volume_label_widget->set_label_width(width);
}

void TechnicalsPanel::on_close_signal(const Money& close) {
  m_close_label_widget->set_data_text(
    m_item_delegate->displayText(QVariant::fromValue(close), QLocale()));
}

void TechnicalsPanel::on_high_signal(const Money& high) {
  m_high_label_widget->set_data_text(
    m_item_delegate->displayText(QVariant::fromValue(high), QLocale()));
}

void TechnicalsPanel::on_low_signal(const Money& low) {
  m_low_label_widget->set_data_text(
    m_item_delegate->displayText(QVariant::fromValue(low), QLocale()));
}

void TechnicalsPanel::on_open_signal(const Money& open) {
  m_open_label_widget->set_data_text(
    m_item_delegate->displayText(QVariant::fromValue(open), QLocale()));
}

void TechnicalsPanel::on_volume_signal(const Quantity& volume) {
  m_volume_label_widget->set_data_text(
    m_item_delegate->displayText(QVariant::fromValue(volume), QLocale()));
}
