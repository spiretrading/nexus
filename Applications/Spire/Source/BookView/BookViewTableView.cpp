#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/BookView/BookViewCurrentTableModel.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"
#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include "Spire/BookView/MpidBox.hpp"
#include "Spire/BookView/PreviewOrderDisplayValueModel.hpp"
#include "Spire/BookView/PriceLevelModel.hpp"
#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/ColorConversion.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
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

  template<typename T, typename U>
  struct BookViewProxyValueModel : ValueModel<U> {
    using Type = typename ValueModel<U>::Type;

    using UpdateSignal = typename ValueModel<U>::UpdateSignal;

    std::shared_ptr<ProxyValueModel<T>> m_proxy;
    std::shared_ptr<ValueModel<U>> m_target;

    BookViewProxyValueModel(std::shared_ptr<ValueModel<T>> source)
      : m_proxy(make_proxy_value_model(std::move(source))) {}

    const std::shared_ptr<ProxyValueModel<T>>& get_proxy() const {
      return m_proxy;
    }

    void set_source(std::shared_ptr<ValueModel<T>> source) {
      get_proxy()->set_source(std::move(source));
    }

    void set_target(std::shared_ptr<ValueModel<U>> target) {
      m_target = std::move(target);
    }

    const Type& get() const override {
      return m_target->get();
    }

    QValidator::State test(const Type& value) const override {
      return m_target->test(value);
    }

    QValidator::State set(const Type& value) override {
      return m_target->set(value);
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override {
      return m_target->connect_update_signal(slot);
    }
  };

  struct ItemBuilder {
    std::shared_ptr<PriceLevelModel> m_price_levels;
    std::shared_ptr<TopMpidPriceListModel> m_top_mpid_prices;

    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto column_id = static_cast<BookViewColumn>(column);
      if(column_id == BookViewColumn::MPID) {
        auto entry = make_proxy_value_model(
          make_table_value_model<BookEntry>(table, row, column));
        auto level =
          make_proxy_value_model(make_list_value_model(m_price_levels, row));
        auto is_top_mpid =
          std::make_shared<BookViewProxyValueModel<Money, bool>>(
            make_table_value_model<Money>(
              table, row, static_cast<int>(BookViewColumn::PRICE)));
        is_top_mpid->set_target(std::make_shared<IsTopMpidModel>(
          m_top_mpid_prices, entry, is_top_mpid->get_proxy()));
        auto mpid_box = new MpidBox(
          std::move(entry), std::move(level), std::move(is_top_mpid));
        return mpid_box;
      } else if(column_id == BookViewColumn::PRICE) {
        auto current =
          std::make_shared<BookViewProxyValueModel<Money, QString>>(
            make_table_value_model<Money>(table, row, column));
        current->set_target(make_to_text_model(current->get_proxy()));
        auto price_box = make_label(std::move(current));
        update_style(*price_box, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(horizontal_padding(scale_width(2)));
        });
        return price_box;
      } else {
        auto current =
          std::make_shared<BookViewProxyValueModel<Quantity, QString>>(
            make_table_value_model<Quantity>(table, row, column));
        current->set_target(
          make_to_text_model(make_transform_value_model(current->get_proxy(),
            [] (auto quantity) {
              if(quantity == 0) {
                return Quantity(0);
              }
              return std::max<Quantity>(1, Floor(quantity / 100, 0));
            })));
        auto quantity_box = make_label(std::move(current));
        update_style(*quantity_box, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(PaddingLeft(scale_width(2))).
            set(PaddingRight(scale_width(4)));
        });
        return quantity_box;
      }
    }

    void reset(QWidget& widget, const std::shared_ptr<TableModel>& table,
        int row, int column) {
      auto column_id = static_cast<BookViewColumn>(column);
      if(column_id == BookViewColumn::MPID) {
        auto& mpid_box = static_cast<MpidBox&>(widget);
        auto& entry = static_cast<ProxyValueModel<BookEntry>&>(
          *mpid_box.get_current().get());
        entry.set_source(make_table_value_model<BookEntry>(table, row, column));
        auto& level =
          static_cast<ProxyValueModel<int>&>(*mpid_box.get_level().get());
        level.set_source(make_list_value_model(m_price_levels, row));
        auto& is_top_mpid = static_cast<BookViewProxyValueModel<Money, bool>&>(
          *mpid_box.is_top_mpid().get());
        is_top_mpid.set_source(make_table_value_model<Money>(
          table, row, static_cast<int>(BookViewColumn::PRICE)));
      } else if(column_id == BookViewColumn::PRICE) {
        auto& price_box = static_cast<TextBox&>(widget);
        auto& current = static_cast<BookViewProxyValueModel<Money, QString>&>(
          *price_box.get_current().get());
        current.set_source(make_table_value_model<Money>(table, row, column));
      } else {
        auto& quantity_box = static_cast<TextBox&>(widget);
        auto& current =
          static_cast<BookViewProxyValueModel<Quantity, QString>&>(
            *quantity_box.get_current().get());
        current.set_source(
          make_table_value_model<Quantity>(table, row, column));
      }
    }

    void unmount(QWidget* widget) {
      delete widget;
    }
  };

  auto make_row_selector(Selector selector) {
    return Any() > (+Row() > (Any() > selector));
  }

  auto make_item_selector(Selector selector) {
    return make_row_selector(selector) > Any() > Any();
  }

  void apply_row_style(StyleSheet& style, Selector selector,
      TextColor text_color, BackgroundColor background_color) {
    style.get(make_row_selector(selector)).set(std::move(background_color));
    style.get(make_item_selector(selector)).set(std::move(text_color));
  }

  void clear_row_style(StyleSheet& style, Selector selector) {
    style.get(make_row_selector(selector)).clear();
    style.get(make_item_selector(selector)).clear();
  }

  void apply_transition_styles(StyleSheet& style,
      const HighlightColor& active_highlight,
      const HighlightColor& status_highlight, OrderStatus status) {
    const auto HIGHLIGHT_TRANSITION_DURATION = milliseconds(100);
    const auto HIGHLIGHT_DURATION = milliseconds(900);
    apply_row_style(style, UserOrderRow(status),
      TextColor(chain(linear(active_highlight.m_text_color,
        status_highlight.m_text_color, HIGHLIGHT_TRANSITION_DURATION),
        timeout(status_highlight.m_text_color, HIGHLIGHT_DURATION), revert)),
      BackgroundColor(chain(linear(active_highlight.m_background_color,
        status_highlight.m_background_color, HIGHLIGHT_TRANSITION_DURATION),
        timeout(status_highlight.m_background_color, HIGHLIGHT_DURATION),
        revert)));
  }

  void apply_order_visibility_styles(
      StyleSheet& style, const BookViewProperties& properties) {
    if(properties.m_highlight_properties.m_order_visibility ==
        BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
      auto& preview_highlight = get_highlight(
        properties, BookViewHighlightProperties::OrderHighlightState::PREVIEW);
      apply_row_style(style, PreviewRow(),
        TextColor(preview_highlight.m_text_color),
        BackgroundColor(preview_highlight.m_background_color));
      auto& active_highlight = get_highlight(
        properties, BookViewHighlightProperties::OrderHighlightState::ACTIVE);
      apply_row_style(style, UserOrderRow(OrderStatus::NONE),
        TextColor(active_highlight.m_text_color),
        BackgroundColor(active_highlight.m_background_color));
      auto& filled_highlight = get_highlight(
        properties, BookViewHighlightProperties::OrderHighlightState::FILLED);
      apply_transition_styles(
        style, active_highlight, filled_highlight, OrderStatus::FILLED);
      auto& canceled_highlight = get_highlight(
        properties, BookViewHighlightProperties::OrderHighlightState::CANCELED);
      apply_transition_styles(
        style, active_highlight, canceled_highlight, OrderStatus::CANCELED);
      auto& rejected_highlight = get_highlight(
        properties, BookViewHighlightProperties::OrderHighlightState::REJECTED);
      apply_transition_styles(
        style, active_highlight, rejected_highlight, OrderStatus::REJECTED);
    } else {
      clear_row_style(style, PreviewRow());
      for(auto status : {OrderStatus::NONE, OrderStatus::FILLED,
          OrderStatus::CANCELED, OrderStatus::REJECTED}) {
        clear_row_style(style, UserOrderRow(status));
      }
    }
  }

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

    void apply_market_highlight_styles(
        StyleSheet& style, const BookViewProperties& properties) {
      for(auto& highlight : m_previous_market_highlights) {
        if(highlight.m_level ==
            BookViewHighlightProperties::MarketHighlightLevel::TOP) {
          clear_row_style(
            style, TopMarketRow() && MarketRow(highlight.m_market));
        } else {
          clear_row_style(style, MarketRow(highlight.m_market));
        }
      }
      m_previous_market_highlights.clear();
      auto& market_highlights =
        properties.m_highlight_properties.m_market_highlights;
      for(auto& highlight : market_highlights) {
        if(highlight.m_level ==
            BookViewHighlightProperties::MarketHighlightLevel::TOP) {
          apply_row_style(
            style, TopMarketRow() && MarketRow(highlight.m_market),
            TextColor(highlight.m_color.m_text_color),
            BackgroundColor(highlight.m_color.m_background_color));
        } else {
          apply_row_style(style, MarketRow(highlight.m_market),
            TextColor(highlight.m_color.m_text_color),
            BackgroundColor(highlight.m_color.m_background_color));
        }
        m_previous_market_highlights.push_back(
          PreviousMarketHighlight(highlight.m_market, highlight.m_level));
      }
    }

    void apply_level_highlight_styles(
        StyleSheet& style, const BookViewProperties& properties) {
      auto& level_colors = properties.m_level_properties.m_color_scheme;
      for(auto i = level_colors.size(); i < m_previous_levels; ++i) {
        clear_row_style(style, PriceLevelRow(i));
      }
      m_previous_levels = level_colors.size();
      for(auto i = std::size_t(0); i < level_colors.size(); ++i) {
        apply_row_style(style, PriceLevelRow(i),
          TextColor(apca_text_color(level_colors[i])),
          BackgroundColor(level_colors[i]));
      }
    }

    void on_properties(const BookViewProperties& properties) {
      auto& table_view = *static_cast<TableView*>(parent());
      if(properties.m_level_properties.m_is_grid_enabled) {
        match(table_view.get_body(), ShowGrid());
      } else {
        unmatch(table_view.get_body(), ShowGrid());
      }
      update_style(table_view.get_body(), [&] (auto& style) {
        style.get(Any()).
          set(HorizontalSpacing(0)).
          set(VerticalSpacing(0)).
          set(grid_color(QColor(0xE0E0E0)));
        style.get(ShowGrid()).
          set(HorizontalSpacing(scale_width(1))).
          set(VerticalSpacing(scale_height(1)));
        apply_level_highlight_styles(style, properties);
        apply_order_visibility_styles(style, properties);
        apply_market_highlight_styles(style, properties);
        style.get(Any() > Row() > Any() > Any()).
          set(Font(properties.m_level_properties.m_font));
        style.get(Any() > CurrentColumn()).
          set(BackgroundColor(Qt::transparent));
        style.get(Any() > CurrentRow()).
          set(BackgroundColor(SELECTED_BACKGROUND_COLOR)).
          set(border_color(QColor(0x4B23A0)));
        style.get(Any() > Row() > Current()).
          set(BackgroundColor(Qt::transparent)).
          set(border_color(QColor(Qt::transparent)));
        style.get(Any() > CurrentRow() > Any() > Any()).
          set(TextColor(SELECTED_TEXT_COLOR));
        style.get(Any() > Row() > Any() > Any()).
          set(vertical_padding(scale_width(1.5)));
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
  auto column_orders = std::vector<SortedTableModel::ColumnOrder>{
    {1, ordering}, {2, SortedTableModel::Ordering::DESCENDING}};
  auto displayed_orders =
    std::make_shared<UserOrderDisplayListModel>(
      std::make_shared<ConsolidatedUserOrderListModel>(std::move(orders)),
      properties);
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
  auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(
    std::make_shared<SortedListModel<BookQuote>>(
      std::move(quotes), &BookQuoteListingComparator));
  auto proxy_current = make_proxy_value_model(
    std::make_shared<LocalValueModel<optional<TableIndex>>>());
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_current(proxy_current).
    set_item_builder(RecycledTableViewItemBuilder(
      ItemBuilder(std::move(price_levels), std::move(top_mpid_prices)))).make();
  proxy_current->set_source(std::make_shared<BookViewCurrentTableModel>(table));
  table_view->get_header().setVisible(false);
  table_view->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  auto stylist = new TableViewStylist(*table_view, std::move(properties));
  return table_view;
}
