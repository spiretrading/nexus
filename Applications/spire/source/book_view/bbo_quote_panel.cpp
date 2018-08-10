#include "spire/book_view/bbo_quote_panel.hpp"
#include <QHBoxLayout>
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/quote_panel.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

BboQuotePanel::BboQuotePanel(const std::shared_ptr<BookViewModel>& model,
    QWidget* parent)
    : QWidget(parent) {
  setFixedHeight(scale_height(28));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_bid_quote_panel = new QuotePanel(model, Nexus::Side::BID, this);
  layout->addWidget(m_bid_quote_panel);
  layout->addSpacing(scale_width(2));
  m_ask_quote_panel = new QuotePanel(model, Nexus::Side::ASK, this);
  layout->addWidget(m_ask_quote_panel);
}
