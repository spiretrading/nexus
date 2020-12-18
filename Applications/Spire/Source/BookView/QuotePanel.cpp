#include "Spire/BookView/QuotePanel.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/QuotePanelIndicatorWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Nexus;
using namespace Spire;

QuotePanel::QuotePanel(const BookViewModel& model, Side side,
    QWidget* parent)
    : QWidget(parent),
      m_side(side) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_indicator_widget = new QuotePanelIndicatorWidget(this);
  m_indicator_widget->setFixedHeight(scale_height(2));
  m_indicator_widget->setAutoFillBackground(true);
  layout->addWidget(m_indicator_widget);
  auto label_layout = new QHBoxLayout();
  label_layout->setContentsMargins({});
  label_layout->setSpacing(0);
  m_price_label = new QLabel(this);
  m_price_label->setSizePolicy(QSizePolicy::Ignored,
    m_price_label->sizePolicy().verticalPolicy());
  m_price_label->setAlignment(Qt::AlignRight);
  m_price_label->setStyleSheet(QString(R"(
    QLabel {
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      font-weight: 550;
      padding-right: %2px;
    })").arg(scale_height(12)).arg(scale_width(2)));
  label_layout->addWidget(m_price_label);
  auto separator = new QLabel("/", this);
  separator->setAlignment(Qt::AlignCenter);
  separator->setSizePolicy(QSizePolicy::Fixed,
    separator->sizePolicy().verticalPolicy());
  separator->setStyleSheet(QString(R"(
    QLabel {
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      padding-top: %2px;
      font-weight: 550;
    })").arg(scale_height(10)).arg(scale_height(1)));
  label_layout->addWidget(separator);
  m_size_label = new QLabel(this);
  m_size_label->setSizePolicy(QSizePolicy::Ignored,
    m_size_label->sizePolicy().verticalPolicy());
  m_size_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  m_size_label->setStyleSheet(QString(R"(
    QLabel {
      color: #4B23A0;
      font-family: Roboto;
      font-size: %1px;
      padding-left: %3px;
      padding-top: %2px;
      font-weight: 550;
    })").arg(scale_height(10)).arg(scale_height(1)).arg(scale_width(2)));
  label_layout->addWidget(m_size_label);
  layout->addLayout(label_layout);
  m_item_delegate = new CustomVariantItemDelegate(this);
  set_model(model);
}

void QuotePanel::set_model(const BookViewModel& model) {
  m_indicator_widget->set_color("#C8C8C8");
  m_current_bbo = model.get_bbo();
  if(m_side == Side::BID) {
    set_quote_text(m_current_bbo.m_bid.m_price, m_current_bbo.m_bid.m_size);
  } else {
    set_quote_text(m_current_bbo.m_ask.m_price, m_current_bbo.m_ask.m_size);
  }
  m_bbo_connection = model.connect_bbo_slot(
    [=] (auto& b) { on_bbo_quote(b); });
}

void QuotePanel::set_quote_text(const Money& price, const Quantity& size) {
    m_price_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_price_label->setText(m_item_delegate->displayText(
      QVariant::fromValue(price), QLocale()));
    m_size_label->setText(m_item_delegate->displayText(
      QVariant::fromValue(size), QLocale()));
}

void QuotePanel::on_bbo_quote(const BboQuote& bbo) {
  auto get_quote = [&] (auto& b) {
    if(m_side == Side::BID) {
      return b.m_bid;
    }
    return b.m_ask;
  };
  auto quote = get_quote(bbo);
  auto current_quote = get_quote(m_current_bbo);
  if(quote.m_price > current_quote.m_price) {
    m_indicator_widget->animate_color("#37D186");
  } else if(quote.m_price < current_quote.m_price) {
    m_indicator_widget->animate_color("#FF6F7A");
  }
  set_quote_text(quote.m_price, quote.m_size);
  m_current_bbo = bbo;
}
