#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using UserOrderRow = StateSelector<void, struct UserOrderSelectorTag>;
  using PreviewRow = StateSelector<void, struct PreviewSelectorTag>;
  using ShowGrid = StateSelector<void, struct ShowGridSeletorTag>;
  const auto SELECTED_BACKGROUND_COLOR = QColor(0x8D78EC);
  const auto SELECTED_TEXT_COLOR = QColor(0xFFFFFF);

  auto make_header_model() {
    static const auto COLUMN_COUNT = 3;
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    for(auto i = 0; i != COLUMN_COUNT; ++i) {
      model->push(
        {"", "", TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    }
    return model;
  }

  QWidget* item_builder(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto column_id = static_cast<BookViewColumn>(column);
    if(column_id == BookViewColumn::MPID) {
      auto& listing = table->get<MpidListing>(row, column);
      auto mpid_item = make_label(QString::fromStdString(listing.m_mpid));
      if(listing.m_source == ListingSource::PREVIEW) {
        match(*mpid_item, PreviewRow());
      } else if(listing.m_source == ListingSource::USER_ORDER) {
        match(*mpid_item, UserOrderRow());
      }
      update_style(*mpid_item, [] (auto& style) {
        style.get(Any()).
          set(PaddingLeft(scale_width(4))).
          set(PaddingRight(scale_width(2)));
      });
      return mpid_item;
    } else if(column_id == BookViewColumn::PRICE) {
      auto money_item = make_label(make_to_text_model(
        make_table_value_model<Money>(table, row, column)));
      update_style(*money_item, [] (auto& style) {
        style.get(Any()).
          set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
          set(horizontal_padding(scale_width(2)));
      });
      return money_item;
    } else {
      auto quantity_item = make_label(make_to_text_model(
        make_table_value_model<Quantity>(table, row, column)));
      update_style(*quantity_item, [] (auto& style) {
        style.get(Any()).
          set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
          set(PaddingLeft(scale_width(2))).
          set(PaddingRight(scale_width(4)));
      });
      return quantity_item;
    }
  }

  bool listing_comparator(const AnyRef& left, const AnyRef& right) {
    if(left.get_type() == typeid(MpidListing)) {
      return any_cast<MpidListing>(left) < any_cast<MpidListing>(right);
    }
    return Spire::compare(left, right);
  }

  struct TableViewColumnSizer : QObject {
    TableViewColumnSizer(TableView& table_view)
        : QObject(&table_view) {
      table_view.installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Resize) {
        auto& table_view = *static_cast<TableView*>(parent());
        auto& resize_event = *static_cast<QResizeEvent*>(event);
        auto column_width = resize_event.size().width() / 3;
        table_view.get_header().get_widths()->set(0, column_width);
        table_view.get_header().get_widths()->set(1, column_width);
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

TableView* Spire::make_book_view_table_view(
    std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties, Side side,
    const MarketDatabase& markets, QWidget* parent) {
  auto [quotes, orders, ordering] = [&] {
    if(side == Side::BID) {
      return std::tuple(model->get_bids(), model->get_bid_orders(),
        SortedTableModel::Ordering::DESCENDING);
    }
    return std::tuple(model->get_asks(), model->get_ask_orders(),
      SortedTableModel::Ordering::ASCENDING);
  }();
  auto is_ascending = ordering == SortedTableModel::Ordering::ASCENDING;
  auto column_orders =
    std::vector<SortedTableModel::ColumnOrder>{{1, ordering}, {2, ordering}};
  auto merged_table = std::make_shared<MergedBookQuoteListModel>(
    quotes, orders, model->get_preview_order());
  auto table = std::make_shared<SortedTableModel>(
    make_book_view_table_model(merged_table), column_orders,
    &listing_comparator);
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(&item_builder).make();
  table_view->get_header().setVisible(false);
  table_view->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  auto column_sizer = new TableViewColumnSizer(*table_view);
  update_style(table_view->get_body(), [=] (auto& style) {
    auto item_selector = Any() > Row() > is_a<TableItem>();
    style.get(item_selector > is_a<TextBox>()).
      set(Font(properties->get().m_level_properties.m_font)).
      set(vertical_padding(scale_width(1.5)));
    style.get(Any()).
      set(HorizontalSpacing(0)).
      set(VerticalSpacing(0)).
      set(grid_color(QColor(0xE0E0E0)));
    style.get(Any() > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    style.get(Any() > CurrentRow()).
      set(BackgroundColor(SELECTED_BACKGROUND_COLOR)).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > Row() > is_a<TableItem>() > PreviewRow() < Any() < Row()).
      set(BackgroundColor(
        properties->get().m_highlight_properties.m_order_highlights[0].m_background_color));
    style.get(Any() > Row() > is_a<TableItem>() > UserOrderRow() < Any() < Row()).
      set(BackgroundColor(
        properties->get().m_highlight_properties.m_order_highlights[1].m_background_color));
    style.get(Any() > CurrentRow() > is_a<TableItem>() > is_a<TextBox>()).
      set(TextColor(SELECTED_TEXT_COLOR));
    style.get(ShowGrid()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_height(1)));
  });
  return table_view;
}
