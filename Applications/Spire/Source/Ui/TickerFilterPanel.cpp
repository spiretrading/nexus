#include "Spire/Ui/TickerFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

TickerFilterPanel* Spire::make_ticker_filter_panel(
    std::shared_ptr<TickerInfoQueryModel> tickers, QWidget* parent) {
  return make_ticker_filter_panel(std::move(tickers),
    std::make_shared<ArrayListModel<Ticker>>(), parent);
}

TickerFilterPanel* Spire::make_ticker_filter_panel(
    std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<TickerListModel> current, QWidget* parent) {
  auto box = make_ticker_list_box(std::move(tickers), std::move(current));
  box->set_placeholder(QObject::tr("Enter tickers"));
  return new OpenFilterPanel(*box, parent);
}
