#include "spire/charting/charting_technicals_panel.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace Spire;

ChartingTechnicalsPanel::ChartingTechnicalsPanel(TechnicalsModel& model)
    : m_model(model) {
  setStyleSheet("background-color: #25212E;");
  setFixedHeight(scale_height(50));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), 0, 0, 0);
  layout->addStretch(8);
  auto price_layout = new QHBoxLayout();
  price_layout->setContentsMargins({});
  layout->addLayout(price_layout);
  m_last_label = new QLabel(tr("N/A"), this);
  m_last_label->setFixedHeight(scale_height(19));
  m_last_label->setStyleSheet(QString(R"(
    color: #FFFFFF;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(16)));
  price_layout->addWidget(m_last_label);
  price_layout->addSpacing(scale_width(4));
  m_change_label = new QLabel(tr("N/A"), this);
  m_change_label->setFixedHeight(scale_height(18));
  m_change_label->setStyleSheet(QString(R"(
    color: #FFFFFF;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;
    qproperty-alignment: AlignBottom;)").arg(scale_height(10)));
  price_layout->addWidget(m_change_label);
  price_layout->addStretch(1);
  layout->addStretch(4);
  auto ohlc_layout = new QHBoxLayout();
  ohlc_layout->setContentsMargins({});
  ohlc_layout->setSpacing(0);
  layout->addLayout(ohlc_layout);
  m_open_text_label = new QLabel(tr("O"), this);
  m_open_text_label->setFixedHeight(scale_height(11));
  auto text_label_stylesheet = QString(R"(
    color: #FFFFFF;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(10));
  m_open_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_open_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_open_value_label = new QLabel(tr("N/A"), this);
  m_open_value_label->setFixedHeight(scale_height(11));
  auto value_label_stylesheet = QString(R"(
    color: #FFFFFF;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(10));
  m_open_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_open_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  m_close_text_label = new QLabel(tr("C"), this);
  m_close_text_label->setFixedHeight(scale_height(11));
  m_close_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_close_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_close_value_label = new QLabel(tr("N/A"), this);
  m_close_value_label->setFixedHeight(scale_height(11));
  m_close_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_close_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  m_high_text_label = new QLabel(tr("H"), this);
  m_high_text_label->setFixedHeight(scale_height(11));
  m_high_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_high_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_high_value_label = new QLabel(tr("N/A"), this);
  m_high_value_label->setFixedHeight(scale_height(11));
  m_high_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_high_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  m_low_text_label = new QLabel(tr("L"), this);
  m_low_text_label->setFixedHeight(scale_height(11));
  m_low_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_low_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_low_value_label = new QLabel(tr("N/A"), this);
  m_low_value_label->setFixedHeight(scale_height(11));
  m_low_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_low_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  m_volume_text_label = new QLabel(tr("V"), this);
  m_volume_text_label->setFixedHeight(scale_height(11));
  m_volume_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_volume_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_volume_value_label = new QLabel(tr("N/A"), this);
  m_volume_value_label->setFixedHeight(scale_height(11));
  m_volume_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_volume_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  ohlc_layout->addStretch(1);
  layout->addStretch(8);
}

void ChartingTechnicalsPanel::resizeEvent(QResizeEvent* event) {
  if(width() < scale_width(500)) {
    m_open_text_label->setText(tr("O"));
    m_close_text_label->setText(tr("C"));
    m_high_text_label->setText(tr("H"));
    m_low_text_label->setText(tr("L"));
    m_volume_text_label->setText(tr("V"));
  } else {
    m_open_text_label->setText(tr("Open"));
    m_close_text_label->setText(tr("Close"));
    m_high_text_label->setText(tr("High"));
    m_low_text_label->setText(tr("Low"));
    m_volume_text_label->setText(tr("Vol"));
  }
}
