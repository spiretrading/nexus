#include "Spire/Ui/TickerListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TickerInfoToTickerQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TickerListItem.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

TickerListBox* Spire::make_ticker_list_box(
    std::shared_ptr<TickerInfoQueryModel> tickers, QWidget* parent) {
  return make_ticker_list_box(
    std::move(tickers), std::make_shared<ArrayListModel<Ticker>>(), parent);
}

TickerListBox* Spire::make_ticker_list_box(
    std::shared_ptr<TickerInfoQueryModel> tickers,
    std::shared_ptr<TickerListModel> current, QWidget* parent) {
  return new TagComboBox<Ticker>(
    std::make_shared<TickerInfoToTickerQueryModel>(tickers), std::move(current),
    [=] (const std::shared_ptr<ListModel<Ticker>>& list, int index) {
      if(auto info = tickers->parse(to_text(list->get(index)))) {
        return new TickerListItem(*info);
      }
      return new TickerListItem(TickerInfo());
    }, parent);
}
