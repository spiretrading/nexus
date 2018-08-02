#include "spire/book_view/technicals_panel.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

TechnicalsPanel::TechnicalsPanel(QWidget* parent)
    : QWidget(parent) {
  setStyleSheet("background-color: #F5F5F5;");
  setFixedHeight(scale_height(36));
  m_layout = new QGridLayout(this);
  m_layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
  m_layout->setSpacing(0);
  m_high_label_widget = new LabeledDataWidget(tr("H"), tr("0.00"),
    this);
  m_layout->addWidget(m_high_label_widget, 0, 0);
  m_layout->setColumnStretch(0, 100);
  m_open_label_widget = new LabeledDataWidget(tr("O"), tr("0.00"),
    this);
  m_layout->addWidget(m_open_label_widget, 0, 1);
  m_layout->setColumnStretch(1, 100);
  m_defaults_label_widget = new LabeledDataWidget(tr("D"),
    QString("100%1%2").arg(tr("x")).arg("100"), this);
  m_layout->addWidget(m_defaults_label_widget, 0, 2);
  m_layout->setColumnStretch(2, 1);
  m_low_label_widget = new LabeledDataWidget(tr("L"), tr("0.00"),
    this);
  m_layout->addWidget(m_low_label_widget, 1, 0);
  m_close_label_widget = new LabeledDataWidget(tr("C"), tr("0.00"),
    this);
  m_layout->addWidget(m_close_label_widget, 1, 1);
  m_volume_label_widget = new LabeledDataWidget(tr("V"), "0", this);
  m_layout->addWidget(m_volume_label_widget, 1, 2);
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
  m_open_label_widget->set_label_text(tr("Open"));
  m_defaults_label_widget->set_label_text(tr("Def"));
  m_low_label_widget->set_label_text(tr("Low"));
  m_close_label_widget->set_label_text(tr("Close"));
  m_volume_label_widget->set_label_text(tr("Vol"));
}

void TechnicalsPanel::set_labeled_data_short_form_text() {
  m_high_label_widget->set_label_text(tr("H"));
  m_open_label_widget->set_label_text(tr("O"));
  m_defaults_label_widget->set_label_text(tr("D"));
  m_low_label_widget->set_label_text(tr("L"));
  m_close_label_widget->set_label_text(tr("C"));
  m_volume_label_widget->set_label_text(tr("V"));
}
