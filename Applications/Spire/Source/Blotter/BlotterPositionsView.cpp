#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/PortfolioToPositionsTableModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

const QKeySequence BlotterPositionsView::FLATTEN_SELECTED_KEY_SEQUENCE =
  QKeySequence(Qt::CTRL + Qt::Key_F);

const QKeySequence BlotterPositionsView::FLATTEN_ALL_KEY_SEQUENCE =
  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F);

BlotterPositionsView::BlotterPositionsView(
    std::shared_ptr<PortfolioModel> portfolio,
    std::shared_ptr<ListModel<int>> selection, QWidget* parent)
    : QWidget(parent),
      m_portfolio(std::move(portfolio)),
      m_selection(std::move(selection)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto command_list = std::make_shared<ArrayListModel<Command>>();
  command_list->push(Command::FLATTEN);
  command_list->push(Command::FLATTEN_ALL);
  auto commands = new ListView(command_list,
    [=] (const std::shared_ptr<ListModel<Command>>& model, auto index) {
      auto [path, tooltip, slot] = [&] {
        if(model->get(index) == Command::FLATTEN) {
          return std::tuple(":/Icons/blotter/positions/flatten.svg",
            tr("Flatten") +
              " (" + FLATTEN_SELECTED_KEY_SEQUENCE.toString() + ")",
            std::function<void ()>(
              std::bind_front(&BlotterPositionsView::on_flatten, this)));
        }
        return std::tuple(":/Icons/blotter/positions/flatten_all.svg",
          tr("Flatten all") +
            " (" + FLATTEN_ALL_KEY_SEQUENCE.toString() + ")",
          std::function<void ()>(
            std::bind_front(&BlotterPositionsView::on_flatten_all, this)));
      }();
      auto button = make_icon_button(
        imageFromSvg(path, scale(26, 26)), std::move(tooltip));
      button->connect_click_signal(slot);
      return button;
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
  m_positions = std::make_shared<PortfolioToPositionsTableModel>(m_portfolio);
  auto table_view_builder = TableViewBuilder(m_positions);
  table_view_builder.add_header_item(tr("Security"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Quantity"), tr("Qty"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Side"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Average Price"), tr("Avg Px"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Profit/Loss"), tr("P/L"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Cost Basis"), tr("CB"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(
    tr("Currency"), tr("Curr"), TableFilter::Filter::NONE);
  auto table_selection = std::make_shared<TableSelectionModel>(
    std::make_shared<TableEmptySelectionModel>(), m_selection,
    std::make_shared<ListEmptySelectionModel>());
  table_view_builder.set_selection(std::move(table_selection));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  layout->addWidget(scroll_box);
}

const std::shared_ptr<PortfolioModel>&
    BlotterPositionsView::get_portfolio() const {
  return m_portfolio;
}

connection BlotterPositionsView::connect_flatten_signal(
    const FlattenSignal::slot_type& slot) const {
  return m_flatten_signal.connect(slot);
}

connection BlotterPositionsView::connect_reverse_signal(
    const ReverseSignal::slot_type& slot) const {
  return m_reverse_signal.connect(slot);
}

void BlotterPositionsView::on_flatten() {
  auto securities = std::vector<Security>();
  for(auto i = 0; i != m_selection->get_size(); ++i) {
    securities.push_back(m_positions->get<Security>(m_selection->get(i),
      static_cast<int>(PortfolioToPositionsTableModel::Column::SECURITY)));
  }
  m_flatten_signal(securities);
}

void BlotterPositionsView::on_flatten_all() {
  auto securities = std::vector<Security>();
  for(auto i = 0; i != m_positions->get_row_size(); ++i) {
    securities.push_back(m_positions->get<Security>(i,
      static_cast<int>(PortfolioToPositionsTableModel::Column::SECURITY)));
  }
  m_flatten_signal(securities);
}