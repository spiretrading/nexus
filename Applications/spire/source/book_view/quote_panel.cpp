#include "spire/book_view/quote_panel.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "spire/book_view/book_view_model.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace Spire;

QuotePanel::QuotePanel(const std::shared_ptr<BookViewModel>& model, Side side,
    QWidget* parent)
    : QWidget(parent),
      m_side(side) {
  model->connect_bbo_slot([=] (auto& b) { on_bbo_quote(b); });
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_indicator_widget = new QWidget(this);
  m_indicator_widget->setFixedHeight(scale_height(2));
  m_indicator_widget->setAutoFillBackground(true);
  set_indicator_color("#C8C8C8");
  layout->addWidget(m_indicator_widget);
  auto label_layout = new QHBoxLayout();
  m_price_label = new QLabel(this);
  m_price_label->setAlignment(Qt::AlignCenter);
  m_price_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12)));
  label_layout->addWidget(m_price_label);
  m_size_label = new QLabel(this);
  m_size_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  m_size_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    padding-top: %2px;)").arg(scale_height(10)).arg(scale_height(1)));
  label_layout->addWidget(m_size_label);
  layout->addLayout(label_layout);
  auto bbo = model->get_bbo();
  if(m_side == Side::BID) {
    if(bbo.m_bid.m_price == Nexus::Money()) {
      m_price_label->setText(tr("N/A"));
      m_size_label->hide();
    } else {
      set_quote_text(bbo.m_bid.m_price, bbo.m_bid.m_size);
    }
  } else {
    if(bbo.m_ask.m_price == Nexus::Money()) {
      m_price_label->setText(tr("N/A"));
      m_size_label->hide();
    } else {
      set_quote_text(bbo.m_ask.m_price, bbo.m_ask.m_size);
    }
  }
}

void QuotePanel::set_indicator_color(const QColor& color) {
  m_indicator_widget->setStyleSheet(
    QString("background-color: %1").arg(color.name()));
}

void QuotePanel::set_quote_text(const Money& price, const Quantity& size) {
    m_price_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_price_label->setText(QString::fromStdString(price.ToString()));
    m_size_label->setText(QString::fromStdString(
      " / " + Beam::ToString(size)));
}

void QuotePanel::on_bbo_quote(const BboQuote& bbo) {
  if(m_side == Side::BID) {
    if(bbo.m_bid.m_price > m_current_bbo.m_bid.m_price) {
      set_indicator_color("#37D186");
    } else if(bbo.m_bid.m_price < m_current_bbo.m_bid.m_price) {
      set_indicator_color("#FF6F7A");
    }
    if(bbo.m_bid.m_price != Money()) {
      set_quote_text(bbo.m_bid.m_price, bbo.m_bid.m_size);
    }
  } else {
    if(bbo.m_ask.m_price > m_current_bbo.m_ask.m_price) {
      set_indicator_color("#37D186");
    } else if(bbo.m_ask.m_price < m_current_bbo.m_ask.m_price) {
      set_indicator_color("#FF6F7A");
    }
    if(bbo.m_ask.m_price != Money()) {
      set_quote_text(bbo.m_ask.m_price, bbo.m_ask.m_size);
    }
  }
  m_current_bbo = bbo;
}
