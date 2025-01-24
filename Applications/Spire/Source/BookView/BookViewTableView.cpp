#include "Spire/BookView/BookViewTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QResizeEvent>
#include <QTimer>
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/BookView/BookViewHighlightPropertiesPage.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/ColorConversion.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  constexpr auto ORDER_HIGHLIGHT_STATE_COUNT =
    BookViewHighlightProperties::ORDER_HIGHLIGHT_STATE_COUNT;
  using OrderVisibility = BookViewHighlightProperties::OrderVisibility;
  using MarketHighlight = BookViewHighlightProperties::MarketHighlight;
  using MarketHighlightLevel =
    BookViewHighlightProperties::MarketHighlightLevel;
  using OrderHighlightState = BookViewHighlightProperties::OrderHighlightState;
  using OrderHighlightArray =
    std::array<HighlightColor, ORDER_HIGHLIGHT_STATE_COUNT>;
  using UserOrder = BookViewModel::UserOrder;
  using ShowGrid = StateSelector<void, struct ShowGridSeletorTag>;
  using LevelIndicator = StateSelector<int, struct LevelIndicatorTag>;
  using MarketHighlightIndicator =
    StateSelector<std::string, struct MarketHighlightIndicatorTag>;
  using OrderHighlightIndicator =
    StateSelector<std::string, struct OrderHighlightIndicatorTag>;
  using OrderIndicator = StateSelector<void, struct OrderIndicatorTag>;
  using CanceledOrderIndicator =
    StateSelector<void, struct CanceledOrderIndicatorTag>;
  using FilledOrderIndicator =
    StateSelector<void, struct FilledOrderIndicatorTag>;
  using RejectedOrderIndicator =
    StateSelector<void, struct RejectedOrderIndicatorTag>;
  const auto ORDER_HIGHLIGHT_TRANSITION_MS = 100;
  const auto ORDER_HIGHLIGHT_DELAY_MS = 900;
  const auto SELECTED_BACKGROUND_COLOR = QColor(0x8D78EC);
  const auto SELECTED_TEXT_COLOR = QColor(0xFFFFFF);

  enum class SelectorType {
    LEVEL_SELECTOR,
    MARKET_HIGHLIGHT_SELECTOR,
    ORDER_HIGHLIGHT_SELECTOR
  };

  struct OrderKey {
    std::string m_destination;
    Nexus::Money m_price;

    auto operator <=>(const OrderKey&) const = default;
  };

  struct OrderKeyHash {
    std::size_t operator()(const OrderKey& order_key) const {
      auto seed = std::size_t(0);
      hash_combine(seed, hash_value(order_key.m_destination));
      hash_combine(seed, hash_value(order_key.m_price));
      return seed;
    }
  };

  void apply_row_style(StyleSheet& style,
      const Selector& row_selector, const Selector& text_selector,
      const HighlightColor& highlight) {
    style.get(row_selector).set(BackgroundColor(highlight.m_background_color));
    style.get(text_selector).set(TextColor(highlight.m_text_color));
  }

  void apply_row_style(StyleSheet& style, const Selector& selector,
      const HighlightColor& highlight) {
    auto row_selector = selector < is_a<TableItem>() < Row();
    auto text_selector = row_selector > is_a<TableItem>() > is_a<TextBox>();
    apply_row_style(style, row_selector, text_selector, highlight);
  }

  void apply_row_highlight_animation_style(StyleSheet& style,
      const Selector& row_selector, const Selector& text_selector,
      const HighlightColor& old_highlight, const HighlightColor& highlight) {
    style.get(row_selector).set(BackgroundColor(
      linear(old_highlight.m_background_color, highlight.m_background_color,
        milliseconds(ORDER_HIGHLIGHT_TRANSITION_MS))));
    style.get(text_selector).set(TextColor(
      linear(old_highlight.m_text_color, highlight.m_text_color,
        milliseconds(ORDER_HIGHLIGHT_TRANSITION_MS))));
  }

  auto to_string(const OrderKey& order_key) {
    return order_key.m_destination +
      lexical_cast<std::string>(order_key.m_price);
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({"", "", TableHeaderItem::Order::UNORDERED,
      TableFilter::Filter::NONE});
    model->push({"", "", TableHeaderItem::Order::UNORDERED,
      TableFilter::Filter::NONE});
    model->push({"", "", TableHeaderItem::Order::UNORDERED,
      TableFilter::Filter::NONE});
    return model;
  }

  Selector get_order_status_selector(OrderStatus status) {
    if(status == OrderStatus::CANCELED) {
      return CanceledOrderIndicator();
    } else if(status == OrderStatus::FILLED) {
      return FilledOrderIndicator();
    } else if(status == OrderStatus::REJECTED) {
      return RejectedOrderIndicator();
    }
    return OrderIndicator();
  }

  Selector get_order_selector(OrderHighlightState state) {
    if(state == OrderHighlightState::CANCELED) {
      return CanceledOrderIndicator();
    } else if(state == OrderHighlightState::FILLED) {
      return FilledOrderIndicator();
    } else if(state == OrderHighlightState::REJECTED) {
      return RejectedOrderIndicator();
    }
    return OrderIndicator();
  }

  QWidget* get_table_body(QWidget* cell) {
    if(auto table_item = cell->parentWidget()) {
      if(auto row = table_item->parentWidget()) {
        return row->parentWidget();
      }
    }
    return nullptr;
  }

  auto to_highlight_state(OrderStatus status) {
    if(status == OrderStatus::CANCELED) {
      return OrderHighlightState::CANCELED;
    } else if(status == OrderStatus::FILLED) {
      return OrderHighlightState::FILLED;
    } else if(status == OrderStatus::REJECTED) {
      return OrderHighlightState::REJECTED;
    }
    return OrderHighlightState::ACTIVE;
  }

  const auto& get_mpid(const TableModel& table, int row) {
    return table.get<std::string>(row, static_cast<int>(BookViewColumns::MPID));
  }

  const auto& get_price(const TableModel& table, int row) {
    return table.get<Money>(row, static_cast<int>(BookViewColumns::PRICE));
  }

  const auto& get_size(const TableModel& table, int row) {
    return table.get<Quantity>(row, static_cast<int>(BookViewColumns::SIZE));
  }

  auto is_order(const std::string& mpid) {
    return mpid.front() == '@';
  }

  struct RowTracker : QObject {
    struct Selectors {
      optional<Selector> m_level_selector;
      optional<Selector> m_highlight_selector;
    };
    std::shared_ptr<TableModel> m_quote_table;
    ArrayListModel<QWidget*> m_items;
    ArrayListModel<Selectors> m_row_selectors;
    connection m_connection;

    explicit RowTracker(std::shared_ptr<TableModel> quote_table,
      QObject* parent = nullptr)
      : QObject(parent),
        m_quote_table(std::move(quote_table)),
        m_items(std::vector<QWidget*>(m_quote_table->get_row_size(), nullptr)),
        m_row_selectors(std::vector<Selectors>(m_quote_table->get_row_size())),
        m_connection(m_quote_table->connect_operation_signal(
          std::bind_front(&RowTracker::on_table_operation, this))) {}

    QWidget* get_quantity_item(int index) const {
      return m_items.get(index);
    }

    void set_quantity_item(int index, QWidget* item) {
      m_items.set(index, item);
      if(item) {
        item->installEventFilter(this);
      }
    }

    void unmatch_selector(int index, SelectorType selector_type) {
      auto item = get_quantity_item(index);
      if(!item) {
        return;
      }
      auto& selectors = m_row_selectors.get(index);
      if(selector_type == SelectorType::MARKET_HIGHLIGHT_SELECTOR) {
        if(selectors.m_highlight_selector) {
          unmatch(*item, *selectors.m_highlight_selector);
          m_row_selectors.set(index, {selectors.m_level_selector, none});
        }
        if(selectors.m_level_selector) {
          match(*item, *selectors.m_level_selector);
        }
      } else if(selector_type == SelectorType::ORDER_HIGHLIGHT_SELECTOR) {
        if(selectors.m_highlight_selector) {
          unmatch(*item, *selectors.m_highlight_selector);
          m_row_selectors.set(index, {selectors.m_level_selector, none});
        }
        if(selectors.m_level_selector) {
          match(*item, *selectors.m_level_selector);
        }
      } else {
        if(!selectors.m_level_selector) {
          unmatch(*item, *selectors.m_level_selector);
        }
        m_row_selectors.set(index, {none, selectors.m_highlight_selector});
      }
    }

    void match_selector(int index, SelectorType selector_type,
        const Selector& selector) {
      auto item = get_quantity_item(index);
      if(!item) {
        return;
      }
      auto& selectors = m_row_selectors.get(index);
      if(selector_type == SelectorType::LEVEL_SELECTOR) {
        if(!selectors.m_highlight_selector) {
          if(selectors.m_level_selector) {
            unmatch(*item, *selectors.m_level_selector);
          }
          match(*item, selector);
        }
        m_row_selectors.set(index, {selector, selectors.m_highlight_selector});
      } else {
        if(selectors.m_highlight_selector) {
          unmatch(*item, *selectors.m_highlight_selector);
        }
        if(selectors.m_level_selector) {
          unmatch(*item, *selectors.m_level_selector);
        }
        match(*item, selector);
        m_row_selectors.set(index, {selectors.m_level_selector, selector});
      }
    }

    void update_level_style(int index, const LevelIndicator& indicator,
        const QColor& color) {
      if(auto item = get_quantity_item(index)) {
        update_style(*item, [&] (auto& style) {
          apply_row_style(style, indicator,
            HighlightColor(color, get_apca_text_color(color)));
        });
      }
    }

    void update_market_style(int index,
        const MarketHighlightIndicator& indicator,
        const HighlightColor& highlight) {
      if(auto item = get_quantity_item(index)) {
        update_style(*item, [&] (auto& style) {
          apply_row_style(style, indicator, highlight);
        });
      }
    }

    void update_active_order_style(int index, const HighlightColor& highlight) {
      if(auto item = get_quantity_item(index)) {
        update_style(*item, [&] (auto& style) {
          apply_row_style(style, OrderIndicator(), highlight);
        });
      }
    }

    void update_terminal_order_style(int index, const Selector& selector,
        const HighlightColor& old_highlight, const HighlightColor& highlight) {
      if(auto item = get_quantity_item(index)) {
        update_style(*item, [&] (auto& style) {
          auto row_selector = selector < is_a<TableItem>() < Row();
          auto text_selector =
            row_selector > is_a<TableItem>() > is_a<TextBox>();
          apply_row_highlight_animation_style(style, row_selector,
            text_selector, old_highlight, highlight);
        });
      }
    }

    void update_current_order_style(OrderHighlightState state,
        const HighlightColor& old_highlight, const HighlightColor& highlight) {
      auto i = std::find_if(m_items.begin(), m_items.end(), [] (QWidget* item) {
        return item != nullptr;
      });
      if(i == m_items.end()) {
        return;
      }
      if(auto table_body = get_table_body(*i)) {
        update_style(*table_body, [&] (auto& style) {
          auto item_selector = Any() > CurrentRow() > is_a<TableItem>();
          auto order_selector = get_order_selector(state);
          auto row_selector =
            item_selector > order_selector < is_a<TableItem>() < CurrentRow();
          auto text_selector = item_selector > order_selector <
            is_a<TableItem>() < Row() > is_a<TableItem>() > is_a<TextBox>();
          apply_row_highlight_animation_style(style, row_selector,
            text_selector, old_highlight, highlight);
        });
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Destroy) {
        if(auto i = std::find(m_items.begin(), m_items.end(), watched);
            i != m_items.end()) {
          *i = nullptr;
          m_row_selectors.set(std::distance(m_items.begin(), i), {none, none});
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void on_table_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          m_items.insert(nullptr, operation.m_index);
          m_row_selectors.insert({none, none}, operation.m_index);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          if(auto item = get_quantity_item(operation.m_index)) {
            item->removeEventFilter(this);
          }
          m_row_selectors.remove(operation.m_index);
          m_items.remove(operation.m_index);
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(static_cast<BookViewColumns>(operation.m_column) ==
              BookViewColumns::SIZE) {
            if(auto item = get_quantity_item(operation.m_row)) {
              static_cast<TextBox*>(item)->get_current()->set(
                to_text(std::any_cast<const Quantity&>(operation.m_value)));
            }
          }
        },
        [&] (const TableModel::MoveOperation& operation) {
          m_items.move(operation.m_source, operation.m_destination);
          m_row_selectors.move(operation.m_source, operation.m_destination);
      });
    }
  };

  struct LevelQuoteModel {
    std::shared_ptr<TableModel> m_table;
    std::shared_ptr<RowTracker> m_row_tracker;
    std::shared_ptr<ValueModel<BookViewLevelProperties>> m_level_properties;
    std::shared_ptr<ValueModel<OrderVisibility>> m_visibility_property;
    std::vector<LevelIndicator> m_indicators;
    std::vector<int> m_levels;
    std::vector<QColor> m_color_scheme;
    OrderVisibility m_order_visibility;
    connection m_operation_connection;
    connection m_properties_connection;
    connection m_visibility_connection;

    LevelQuoteModel(std::shared_ptr<TableModel> quote_table,
        std::shared_ptr<RowTracker> row_tracker,
        std::shared_ptr<ValueModel<BookViewLevelProperties>> level_properties,
        std::shared_ptr<ValueModel<OrderVisibility>> visibility_property)
        : m_table(std::move(quote_table)),
          m_row_tracker(std::move(row_tracker)),
          m_level_properties(std::move(level_properties)),
          m_visibility_property(std::move(visibility_property)),
          m_order_visibility(m_visibility_property->get()) {
      on_properties_update(m_level_properties->get());
      m_operation_connection = m_table->connect_operation_signal(
        std::bind_front(&LevelQuoteModel::on_operation, this));
      m_properties_connection = m_level_properties->connect_update_signal(
        std::bind_front(&LevelQuoteModel::on_properties_update, this));
      m_visibility_connection = m_visibility_property->connect_update_signal(
        std::bind_front(&LevelQuoteModel::on_visibility_update, this));
    }

    int get_max_level() const {
      return static_cast<int>(m_level_properties->get().m_color_scheme.size());
    }

    int estimate_level(int index) const {
      if(index == 0) {
        return 1;
      }
      auto level = m_levels[index - 1];
      if(level < get_max_level() &&
          get_price(*m_table, index) != get_price(*m_table, index - 1)) {
        ++level;
      }
      return level;
    }

    void highlight(int row, int level) {
      m_row_tracker->match_selector(row, SelectorType::LEVEL_SELECTOR,
        m_indicators[level - 1]);
      m_row_tracker->update_level_style(row, m_indicators[level - 1],
        m_color_scheme[level - 1]);
    }

    void highlight(int row) {
      highlight(row, m_levels[row]);
    }

    void update_styles(int begin, int end) {
      for(auto i = begin; i < end; ++i) {
        auto level = m_levels[i];
        m_row_tracker->update_level_style(i, m_indicators[level - 1],
          m_color_scheme[level - 1]);
      }
    }

    void on_operation(const TableModel::Operation& operation) {
      auto update_levels = [&] (int index) {
        for(auto i = index; i < m_table->get_row_size(); ++i) {
          auto level = estimate_level(i);
          if(m_levels[i] != level) {
            m_levels[i] = level;
            highlight(i);
          } else {
            break;
          }
        }
      };
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          m_levels.insert(m_levels.begin() + operation.m_index,
            estimate_level(operation.m_index));
          update_levels(operation.m_index + 1);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_levels.erase(m_levels.begin() + operation.m_index);
          update_levels(operation.m_index);
        });
    }

    void on_properties_update(const BookViewLevelProperties& properties) {
      auto update_levels = [&] (int index) {
        for(auto i = index; i < m_table->get_row_size(); ++i) {
          m_levels[i] = estimate_level(i);
          highlight(i);
        }
      };
      if(m_color_scheme.size() < properties.m_color_scheme.size()) {
        m_color_scheme = properties.m_color_scheme;
        for(auto i = m_indicators.size(); i < properties.m_color_scheme.size();
            ++i) {
          m_indicators.emplace_back(i + 1);
        }
        if(auto i = std::max_element(m_levels.begin(), m_levels.end());
            i != m_levels.end()) {
          auto index = std::distance(m_levels.begin(), i);
          update_styles(1, index);
          update_levels(index);
        }
      } else if(m_color_scheme.size() > properties.m_color_scheme.size()) {
        m_color_scheme = properties.m_color_scheme;
        for(auto i = properties.m_color_scheme.size(); i < m_indicators.size();
            ++i) {
          m_indicators.erase(m_indicators.begin() + i);
        }
        if(auto i = std::lower_bound(m_levels.begin(), m_levels.end(),
            get_max_level()); i != m_levels.end()) {
          auto index = std::distance(m_levels.begin(), i);
          update_styles(1, index);
          update_levels(index);
        }
      } else {
        m_color_scheme = properties.m_color_scheme;
        update_styles(0, m_table->get_row_size());
      }
    }

    void on_visibility_update(OrderVisibility visibility) {
      if(m_order_visibility != visibility) {
        if(m_order_visibility == OrderVisibility::HIDDEN &&
            visibility != OrderVisibility::HIDDEN ||
            m_order_visibility != OrderVisibility::HIDDEN &&
            visibility == OrderVisibility::HIDDEN) {
          for(auto i = 0; i < m_table->get_row_size(); ++i) {
            m_levels[i] = estimate_level(i);
            highlight(i);
          }
        }
        m_order_visibility = visibility;
      }
    }
  };

  struct MarketHighlightModel {
    struct HighlightContext {
      std::unique_ptr<MarketHighlightIndicator> m_indicator;
      MarketHighlight m_highlight;
    };
    std::shared_ptr<TableModel> m_quote_table;
    std::shared_ptr<RowTracker> m_row_tracker;
    std::shared_ptr<ValueModel<std::vector<MarketHighlight>>>
      m_highlight_properties;
    MarketDatabase m_markets;
    bool m_is_ascending;
    std::unordered_map<std::string, HighlightContext> m_highlight_contexts;
    std::unordered_map<std::string, Quote> m_highlight_tops;
    std::vector<MarketHighlight> m_market_highlights;
    connection m_operation_connection;
    connection m_properties_connection;

    MarketHighlightModel(std::shared_ptr<TableModel> quote_table,
        std::shared_ptr<RowTracker> row_tracker,
        std::shared_ptr<ValueModel<std::vector<MarketHighlight>>> properties,
        const MarketDatabase& markets, bool is_ascending)
        : m_quote_table(std::move(quote_table)),
          m_row_tracker(std::move(row_tracker)),
          m_highlight_properties(std::move(properties)),
          m_markets(markets),
          m_is_ascending(is_ascending) {
      on_properties_update(m_highlight_properties->get());
      m_operation_connection = m_quote_table->connect_operation_signal(
        std::bind_front(&MarketHighlightModel::on_table_operation, this));
      m_properties_connection = m_highlight_properties->connect_update_signal(
        std::bind_front(&MarketHighlightModel::on_properties_update, this));
    }

    void highlight(int row) {
      match_market_highlight(row);
    }

    void match_market_highlight(int row) {
      auto& mpid = get_mpid(*m_quote_table, row);
      if(!m_highlight_contexts.contains(mpid)) {
        return;
      }
      auto& context = m_highlight_contexts[mpid];
      if(context.m_highlight.m_level == MarketHighlightLevel::ALL) {
        m_row_tracker->match_selector(row,
          SelectorType::MARKET_HIGHLIGHT_SELECTOR, *context.m_indicator);
        m_row_tracker->update_market_style(row, *context.m_indicator,
          context.m_highlight.m_color);
      } else {
        auto& price = get_price(*m_quote_table, row);
        auto& size = get_size(*m_quote_table, row);
        auto i = m_highlight_tops.find(mpid);
        if(i != m_highlight_tops.end() && i->second.m_price == price &&
          i->second.m_size == size) {
          m_row_tracker->match_selector(row,
            SelectorType::MARKET_HIGHLIGHT_SELECTOR, *context.m_indicator);
          m_row_tracker->update_market_style(row, *context.m_indicator,
            context.m_highlight.m_color);
        }
      }
    }

    void on_table_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          auto& size = get_size(*m_quote_table, operation.m_index);
          if(auto i = m_highlight_tops.find(mpid);
              i != m_highlight_tops.end()) {
            auto is_topmost = [&] {
              if(m_is_ascending) {
                return i->second.m_price > price ||
                  i->second.m_price == price && i->second.m_size > size;
              }
              return i->second.m_price < price ||
                i->second.m_price == price && i->second.m_size < size;
            }();
            if(is_topmost) {
              if(m_highlight_contexts.contains(mpid) &&
                  m_highlight_contexts[mpid].m_highlight.m_level ==
                  MarketHighlightLevel::TOP) {
                for(auto j = 0; j < m_quote_table->get_row_size(); ++j) {
                  if(mpid == get_mpid(*m_quote_table, j) &&
                      i->second.m_price == get_price(*m_quote_table, j)) {
                    m_row_tracker->unmatch_selector(j,
                      SelectorType::MARKET_HIGHLIGHT_SELECTOR);
                    break;
                  }
                }
              }
              i->second.m_price = price;
              i->second.m_size = size;
            }
          } else {
            auto& quote = m_highlight_tops[mpid];
            quote.m_price = price;
            quote.m_size = size;
          }
        },
        [&] (const TableModel::PreRemoveOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          auto& size = get_size(*m_quote_table, operation.m_index);
          if(auto i = m_highlight_tops.find(mpid);
              i != m_highlight_tops.end()) {
            if(i->second.m_price == price && i->second.m_size == size) {
              for(auto j = operation.m_index + 1;
                  j < m_quote_table->get_row_size(); ++j) {
                if(get_mpid(*m_quote_table, j) == mpid) {
                  i->second.m_price = get_price(*m_quote_table, j);
                  i->second.m_size = get_size(*m_quote_table, j);
                  match_market_highlight(j);
                  break;
                }
              }
            }
          }
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(operation.m_column == static_cast<int>(BookViewColumns::SIZE)) {
            auto& mpid = get_mpid(*m_quote_table, operation.m_row);
            if(auto i = m_highlight_tops.find(mpid);
                i != m_highlight_tops.end()) {
              if(i->second.m_price ==
                  get_price(*m_quote_table, operation.m_row)) {
                i->second.m_size = get_size(*m_quote_table, operation.m_row);
              }
            }
          }
        });
    }

    void on_properties_update(
      const std::vector<MarketHighlight>& market_highlights) {
      if(m_market_highlights != market_highlights) {
        m_market_highlights = market_highlights;
        m_highlight_contexts.clear();
        for(auto& highlight : m_market_highlights) {
          auto& name = m_markets.FromCode(highlight.m_market).m_displayName;
          m_highlight_contexts[name] =
            HighlightContext{std::make_unique<MarketHighlightIndicator>(name),
            highlight};
        }
        for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
          if(!is_order(get_mpid(*m_quote_table, i))) {
            m_row_tracker->unmatch_selector(i,
              SelectorType::MARKET_HIGHLIGHT_SELECTOR);
            match_market_highlight(i);
          }
        }
      }
    }
  };

  struct OrderHighlightModel {
    std::shared_ptr<TableModel> m_quote_table;
    std::shared_ptr<ListModel<BookViewModel::UserOrder>> m_orders;
    std::shared_ptr<RowTracker> m_row_tracker;
    std::shared_ptr<ValueModel<OrderVisibility>> m_visibility_property;
    std::shared_ptr<ValueModel<OrderHighlightArray>> m_highlight_properties;
    bool m_is_ascending;
    ColumnViewListModel<Money> m_prices;
    std::unordered_map<OrderKey, OrderStatus, OrderKeyHash> m_order_status;
    OrderVisibility m_visibility;
    OrderHighlightArray m_highlights;
    QTimer m_timer;
    connection m_table_operation_connection;
    connection m_order_connection;
    connection m_visibility_connection;
    connection m_highlight_connection;

    OrderHighlightModel(std::shared_ptr<TableModel> quote_table,
        std::shared_ptr<ListModel<BookViewModel::UserOrder>> orders,
        std::shared_ptr<RowTracker> row_tracker,
        std::shared_ptr<ValueModel<OrderVisibility>> visibility_property,
        std::shared_ptr<ValueModel<OrderHighlightArray>> highlight_properties,
        bool is_ascending)
        : m_quote_table(std::move(quote_table)),
          m_orders(std::move(orders)),
          m_row_tracker(std::move(row_tracker)),
          m_visibility_property(std::move(visibility_property)),
          m_highlight_properties(std::move(highlight_properties)),
          m_is_ascending(is_ascending),
          m_prices(m_quote_table, static_cast<int>(BookViewColumns::PRICE)) {
      for(auto i = 0; i < m_orders->get_size(); ++i) {
        auto& order = m_orders->get(i);
        m_order_status[OrderKey(order.m_destination, order.m_price)] =
          order.m_status;
      }
      on_visibility_update(m_visibility_property->get());
      on_highlight_update(m_highlight_properties->get());
      m_timer.setSingleShot(true);
      m_order_connection = m_orders->connect_operation_signal(
        std::bind_front(&OrderHighlightModel::on_order_operation, this));
      m_visibility_connection = m_visibility_property->connect_update_signal(
        std::bind_front(&OrderHighlightModel::on_visibility_update, this));
      m_highlight_connection = m_highlight_properties->connect_update_signal(
        std::bind_front(&OrderHighlightModel::on_highlight_update, this));
    }

    void highlight(int row) {
      auto& mpid = get_mpid(*m_quote_table, row);
      if(is_order(mpid)) {
        if(m_visibility == OrderVisibility::HIGHLIGHTED) {
          auto key = OrderKey(mpid.substr(1), get_price(*m_quote_table, row));
          if(auto i = m_order_status.find(key); i != m_order_status.end()) {
            m_row_tracker->match_selector(row,
              SelectorType::ORDER_HIGHLIGHT_SELECTOR,
              get_order_status_selector(i->second));
          }
        }
        auto active_index = static_cast<int>(OrderHighlightState::ACTIVE);
        m_row_tracker->update_active_order_style(row,
          m_highlights[active_index]);
        for(auto i = active_index + 1; i < ORDER_HIGHLIGHT_STATE_COUNT; ++i) {
          m_row_tracker->update_terminal_order_style(row,
            get_order_selector(static_cast<OrderHighlightState>(i)),
            m_highlights[active_index], m_highlights[i]);
        }
      }
    }

    int find_order_index(const UserOrder& order) {
      auto compare = [&] () -> std::function<bool(const Money&, const Money&)> {
        if(m_is_ascending) {
          return std::less<Money>();
        }
        return std::greater<Money>();
      }();
      auto i = std::lower_bound(m_prices.begin(), m_prices.end(), order.m_price,
        compare);
      if(i != m_prices.end()) {
        auto mpid = '@' + order.m_destination;
        for(; i != m_prices.end(); ++i) {
          if(*i != order.m_price) {
            return -1;
          }
          auto index = std::distance(m_prices.begin(), i);
          if(mpid == get_mpid(*m_quote_table, index)) {
            return index;
          }
        }
      }
      return -1;
    }

    void update_order_status(const UserOrder& order) {
      if(auto index = find_order_index(order); index >= 0) {
        m_row_tracker->match_selector(index,
          SelectorType::ORDER_HIGHLIGHT_SELECTOR,
          get_order_status_selector(order.m_status));
      }
    }

    void on_order_operation(const ListModel<UserOrder>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<UserOrder>::AddOperation& operation) {
          auto active_index = static_cast<int>(OrderHighlightState::ACTIVE);
          for(auto i = active_index + 1; i < ORDER_HIGHLIGHT_STATE_COUNT; ++i) {
            m_row_tracker->update_current_order_style(
              static_cast<OrderHighlightState>(i),
              HighlightColor(SELECTED_BACKGROUND_COLOR, SELECTED_TEXT_COLOR),
              m_highlights[i]);
          }
          auto& order = m_orders->get(operation.m_index);
          auto key = OrderKey(order.m_destination, order.m_price);
          m_order_status[key] = order.m_status;
          if(m_visibility == OrderVisibility::HIGHLIGHTED) {
            update_order_status(order);
          }
        },
        [&] (const ListModel<UserOrder>::PreRemoveOperation& operation) {
          auto& order = m_orders->get(operation.m_index);
          m_timer.disconnect();
          QObject::connect(&m_timer, &QTimer::timeout, [=] {
            if(auto index = find_order_index(order); index >= 0) {
              m_quote_table->remove(index);
            }
          });
          m_timer.start(ORDER_HIGHLIGHT_DELAY_MS);
        },
        [&] (const ListModel<UserOrder>::UpdateOperation& operation) {
          auto& order = m_orders->get(operation.m_index);
          auto key = OrderKey(order.m_destination, order.m_price);
          m_order_status[key] = order.m_status;
          if(m_visibility_property->get() != OrderVisibility::HIGHLIGHTED ||
              order.m_status == OrderStatus::NONE) {
            return;
          }
          update_order_status(order);
          m_timer.disconnect();
          QObject::connect(&m_timer, &QTimer::timeout, [=] {
            if(auto i = m_order_status.find(key); i != m_order_status.end()) {
              if(i->second == OrderStatus::NONE) {
                update_order_status(UserOrder(i->first.m_destination,
                  i->first.m_price, i->second));
              }
            }
          });
          m_timer.start(ORDER_HIGHLIGHT_DELAY_MS);
        });
    }

    void on_visibility_update(const OrderVisibility& visibility) {
      if(m_visibility != visibility) {
        if(visibility == OrderVisibility::VISIBLE) {
          for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
            auto& mpid = get_mpid(*m_quote_table, i);
            if(is_order(mpid)) {
              m_row_tracker->unmatch_selector(i,
                SelectorType::ORDER_HIGHLIGHT_SELECTOR);
            }
          }
        } else if(visibility == OrderVisibility::HIGHLIGHTED) {
          for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
            auto key = OrderKey(get_mpid(*m_quote_table, i).substr(1),
              get_price(*m_quote_table, i));
            if(auto j = m_order_status.find(key); j != m_order_status.end()) {
              m_row_tracker->match_selector(i,
                SelectorType::ORDER_HIGHLIGHT_SELECTOR,
                get_order_status_selector(j->second));
            }
          }
        }
        m_visibility = visibility;
      }
    }

    void on_highlight_update(const OrderHighlightArray& highlights) {
      if(m_visibility_property->get() != OrderVisibility::HIGHLIGHTED) {
        return;
      }
      auto active_highlight_updated = false;
      auto active_index = static_cast<int>(OrderHighlightState::ACTIVE);
      if(m_highlights[active_index] != highlights[active_index]) {
        active_highlight_updated = true;
        m_highlights[active_index] = highlights[active_index];
        for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
          if(is_order(get_mpid(*m_quote_table, i))) {
            m_row_tracker->update_active_order_style(i,
              highlights[active_index]);
          }
        }
      }
      for(auto i = active_index + 1; i < ORDER_HIGHLIGHT_STATE_COUNT; ++i) {
        if(m_highlights[i] != highlights[i] || active_highlight_updated) {
          if(m_highlights[i] != highlights[i]) {
            m_highlights[i] = highlights[i];
            m_row_tracker->update_current_order_style(
              static_cast<OrderHighlightState>(i),
              HighlightColor(SELECTED_BACKGROUND_COLOR, SELECTED_TEXT_COLOR),
              highlights[i]);
          }
          for(auto j = 0; j < m_quote_table->get_row_size(); ++j) {
            if(is_order(get_mpid(*m_quote_table, j))) {
              m_row_tracker->update_terminal_order_style(j,
                get_order_selector(static_cast<OrderHighlightState>(i)),
                highlights[active_index], highlights[i]);
            }
          }
        }
      }
    }
  };

  struct BookViewTableViewObserver : QObject {
    TableView* m_table_view;
    std::shared_ptr<ListModel<BookQuote>> m_book_quotes;
    std::shared_ptr<ListModel<UserOrder>> m_orders;
    std::shared_ptr<BookViewPropertiesModel> m_properties;
    std::shared_ptr<LevelQuoteModel> m_level_quote_model;
    std::shared_ptr<MarketHighlightModel> m_market_highlight_model;
    std::shared_ptr<OrderHighlightModel> m_order_highlight_model;
    std::shared_ptr<LevelPropertiesModel> m_level_properties;
    std::shared_ptr<HighlightPropertiesModel> m_highlight_properties;
    bool m_is_grid_enabled;
    QFont m_font;
    connection m_level_properties_connection;
    connection m_current_connection;
    connection m_operation_connection;

    BookViewTableViewObserver(TableView& table_view,
       std::shared_ptr<ListModel<BookQuote>> book_quotes,
       std::shared_ptr<ListModel<UserOrder>> orders,
       std::shared_ptr<BookViewPropertiesModel> properties,
       std::shared_ptr<LevelQuoteModel> level_quote_model,
       std::shared_ptr<MarketHighlightModel> market_highlight_model,
       std::shared_ptr<OrderHighlightModel> order_highlight_model,
       QObject* parent = nullptr)
      : QObject(parent),
        m_table_view(&table_view),
        m_book_quotes(std::move(book_quotes)),
        m_orders(std::move(orders)),
        m_properties(std::move(properties)),
        m_level_quote_model(std::move(level_quote_model)),
        m_market_highlight_model(std::move(market_highlight_model)),
        m_order_highlight_model(std::move(order_highlight_model)),
        m_level_properties(make_field_value_model(m_properties,
          &BookViewProperties::m_level_properties)),
        m_highlight_properties(make_field_value_model(m_properties,
          &BookViewProperties::m_highlight_properties)),
        m_is_grid_enabled(m_level_properties->get().m_is_grid_enabled),
        m_font(m_level_properties->get().m_font) {
      m_table_view->installEventFilter(this);
      m_table_view->get_body().installEventFilter(this);
      on_level_properties_update(m_level_properties->get());
      m_level_properties_connection = m_level_properties->connect_update_signal(
        std::bind_front(&BookViewTableViewObserver::on_level_properties_update,
          this));
      m_current_connection = m_table_view->get_current()->connect_update_signal(
        std::bind_front(&BookViewTableViewObserver::on_current, this));
      m_operation_connection = m_book_quotes->connect_operation_signal(
        std::bind_front(&BookViewTableViewObserver::on_operation, this));
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_table_view && event->type() == QEvent::Resize) {
        auto& resize_event = *static_cast<QResizeEvent*>(event);
        auto column_width = resize_event.size().width() / 3;
        m_table_view->get_header().get_widths()->set(0, column_width);
        m_table_view->get_header().get_widths()->set(1, column_width);
      } else if(watched == &m_table_view->get_body()) {
        if(event->type() == QEvent::KeyPress) {
          auto& key_event = *static_cast<QKeyEvent*>(event);
          if(key_event.key() == Qt::Key_Home &&
              key_event.modifiers() & Qt::ControlModifier) {
            move_current(
              std::bind_front(&BookViewTableViewObserver::find_home_order,
                this));
            return true;
          } else if(key_event.key() == Qt::Key_End &&
              key_event.modifiers() & Qt::ControlModifier) {
            move_current(
              std::bind_front(&BookViewTableViewObserver::find_end_order,
                this));
            return true;
          } else if(key_event.key() == Qt::Key_Up) {
            move_current(
              std::bind_front(&BookViewTableViewObserver::find_previous_order,
                this));
            return true;
          } else if(key_event.key() == Qt::Key_Down) {
            move_current(
              std::bind_front(&BookViewTableViewObserver::find_next_order,
                this));
            return true;
          }
        }
      }
      return QObject::eventFilter(watched, event);
    }

    optional<int> find_home_order(int row) {
      for(auto i = 0; i < row; ++i) {
        if(is_order(get_mpid(*m_table_view->get_table(), i))) {
          return i;
        }
      }
      return none;
    }

    optional<int> find_end_order(int row) {
      for(auto i = m_table_view->get_table()->get_row_size() - 1; i > row;
          --i) {
        if(is_order(get_mpid(*m_table_view->get_table(), i))) {
          return i;
        }
      }
      return none;
    }

    optional<int> find_previous_order(int row) {
      for(auto i = row - 1; i >= 0; --i) {
        if(is_order(get_mpid(*m_table_view->get_table(), i))) {
          return i;
        }
      }
      return none;
    }

    optional<int> find_next_order(int row) {
      for(auto i = row + 1; i < m_table_view->get_table()->get_row_size();
          ++i) {
        if(is_order(get_mpid(*m_table_view->get_table(), i))) {
          return i;
        }
      }
      return none;
    }

    void move_current(std::function<optional<int>(int row)> find_order) {
      if(auto current = m_table_view->get_current()->get()) {
        if(auto next = find_order(current->m_row);
            next && next != current->m_row) {
          auto blocker = shared_connection_block(m_current_connection);
          m_table_view->get_current()->set(TableView::Index(*next, 0));
        }
      }
    }

    void on_level_properties_update(const BookViewLevelProperties& properties) {
      if(m_is_grid_enabled != properties.m_is_grid_enabled) {
        if(properties.m_is_grid_enabled) {
          match(m_table_view->get_body(), ShowGrid());
        } else {
          unmatch(m_table_view->get_body(), ShowGrid());
        }
        m_is_grid_enabled = properties.m_is_grid_enabled;
      }
      if(m_font != properties.m_font) {
        update_style(m_table_view->get_body(), [&] (auto& style) {
          style.get(Any() > Row() > is_a<TableItem>() > is_a<TextBox>()).
            set(Font(properties.m_font));
        });
        m_font = properties.m_font;
      }
    }

    void on_current(const optional<TableView::Index>& current) {
      if(!current) {
        return;
      }
      if(!is_order(get_mpid(*m_table_view->get_table(), current->m_row))) {
        m_table_view->get_current()->set(none);
      }
    }

    void on_operation(const ListModel<BookQuote>::Operation& operation) {
      visit(operation,
        [&] (const auto& operation) {
          if(auto current = m_table_view->get_current()->get()) {
            if(!is_order(
                get_mpid(*m_table_view->get_table(), current->m_row))) {
              m_table_view->get_current()->set(none);
            }
          }
        });
    }
  };

  struct OrderFilteredListModel : FilteredListModel<BookQuote> {
    std::shared_ptr<HighlightPropertiesModel> m_highlight_properties;
    BookViewHighlightProperties::OrderVisibility m_order_visibility;
    connection m_connection;

    OrderFilteredListModel(std::shared_ptr<ListModel<BookQuote>> source,
      std::shared_ptr<HighlightPropertiesModel> highlight_properties)
      : FilteredListModel<BookQuote>(std::move(source),
          std::bind_front(&OrderFilteredListModel::filter, this)),
        m_highlight_properties(std::move(highlight_properties)),
        m_order_visibility(m_highlight_properties->get().m_order_visibility),
        m_connection(m_highlight_properties->connect_update_signal(
          std::bind_front(&OrderFilteredListModel::on_properties, this))) {}

    bool filter(const ListModel<BookQuote>& list, int index) {
      if(m_highlight_properties->get().m_order_visibility ==
          BookViewHighlightProperties::OrderVisibility::HIDDEN) {
        return is_order(list.get(index).m_mpid);
      }
      return false;
    }

    void on_properties(const BookViewHighlightProperties& properties) {
      if(m_order_visibility != properties.m_order_visibility) {
        if(m_order_visibility == OrderVisibility::HIDDEN &&
            properties.m_order_visibility != OrderVisibility::HIDDEN ||
            m_order_visibility != OrderVisibility::HIDDEN &&
            properties.m_order_visibility == OrderVisibility::HIDDEN) {
          set_filter(std::bind_front(&OrderFilteredListModel::filter, this));
        }
        m_order_visibility = properties.m_order_visibility;
      }
    }
  };

  QWidget* item_builder(std::shared_ptr<RowTracker> row_tracker,
      std::shared_ptr<LevelQuoteModel> level_quote_model,
      std::shared_ptr<MarketHighlightModel> market_highlight_model,
      std::shared_ptr<OrderHighlightModel> order_highlight_model,
      const std::shared_ptr<TableModel>& table,
      int row, int column) {
    auto item = [&] {
      auto column_id = static_cast<BookViewColumns>(column);
      if(column_id == BookViewColumns::MPID) {
        auto mpid_item = make_label(
          QString::fromStdString(table->get<std::string>(row, column)));
        update_style(*mpid_item, [] (auto& style) {
          style.get(Any()).
            set(PaddingLeft(scale_width(4))).
            set(PaddingRight(scale_width(2)));
        });
        return mpid_item;
      } else if(column_id == BookViewColumns::PRICE) {
        auto money_item = make_label(to_text(table->get<Money>(row, column)));
        update_style(*money_item, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(horizontal_padding(scale_width(2)));
        });
        return money_item;
      } else if(column_id == BookViewColumns::SIZE) {
        auto quantity_item =
          make_label(to_text(table->get<Quantity>(row, column)));
        update_style(*quantity_item, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(PaddingLeft(scale_width(2))).
            set(PaddingRight(scale_width(4)));
        });
        row_tracker->set_quantity_item(row, quantity_item);
        level_quote_model->highlight(row);
        market_highlight_model->highlight(row);
        order_highlight_model->highlight(row);
        return quantity_item;
      }
      return make_label("");
    }();
    return item;
  }
}

TableView* Spire::make_book_view_table_view(
    std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties, Side side,
    const MarketDatabase& markets, QWidget* parent) {
  auto [book_quotes, orders, ordering] = [&] {
    if(side == Side::BID) {
      return std::tuple(model->get_bids(), model->get_bid_orders(),
        SortedTableModel::Ordering::DESCENDING);
    }
    return std::tuple(model->get_asks(), model->get_ask_orders(),
      SortedTableModel::Ordering::ASCENDING);
  }();
  auto is_ascending = ordering == SortedTableModel::Ordering::ASCENDING;
  auto highlight_property = make_field_value_model(properties,
    &BookViewProperties::m_highlight_properties);
  auto order_visibility = make_field_value_model(highlight_property,
    &BookViewHighlightProperties::m_order_visibility);
  auto column_orders = std::vector<SortedTableModel::ColumnOrder>{
    {1, ordering}, {2, ordering}};
  auto order_filtered_list = std::make_shared<OrderFilteredListModel>(
    std::move(book_quotes), highlight_property);
  auto table = std::make_shared<SortedTableModel>(
    make_book_view_table_model(order_filtered_list), column_orders);
  auto row_tracker = std::make_shared<RowTracker>(table);
  auto level_quote_model = std::make_shared<LevelQuoteModel>(table, row_tracker,
    make_field_value_model(properties, &BookViewProperties::m_level_properties),
    order_visibility);
  auto market_highlight_model = std::make_shared<MarketHighlightModel>(table,
    row_tracker, make_field_value_model(highlight_property,
      &BookViewHighlightProperties::m_market_highlights), markets,
      is_ascending);
  auto order_highlight_model = std::make_shared<OrderHighlightModel>(table,
    orders, row_tracker, order_visibility,
    make_field_value_model(highlight_property,
      &BookViewHighlightProperties::m_order_highlights), is_ascending);
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(std::bind_front(item_builder, row_tracker,
      level_quote_model, market_highlight_model, order_highlight_model)).make();
  table_view->get_header().setVisible(false);
  table_view->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
  auto observer = new BookViewTableViewObserver(*table_view,
    order_filtered_list, orders, properties, level_quote_model,
    market_highlight_model, order_highlight_model, table_view);
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
    style.get(Any() > CurrentRow() > is_a<TableItem>() > is_a<TextBox>()).
      set(TextColor(SELECTED_TEXT_COLOR));
    style.get(ShowGrid()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_height(1)));
  });
  return table_view;
}
