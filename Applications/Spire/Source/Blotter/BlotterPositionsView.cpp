#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_table(std::shared_ptr<BlotterPositionsModel> positions) {
    return std::make_shared<ArrayTableModel>();
  }
}

BlotterPositionsView::BlotterPositionsView(
    std::shared_ptr<BlotterPositionsModel> positions, QWidget* parent)
    : m_positions(std::move(positions)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto command_list = std::make_shared<ArrayListModel<Command>>();
  command_list->push(Command::FLATTEN);
  command_list->push(Command::FLATTEN_ALL);
  auto commands = new ListView(command_list,
    [] (const std::shared_ptr<ListModel<Command>>& model, auto index) {
      auto path = [&] {
        if(model->get(index) == Command::FLATTEN) {
          return ":/Icons/blotter/positions/flatten.svg";
        }
        return ":/Icons/blotter/positions/flatten_all.svg";
      }();
      return make_icon_button(imageFromSvg(path, scale(26, 26)));
    });
  update_style(*commands, [] (auto& style) {
    style.get(Any()).
      set(Qt::Orientation::Horizontal).
      set(EdgeNavigation::CONTAIN);
    style.get(Any() > is_a<ListItem>()).
      set(padding(0)).
      set(border(0, QColor(Qt::transparent))).
      set(BackgroundColor(QColor(Qt::transparent)));
  });
  auto command_bar = new Box(commands, nullptr);
  command_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*command_bar, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xC8C8C8))).
      set(padding(scale_width(8)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(command_bar);
  auto table_view_builder = TableViewBuilder(to_table(positions));
  table_view_builder.add_header_item(tr("Security"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Quantity"), tr("Qty"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Side"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Average Price"), tr("Avg Px"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Profit/Loss"), tr("P/L"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Fees"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Cost Basis"), tr("CB"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Currency"), tr("Curr"), TableFilter::Filter::NONE);
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(table);
}

const std::shared_ptr<BlotterPositionsModel>&
    BlotterPositionsView::get_positions() const {
  return m_positions;
}

connection BlotterPositionsView::connect_flatten_signal(
    const FlattenSignal::slot_type& slot) const {
  return m_flatten_signal.connect(slot);
}

connection BlotterPositionsView::connect_reverse_signal(
    const ReverseSignal::slot_type& slot) const {
  return m_reverse_signal.connect(slot);
}
