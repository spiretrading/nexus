#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/PriceLevelModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using PriceLevelRow = StateSelector<int, struct PriceLevelRowSelectorTag>;
  using UserOrderRow = StateSelector<void, struct UserOrderSelectorTag>;
  using PreviewRow = StateSelector<void, struct PreviewSelectorTag>;
  using ShowGrid = StateSelector<void, struct ShowGridSeletorTag>;
  const auto SELECTED_BACKGROUND_COLOR = QColor(0x8D78EC);
  const auto SELECTED_TEXT_COLOR = QColor(0xFFFFFF);

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    for(auto i = 0; i != BOOK_VIEW_COLUMN_SIZE; ++i) {
      model->push(
        {"", "", TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    }
    return model;
  }

  struct MpidBox : QWidget {
    std::shared_ptr<ValueModel<MpidListing>> m_listing;
    optional<ListingSource> m_previous_source;
    std::shared_ptr<ValueModel<int>> m_level;
    int m_previous_level;
    scoped_connection m_listing_connection;
    scoped_connection m_level_connection;

    MpidBox(std::shared_ptr<ValueModel<MpidListing>> listing,
        std::shared_ptr<ValueModel<int>> level)
        : m_listing(std::move(listing)),
          m_level(std::move(level)),
          m_previous_level(-1) {
      auto label = make_label(make_read_only_to_text_model(
        m_listing, std::bind_front(&MpidBox::make_mpid, this)));
      enclose(*this, *label);
      proxy_style(*this, *label);
      update_style(*this, [] (auto& style) {
        style.get(Any()).
          set(PaddingLeft(scale_width(4))).
          set(PaddingRight(scale_width(2)));
      });
      on_listing(m_listing->get());
      on_level(m_level->get());
      m_listing_connection = m_listing->connect_update_signal(
        std::bind_front(&MpidBox::on_listing, this));
      m_level_connection = m_level->connect_update_signal(
        std::bind_front(&MpidBox::on_level, this));
    }

    QString make_mpid(const MpidListing& listing) const {
      if(listing.m_source == ListingSource::USER_ORDER ||
          listing.m_source == ListingSource::PREVIEW) {
        return QString::fromStdString('@' + listing.m_mpid);
      }
      return QString::fromStdString(listing.m_mpid);
    }

    void on_listing(const MpidListing& listing) {
      if(listing.m_source == m_previous_source) {
        return;
      }
      if(m_previous_source == ListingSource::PREVIEW) {
        unmatch(*this, PreviewRow());
      } else if(m_previous_source == ListingSource::USER_ORDER) {
        unmatch(*this, UserOrderRow());
      }
      if(listing.m_source == ListingSource::PREVIEW) {
        match(*this, PreviewRow());
      } else if(listing.m_source == ListingSource::USER_ORDER) {
        match(*this, UserOrderRow());
      }
      m_previous_source = listing.m_source;
    }

    void on_level(int level) {
      unmatch(*this, PriceLevelRow(m_previous_level));
      match(*this, PriceLevelRow(level));
      m_previous_level = level;
    }
  };

  struct ItemBuilder {
    std::shared_ptr<PriceLevelModel> m_price_levels;

    QWidget* operator ()(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto column_id = static_cast<BookViewColumn>(column);
      if(column_id == BookViewColumn::MPID) {
        return new MpidBox(
          make_table_value_model<MpidListing>(table, row, column),
          make_list_value_model(m_price_levels, row));
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
  };

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
  auto price_levels = std::make_shared<PriceLevelModel>(
    std::make_shared<ColumnViewListModel<Money>>(
      table, static_cast<int>(BookViewColumn::PRICE)),
    std::make_shared<LocalValueModel<int>>(static_cast<int>(
      properties->get().m_level_properties.m_color_scheme.size() - 1)));
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(ItemBuilder(std::move(price_levels))).make();
  table_view->get_header().setVisible(false);
  table_view->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  auto column_sizer = new TableViewColumnSizer(*table_view);
  update_style(table_view->get_body(), [=] (auto& style) {
    auto item_selector = Any() > Row() > is_a<TableItem>();
    style.get(item_selector > Any()).
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
    style.get(Any() > +Row() > is_a<TableItem>() > PreviewRow()).
      set(BackgroundColor(
        properties->get().m_highlight_properties.m_order_highlights[0].m_background_color));
    style.get(Any() > +Row() > is_a<TableItem>() > UserOrderRow()).
      set(BackgroundColor(
        properties->get().m_highlight_properties.m_order_highlights[1].m_background_color));
    style.get(Any() > CurrentRow() > is_a<TableItem>() > is_a<TextBox>()).
      set(TextColor(SELECTED_TEXT_COLOR));
    style.get(ShowGrid()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_height(1)));
    for(auto i = 0; i != properties->get().m_level_properties.m_color_scheme.size(); ++i) {
      style.get(Any() > +Row() > is_a<TableItem>() > PriceLevelRow(i)).
        set(BackgroundColor(
          properties->get().m_level_properties.m_color_scheme[i]));
    }
  });
  return table_view;
}
