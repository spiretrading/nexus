#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include "Spire/BookView/MpidBox.hpp"
#include "Spire/BookView/PreviewOrderDisplayValueModel.hpp"
#include "Spire/BookView/PriceLevelModel.hpp"
#include "Spire/BookView/TopMpidLevelListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

namespace {
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

  struct ItemBuilder {
    std::shared_ptr<PriceLevelModel> m_price_levels;
    std::shared_ptr<TopMpidLevelListModel> m_top_mpid_levels;

    QWidget* operator ()(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto column_id = static_cast<BookViewColumn>(column);
      if(column_id == BookViewColumn::MPID) {
        auto mpid = make_table_value_model<Mpid>(table, row, column);
        auto level = make_list_value_model(m_price_levels, row);
        auto is_top_mpid =
          std::make_shared<IsTopMpidModel>(m_top_mpid_levels, mpid, level);
        return new MpidBox(mpid, level, is_top_mpid);
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

  struct TableViewStylist : QObject {
    struct PreviousMarketHighlight {
      MarketCode m_market;
      BookViewHighlightProperties::MarketHighlightLevel m_level;
    };
    std::shared_ptr<BookViewPropertiesModel> m_properties;
    std::size_t m_previous_levels;
    std::vector<PreviousMarketHighlight> m_previous_market_highlights;
    scoped_connection m_connection;

    TableViewStylist(TableView& table_view,
        std::shared_ptr<BookViewPropertiesModel> properties)
        : QObject(&table_view),
          m_properties(std::move(properties)),
          m_previous_levels(0) {
      table_view.installEventFilter(this);
      update_style(table_view.get_body(), [=] (auto& style) {
        style.get(Any()).
          set(HorizontalSpacing(0)).
          set(VerticalSpacing(0)).
          set(grid_color(QColor(0xE0E0E0)));
        style.get(ShowGrid()).
          set(HorizontalSpacing(scale_width(1))).
          set(VerticalSpacing(scale_height(1)));
        style.get(Any() > CurrentColumn()).
          set(BackgroundColor(Qt::transparent));
        style.get(Any() > CurrentRow()).
          set(BackgroundColor(SELECTED_BACKGROUND_COLOR)).
          set(border_color(QColor(0x4B23A0)));
        style.get(Any() > Row() > Current()).
          set(BackgroundColor(Qt::transparent)).
          set(border_color(QColor(Qt::transparent)));
        style.get(Any() > CurrentRow() > is_a<TableItem>() > Any()).
          set(TextColor(SELECTED_TEXT_COLOR));
        style.get((Any() > Row() > is_a<TableItem>()) > Any()).
          set(vertical_padding(scale_width(1.5)));
      });
      on_properties(m_properties->get());
      m_connection = m_properties->connect_update_signal(
        std::bind_front(&TableViewStylist::on_properties, this));
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

    void apply_order_visibility_styles(
        StyleSheet& style, const BookViewProperties& properties) {
      if(properties.m_highlight_properties.m_order_visibility ==
          BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
        auto& preview_highlight = get_highlight(properties,
          BookViewHighlightProperties::OrderHighlightState::PREVIEW);
        style.get(Any() > +Row() > is_a<TableItem>() > PreviewRow()).
          set(BackgroundColor(preview_highlight.m_background_color));
        auto& active_highlight = get_highlight(
          properties, BookViewHighlightProperties::OrderHighlightState::ACTIVE);
        style.get(Any() > +Row() > is_a<TableItem>() > UserOrderRow()).
          set(BackgroundColor(active_highlight.m_background_color));
      } else {
        style.get(Any() > +Row() > is_a<TableItem>() > PreviewRow()).clear();
        style.get(Any() > +Row() > is_a<TableItem>() > UserOrderRow()).clear();
      }
    }

    void apply_market_highlight_styles(
        StyleSheet& style, const BookViewProperties& properties) {
      for(auto& highlight : m_previous_market_highlights) {
        if(highlight.m_level ==
            BookViewHighlightProperties::MarketHighlightLevel::TOP) {
          style.get(Any() > +Row() > is_a<TableItem>() >
            (TopMarketRow() && MarketRow(highlight.m_market))).clear();
        } else {
          style.get(Any() >
            +Row() > is_a<TableItem>() > MarketRow(highlight.m_market)).clear();
        }
      }
      m_previous_market_highlights.clear();
      auto& market_highlights =
        properties.m_highlight_properties.m_market_highlights;
      for(auto& highlight : market_highlights) {
        if(highlight.m_level ==
            BookViewHighlightProperties::MarketHighlightLevel::TOP) {
          style.get(Any() > +Row() > is_a<TableItem>() >
            (TopMarketRow() && MarketRow(highlight.m_market))).
              set(BackgroundColor(highlight.m_color.m_background_color)).
              set(TextColor(highlight.m_color.m_text_color));
        } else {
          style.get(Any() >
            +Row() > is_a<TableItem>() > MarketRow(highlight.m_market)).
              set(BackgroundColor(highlight.m_color.m_background_color)).
              set(TextColor(highlight.m_color.m_text_color));
        }
        m_previous_market_highlights.push_back(
          PreviousMarketHighlight(highlight.m_market, highlight.m_level));
      }
    }

    void apply_level_highlight_styles(
        StyleSheet& style, const BookViewProperties& properties) {
      auto& level_colors = properties.m_level_properties.m_color_scheme;
      for(auto i = level_colors.size(); i < m_previous_levels; ++i) {
        style.get(Any() > +Row() > is_a<TableItem>() > PriceLevelRow(i)).
          clear();
      }
      m_previous_levels = level_colors.size();
      for(auto i = std::size_t(0); i < level_colors.size(); ++i) {
        style.get(Any() > +Row() > is_a<TableItem>() > PriceLevelRow(i)).
          set(BackgroundColor(level_colors[i]));
      }
    }

    void on_properties(const BookViewProperties& properties) {
      auto& table_view = *static_cast<TableView*>(parent());
      if(properties.m_level_properties.m_is_grid_enabled) {
        match(table_view.get_body(), ShowGrid());
      } else {
        unmatch(table_view.get_body(), ShowGrid());
      }
      update_style(table_view.get_body(), [=] (auto& style) {
        style.get((Any() > Row() > is_a<TableItem>()) > Any()).
          set(Font(properties.m_level_properties.m_font));
        apply_order_visibility_styles(style, properties);
        apply_market_highlight_styles(style, properties);
        apply_level_highlight_styles(style, properties);
      });
    }
  };

  auto make_max_level_model(
      std::shared_ptr<BookViewPropertiesModel> properties) {
    return make_transform_value_model(std::move(properties),
      [] (const auto& properties) {
        return std::max(static_cast<int>(
          properties.m_level_properties.m_color_scheme.size()) - 1, 1);
      });
  }

  auto filter_by_side(
      std::shared_ptr<BookViewModel::PreviewOrderModel> preview, Side side) {
    return make_transform_value_model(std::move(preview),
      [=] (const auto& preview) -> const optional<OrderFields>& {
        if(preview && preview->m_side == side) {
          return preview;
        }
        static const auto NONE = optional<OrderFields>();
        return NONE;
      });
  }

  struct UserOrderDisplayListModel :
      FilteredListModel<BookViewModel::UserOrder> {
    std::shared_ptr<BookViewPropertiesModel> m_properties;
    bool m_previous_is_displayed;
    scoped_connection m_connection;

    UserOrderDisplayListModel(
        std::shared_ptr<BookViewModel::UserOrderListModel> orders,
        std::shared_ptr<BookViewPropertiesModel> properties)
        : FilteredListModel(std::move(orders),
            [] (const auto&, auto) { return false; }),
          m_properties(std::move(properties)) {
      on_properties(m_properties->get());
      m_connection = m_properties->connect_update_signal(
        std::bind_front(&UserOrderDisplayListModel::on_properties, this));
    }

    void on_properties(const BookViewProperties& properties) {
      auto is_displayed =
        properties.m_highlight_properties.m_order_visibility !=
          BookViewHighlightProperties::OrderVisibility::HIDDEN;
      if(is_displayed == m_previous_is_displayed) {
        return;
      }
      m_previous_is_displayed = is_displayed;
      set_filter([=] (const auto&, auto) {
        return !is_displayed;
      });
    }
  };
}

TableView* Spire::make_book_view_table_view(
    std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties, Side side,
    QWidget* parent) {
  auto [quotes, orders, ordering] = [&] {
    if(side == Side::BID) {
      return std::tuple(model->get_bids(), model->get_bid_orders(),
        SortedTableModel::Ordering::DESCENDING);
    }
    return std::tuple(model->get_asks(), model->get_ask_orders(),
      SortedTableModel::Ordering::ASCENDING);
  }();
  auto column_orders =
    std::vector<SortedTableModel::ColumnOrder>{{1, ordering}, {2, ordering}};
  auto displayed_orders =
    std::make_shared<UserOrderDisplayListModel>(std::move(orders), properties);
  auto displayed_preview = std::make_shared<PreviewOrderDisplayValueModel>(
    filter_by_side(model->get_preview_order(), side), properties);
  auto entries = std::make_shared<MergedBookEntryListModel>(quotes,
    std::move(displayed_orders), std::move(displayed_preview));
  auto table = std::make_shared<SortedTableModel>(
    make_book_view_table_model(std::move(entries)), std::move(column_orders),
    &book_view_comparator);
  auto price_levels = std::make_shared<PriceLevelModel>(
    std::make_shared<ColumnViewListModel<Money>>(
      table, static_cast<int>(BookViewColumn::PRICE)),
      make_max_level_model(properties));
  auto top_mpid_levels =
    std::make_shared<TopMpidLevelListModel>(std::move(quotes));
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(
      ItemBuilder(std::move(price_levels), std::move(top_mpid_levels))).make();
  table_view->get_header().setVisible(false);
  table_view->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  auto stylist = new TableViewStylist(*table_view, std::move(properties));
  return table_view;
}
