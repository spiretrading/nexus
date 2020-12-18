#include "Spire/Charting/ChartingTechnicalsPanel.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TechnicalsModel.hpp"

using namespace Nexus;
using namespace Spire;

ChartingTechnicalsPanel::ChartingTechnicalsPanel(TechnicalsModel& model)
    : m_model(&model) {
  m_item_delegate = new CustomVariantItemDelegate(this);
  setObjectName("charting_technials_panel");
  setStyleSheet("#charting_technicals_panel { background-color: #25212E; }");
  setFixedHeight(scale_height(50));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), 0, 0, 0);
  layout->addStretch(8);
  auto price_layout = new QHBoxLayout();
  price_layout->setContentsMargins({});
  price_layout->setSpacing(0);
  layout->addLayout(price_layout);
  m_last_label = new QLabel(tr("N/A"), this);
  m_last_label->setStyleSheet(QString(R"(
    QLabel {
      color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
    })").arg(scale_height(16)));
  m_last_label->setFixedHeight(scale_height(19));
  price_layout->addWidget(m_last_label);
  price_layout->addSpacing(scale_width(4));
  auto price_change_layout = new QVBoxLayout();
  price_change_layout->setContentsMargins(0, scale_height(5), 0,
    scale_height(3));
  m_change_label = new QLabel(this);
  update_change_label();
  m_change_label->setFixedHeight(scale_height(18));
  price_change_layout->addWidget(m_change_label);
  price_layout->addLayout(price_change_layout);
  price_layout->addStretch(1);
  layout->addStretch(4);
  auto ohlc_layout = new QHBoxLayout();
  ohlc_layout->setContentsMargins({});
  ohlc_layout->setSpacing(0);
  layout->addLayout(ohlc_layout);
  m_open_text_label = new QLabel(tr("O"), this);
  m_open_text_label->setFixedHeight(scale_height(11));
  auto text_label_stylesheet = QString(R"(
    QLabel {
      color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(10));
  m_open_text_label->setStyleSheet(text_label_stylesheet);
  ohlc_layout->addWidget(m_open_text_label);
  ohlc_layout->addSpacing(scale_width(3));
  m_open_value_label = new QLabel(tr("N/A"), this);
  m_open_value_label->setFixedHeight(scale_height(11));
  auto value_label_stylesheet = QString(R"(
    QLabel {
      color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
    })").arg(scale_height(10));
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
  m_volume_value_label = new QLabel("0", this);
  m_volume_value_label->setFixedHeight(scale_height(11));
  m_volume_value_label->setStyleSheet(value_label_stylesheet);
  ohlc_layout->addWidget(m_volume_value_label);
  ohlc_layout->addSpacing(scale_width(10));
  ohlc_layout->addStretch(1);
  layout->addStretch(8);
  m_model_load_promise = m_model->load();
  m_model_load_promise.then([=] (auto result) {
    if(m_model->get_last_price().is_initialized()) {
      on_last_price_signal(*(m_model->get_last_price()));
    }
    if(m_model->get_open().is_initialized()) {
      on_open_signal(*(m_model->get_open()));
    }
    if(m_model->get_close().is_initialized()) {
      on_close_signal(*(m_model->get_close()));
    }
    if(m_model->get_high().is_initialized()) {
      on_high_signal(*(m_model->get_high()));
    }
    if(m_model->get_low().is_initialized()) {
      on_low_signal(*(m_model->get_low()));
    }
    on_volume_signal(m_model->get_volume());
  });
  m_last_price_connection = m_model->connect_last_price_slot(
    [=] (Money last) {
      on_last_price_signal(last);
    });
  m_open_connection = m_model->connect_open_slot(
    [=] (Money open) {
      on_open_signal(open);
    });
  m_close_connection = m_model->connect_close_slot(
    [=] (Money close) {
      on_close_signal(close);
    });
  m_high_connection = m_model->connect_high_slot(
    [=] (Money high) {
      on_high_signal(high);
    });
  m_low_connection = m_model->connect_low_slot(
    [=] (Money low) {
      on_low_signal(low);
    });
  m_volume_connection = m_model->connect_volume_slot(
    [=] (Quantity volume) {
      on_volume_signal(volume);
    });
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

void ChartingTechnicalsPanel::on_last_price_signal(Money last) {
  m_last_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(last), QLocale()));
  update_change_label();
}

void ChartingTechnicalsPanel::on_open_signal(Money open) {
  m_open_value_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(open), QLocale()));
}

void ChartingTechnicalsPanel::on_close_signal(Money close) {
  m_close_value_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(close), QLocale()));
}

void ChartingTechnicalsPanel::on_high_signal(Money high) {
  m_high_value_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(high), QLocale()));
}

void ChartingTechnicalsPanel::on_low_signal(Money low) {
  m_low_value_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(low), QLocale()));
}

void ChartingTechnicalsPanel::on_volume_signal(Quantity volume) {
  m_volume_value_label->setText(m_item_delegate->displayText(
    QVariant::fromValue(volume), QLocale()));
}

void ChartingTechnicalsPanel::update_change_label() {
  auto color = QColor("#FFFFFF");
  if(m_model->get_last_price().is_initialized()) {
    auto change = *(m_model->get_last_price()) - *(m_model->get_open());
    auto change_text = QString(" " + m_item_delegate->displayText(
      QVariant::fromValue(change), QLocale()) + " (" +
      m_item_delegate->displayText(QVariant::fromValue(
        100 * (change / *(m_model->get_open()))),
        QLocale()) + "%)");
    if(change > Money::ZERO) {
      change_text = change_text.replace(0, 1, "+");
      color = QColor("#1FD37A");
    } else if(change < Money::ZERO) {
      change_text = change_text.replace(0, 1, "");
      color = QColor("#EF5357");
    }
    m_change_label->setText(change_text);
  } else {
    m_change_label->setText(tr(""));
  }
  m_change_label->setStyleSheet(QString(R"(
    QLabel {
      color: %1;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
    })").arg(color.name()).arg(scale_height(10)));
}
