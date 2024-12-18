#include "Spire/BookView/BookViewTableView.hpp"
#include <ranges>
#include <QResizeEvent>
#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include "Spire/BookView/BookViewHighlightPropertiesPage.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
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
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

//struct OrderQuoteStatus {
//  OrderKey m_key;
//  OrderStatus m_status;
//
//  auto operator <=>(const OrderQuoteStatus&) const = default;
//};
//
//namespace std {
//  template<>
//  struct hash<OrderQuoteStatus> {
//    std::size_t operator ()(const OrderQuoteStatus& value) const {
//      auto seed = std::size_t(0);
//      boost::hash_combine(seed, value.m_key.m_destination);
//      boost::hash_combine(seed, value.m_key.m_price);
//      boost::hash_combine(seed, value.m_status);
//      return seed;
//    }
//  };
//}

namespace {
  using ShowGrid = StateSelector<void, struct ShowGridSeletorTag>;
  using LevelIndicator = StateSelector<int, struct LevelIndicatorTag>;
  using MarketHighlightIndicator = StateSelector<std::string, struct MarketHighlightIndicatorTag>;
  using OrderHighlightIndicator = StateSelector<std::string, struct OrderHighlightIndicatorTag>;
  //using ActiveOrderIndicator = StateSelector<void, struct ActiveOrderIndicatorTag>;
  using OrderIndicator = StateSelector<void, struct OrderIndicatorTag>;
  using CanceledOrderIndicator = StateSelector<void, struct CanceledOrderIndicatorTag>;
  using FilledOrderIndicator = StateSelector<void, struct FilledCanceledOrderIndicatorTag>;
  using RejectedOrderIndicator = StateSelector<void, struct RejectedOrderIndicatorTag>;

  struct OrderKey {
    std::string m_destination;
    Nexus::Money m_price;

    auto operator <=>(const OrderKey&) const = default;
  };

  auto to_string(const OrderKey& order_key) {
    return order_key.m_destination + lexical_cast<std::string>(order_key.m_price);
  }

  struct OrderKeyHash {
    std::size_t operator()(const OrderKey& order_key) const {
      auto seed = std::size_t(0);
      hash_combine(seed, hash_value(order_key.m_destination));
      hash_combine(seed, hash_value(order_key.m_price));
      return seed;
    }
  };

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

  auto get_text_color(const QColor& background_color) {
    if(std::abs(apca(QColor(Qt::black), background_color)) >
        std::abs(apca(QColor(Qt::white), background_color))) {
      return QColor(Qt::black);
    }
    return QColor(Qt::white);
  }

  const auto& get_mpid(TableModel& table, int index) {
    return table.get<std::string>(index, static_cast<int>(BookViewColumns::MPID));
  }

  const auto& get_price(TableModel& table, int index) {
    return table.get<Money>(index, static_cast<int>(BookViewColumns::PRICE));
  }

  const auto& get_size(TableModel& table, int index) {
    return table.get<Quantity>(index, static_cast<int>(BookViewColumns::SIZE));
  }

  auto is_order(const std::string& mpid) {
    return mpid.front() == '@';
  }

  void make_menu(TableView& table_view,
      const std::shared_ptr<BookViewPropertiesModel>& properties) {
    table_view.setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(&table_view, &QWidget::customContextMenuRequested,
      [=, &table_view] (const auto& pos) {
        auto menu = new ContextMenu(table_view);
        menu->add_action(QObject::tr("Properties"), [] () {
          

        });
        menu->window()->setAttribute(Qt::WA_DeleteOnClose);
        menu->window()->move(table_view.mapToGlobal(pos));
        menu->window()->show();
      });
  }

  //auto apply_table_cell_right_align_style(StyleSheet& style) {
  //  style.get(Any()).set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
  //}

  void apply_row_style(StyleSheet& style, const Selector& level_selector,
      const QColor& background_color/*, const QColor& text_color*/) {
    style.get(level_selector).
      set(BackgroundColor(background_color));
    //style.get(level_selector > is_a<TableItem>() > is_a<TextBox>()).
    //  set(TextColor(text_color));
  };

  struct RowObserver : QObject {
    std::shared_ptr<TableModel> m_quote_table;
    ArrayListModel<TextBox*> m_rows;
    connection m_connection;

    RowObserver(std::shared_ptr<TableModel> quote_table,
      QObject* parent = nullptr)
      : QObject(parent),
        m_quote_table(std::move(quote_table)),
        m_rows(std::vector<TextBox*>(m_quote_table->get_row_size())),
        m_connection(m_quote_table->connect_operation_signal(
          std::bind_front(&RowObserver::on_table_operation, this))) {
    }

    int get_size() const {
      return m_rows.get_size();
    }

    TextBox* get(int index) const {
      return m_rows.get(index);
    }

    void set(int index, TextBox* widget) {
      m_rows.set(index, widget);
      if(widget) {
        widget->installEventFilter(this);
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Destroy) {
        if(auto i = std::find(m_rows.begin(), m_rows.end(), watched);
            i != m_rows.end()) {
          *i = nullptr;
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void on_table_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          m_rows.insert(nullptr, operation.m_index);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          auto row = get(operation.m_index);
          m_rows.remove(operation.m_index);
          if(row) {
            row->removeEventFilter(this);
          }
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(static_cast<BookViewColumns>(operation.m_column) == BookViewColumns::SIZE) {
            if(auto size_column = get(operation.m_row)) {
              size_column->get_current()->set(
                to_text(std::any_cast<const Quantity&>(operation.m_value)));
            }
          }
        },
        [&] (const TableModel::MoveOperation& operation) {
          m_rows.move(operation.m_source, operation.m_destination);
      });
    }
  };

  struct LevelQuoteModel {
    std::shared_ptr<TableModel> m_table;
    std::shared_ptr<RowObserver> m_row_observer;
    std::shared_ptr<BookViewPropertiesModel> m_properties;
    std::vector<LevelIndicator> m_indicators;
    std::vector<int> m_levels;
    std::vector<QColor> m_color_scheme;
    BookViewHighlightProperties::OrderVisibility m_order_visibility;
    connection m_operation_connection;
    connection m_properties_connection;

    LevelQuoteModel(std::shared_ptr<TableModel> quote_table,
        std::shared_ptr<RowObserver> row_observer,
        std::shared_ptr<BookViewPropertiesModel> properties)
        : m_table(std::move(quote_table)),
          m_row_observer(std::move(row_observer)),
          m_properties(std::move(properties)) {
      on_properties_update(m_properties->get());
      m_operation_connection = m_table->connect_operation_signal(
        std::bind_front(&LevelQuoteModel::on_operation, this));
      m_properties_connection = m_properties->connect_update_signal(
        std::bind_front(&LevelQuoteModel::on_properties_update, this));
    }

    int get_size() const {
      return static_cast<int>(m_levels.size());
    }

    int get_level(int index) const {
      if(index < 0 || index > get_size()) {
        throw std::out_of_range("The index is out of range.");
      }
      return m_levels[index];
    }

    int get_max_level() const {
      return static_cast<int>(m_properties->get().m_level_properties.m_color_scheme.size());
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

    optional<int> estimate_next_level(int level, int index) const {
      if(index + 1 >= m_table->get_row_size()) {
        return none;
      }
      if(level < get_max_level() &&
          get_price(*m_table, index) != get_price(*m_table, index + 1)) {
        ++level;
      }
      return level;
    }

    void highlight(QWidget& widget, int row) {
      if(is_order(get_mpid(*m_table, row)) && m_properties->get().m_highlight_properties.m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
        return;
      }
      match(widget, m_indicators[m_levels[row] - 1]);
    }

    void update_levels(int index) {
      if(index < 0 || index >= static_cast<int>(m_levels.size())) {
        return;
      }
      auto max_level = get_max_level();
      if(index == 0) {
        m_levels[index++] = 1;
      }
      auto level = m_levels[index - 1];
      for(auto i = index; i < m_table->get_row_size(); ++i) {
        if(level < max_level &&
            get_price(*m_table, i) != get_price(*m_table, i - 1)) {
          ++level;
        }
        m_levels[i] = level;
      }
    }

    void unmatch_row_levels(int index) {
      for(auto i = index; i < m_row_observer->get_size(); ++i) {
        if(is_order(get_mpid(*m_table, i)) && m_properties->get().m_highlight_properties.m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
          continue;
        }
        if(auto row = m_row_observer->get(i)) {
          unmatch(*row, m_indicators[m_levels[i] - 1]);
        }
      }
    }

    void match_row_levels(int index) {
      for(auto i = index; i < m_table->get_row_size(); ++i) {
        if(is_order(get_mpid(*m_table, i)) && m_properties->get().m_highlight_properties.m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
          continue;
        }
        if(auto row = m_row_observer->get(i)) {
          match(*row, m_indicators[m_levels[i] - 1]);
        }
      }
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          auto level = estimate_level(operation.m_index);
          auto next_level = estimate_next_level(level, operation.m_index);
          auto need_update = next_level && *next_level != level;
          m_levels.insert(m_levels.begin() + operation.m_index, level);
          if(need_update) {
            unmatch_row_levels(operation.m_index + 1);
            update_levels(operation.m_index + 1);
            match_row_levels(operation.m_index + 1);
          }
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_levels.erase(m_levels.begin() + operation.m_index);
          auto need_update = [&] {
            if(operation.m_index == 0) {
              return m_levels[0] != 1;
            }
            return m_levels[operation.m_index] != m_levels[operation.m_index - 1] + 1;
          }();
          if(need_update) {
            unmatch_row_levels(operation.m_index);
            update_levels(operation.m_index);
            match_row_levels(operation.m_index);
          }
        });
    }

    void on_properties_update(const BookViewProperties& properties) {
      if(m_color_scheme.size() < properties.m_level_properties.m_color_scheme.size()) {
        auto updated_index = [&] () -> boost::optional<int> {
          if(!m_indicators.empty()) {
            auto i = std::find(m_levels.begin(), m_levels.end(),
              m_indicators.back().get_data());
            if(i != m_levels.end()) {
              unmatch_row_levels(std::distance(m_levels.begin(), i));
              return std::distance(m_levels.begin(), i);
            }
          }
          return none;
        }();
        for(auto i = m_indicators.size(); i < properties.m_level_properties.m_color_scheme.size();
            ++i) {
          auto indicator = LevelIndicator(i + 1);
          m_indicators.push_back(indicator);
        }
        if(updated_index) {
          update_levels(*updated_index);
          match_row_levels(*updated_index);
        }
      } else if(m_color_scheme.size() > properties.m_level_properties.m_color_scheme.size()) {
        auto i = std::find(m_levels.begin(), m_levels.end(), get_max_level());
        if(i != m_levels.end()) {
          unmatch_row_levels(std::distance(m_levels.begin(), i));
        }
        for(auto i = properties.m_level_properties.m_color_scheme.size(); i < m_indicators.size(); ++i) {
          m_indicators.erase(m_indicators.begin() + i);
        }
        if(i != m_levels.end()) {
          auto index = std::distance(m_levels.begin(), i);
          update_levels(index);
          match_row_levels(index);
        }
      }
      m_color_scheme = properties.m_level_properties.m_color_scheme;
      if(m_order_visibility != properties.m_highlight_properties.m_order_visibility) {
        if(properties.m_highlight_properties.m_order_visibility == BookViewHighlightProperties::OrderVisibility::VISIBLE) {
          for(auto i = 0; i < m_table->get_row_size(); ++i) {
            if(is_order(get_mpid(*m_table, i))) {
              if(auto row = m_row_observer->get(i)) {
                match(*row, m_indicators[m_levels[i] - 1]);
              }
            }
          }
        } else {
          for(auto i = 0; i < m_table->get_row_size(); ++i) {
            if(is_order(get_mpid(*m_table, i))) {
              if(auto row = m_row_observer->get(i)) {
                unmatch(*row, m_indicators[m_levels[i] - 1]);
              }
            }
          }
        }

        m_order_visibility = properties.m_highlight_properties.m_order_visibility;
      }
    }
  };

  struct MarketHighlightModel {
    struct MarketRow {
      QWidget* m_row;
      Money m_price;
      Quantity m_size;
    };
    std::shared_ptr<TableModel> m_quote_table;
    std::shared_ptr<RowObserver> m_row_observer;
    std::shared_ptr<HighlightPropertiesModel> m_highlight_properties;
    MarketDatabase m_markets;
    std::unordered_map<std::string, std::unique_ptr<MarketHighlightIndicator>>
      m_indicators;
    std::unordered_map<std::string, MarketRow> m_highlight_tops;
    std::vector<BookViewHighlightProperties::MarketHighlight>
      m_market_highlights;
    connection m_row_connection;
    connection m_operation_connection;
    connection m_properties_connection;

    MarketHighlightModel(std::shared_ptr<TableModel> quote_table,
        std::shared_ptr<RowObserver> row_observer,
        std::shared_ptr<HighlightPropertiesModel> highlight_properties,
        const MarketDatabase& markets)
          : m_quote_table(std::move(quote_table)),
            m_row_observer(std::move(row_observer)),
            m_highlight_properties(std::move(highlight_properties)),
            m_markets(markets) {
      on_properties_update(m_highlight_properties->get());
      m_row_connection = m_row_observer->m_rows.connect_operation_signal(
        std::bind_front(&MarketHighlightModel::on_row_operation_update, this));
      m_operation_connection = m_quote_table->connect_operation_signal(
        std::bind_front(&MarketHighlightModel::on_table_operation_update, this));
      m_properties_connection = m_highlight_properties->connect_update_signal(
        std::bind_front(&MarketHighlightModel::on_properties_update, this));
    }

    void highlight(QWidget& widget, const std::string& mpid, const Money& price, const Quantity& size) {
      if(auto level = get_highlight_level(mpid)) {
        match_market_highlight(widget, mpid, price, size, *level);
      }
    }

    optional<BookViewHighlightProperties::MarketHighlightLevel>
        get_highlight_level(const std::string& mpid) const {
      for(auto& highlight : m_highlight_properties->get().m_market_highlights) {
        if(m_markets.FromCode(highlight.m_market).m_displayName == mpid) {
          return highlight.m_level;
        }
      }
      return none;
    }

    void match_market_highlight(QWidget& widget, const std::string& mpid,
        const Money& price, const Quantity& size,
        BookViewHighlightProperties::MarketHighlightLevel highlight_level) {
      if(!m_indicators.contains(mpid)) {
        return;
      }
      if(highlight_level == BookViewHighlightProperties::MarketHighlightLevel::ALL) {
        match(widget, *m_indicators[mpid]);
      } else {
        auto i = m_highlight_tops.find(mpid);
        if(i != m_highlight_tops.end() && i->second.m_price == price &&
            i->second.m_size == size) {
          match(widget, *m_indicators[mpid]);
        }
      }
    }

    void on_row_operation_update(const ArrayListModel<TextBox*>::Operation& operation) {
      //visit(operation,
      //  [&] (const ArrayListModel<TextBox*>::PreRemoveOperation& operation) {
      //    auto& mpid = get_mpid(*m_quote_table, operation.m_index);
      //    if(auto i = m_highlight_tops.find(mpid); i != m_highlight_tops.end()) {
      //      if(i->second.m_row == m_row_observer->get(operation.m_index)) {
      //        auto is_found = false;
      //        for(auto j = operation.m_index + 1; j < m_quote_table->get_row_size(); ++j) {
      //          auto& next_mpid = get_mpid(*m_quote_table, j);
      //          if(next_mpid == mpid) {
      //            is_found = true;
      //            i->second.m_row = m_row_observer->get(j);
      //            i->second.m_price = get_price(*m_quote_table, j);
      //            i->second.m_size = get_size(*m_quote_table, j);
      //            break;
      //          }
      //        }
      //        if(!is_found) {
      //          m_highlight_tops.erase(i);
      //        }
      //      }
      //    }
      //  },
        //[&] (const ArrayListModel<TextBox*>::UpdateOperation& operation) {
        //  auto& mpid = get_mpid(*m_quote_table, operation.m_index);
        //  auto& price = get_price(*m_quote_table, operation.m_index);
        //  auto& size = get_size(*m_quote_table, operation.m_index);
        //  if(auto i = m_highlight_tops.find(mpid); i != m_highlight_tops.end()) {
        //    if(i->second.m_price == price && i->second.m_size == size) {
        //      i->second.m_row = m_row_observer->get(operation.m_index);
        //    }
        //  }
        //});
    }

    void on_table_operation_update(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          auto& size = get_size(*m_quote_table, operation.m_index);
          if(auto i = m_highlight_tops.find(mpid); i != m_highlight_tops.end()) {
            if(i->second.m_price < price || (i->second.m_price == price && i->second.m_size < size)) {
              if(auto level = get_highlight_level(mpid); level &&
                *level == BookViewHighlightProperties::MarketHighlightLevel::TOP) {
                for(auto j = operation.m_index + 1; j < m_quote_table->get_row_size(); ++j) {
                  if(get_mpid(*m_quote_table, j) == mpid && i->second.m_price == get_price(*m_quote_table, j) &&
                    i->second.m_size == get_size(*m_quote_table, j) && m_row_observer->get(j)) {
                    unmatch(*m_row_observer->get(j), *m_indicators[mpid]);
                    break;
                  }
                }
              }
              i->second.m_row = nullptr;
              i->second.m_price = price;
              i->second.m_size = size;
            }
          } else {
            auto& quote = m_highlight_tops[mpid];
            quote.m_row = nullptr;
            quote.m_price = price;
            quote.m_size = size;
          }
        },
        [&] (const TableModel::PreRemoveOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          auto& size = get_size(*m_quote_table, operation.m_index);
          if(auto i = m_highlight_tops.find(mpid); i != m_highlight_tops.end()) {
            if(i->second.m_price == price && i->second.m_size == size) {
              auto j = operation.m_index + 1;
              for(; j < m_quote_table->get_row_size(); ++j) {
                if(get_mpid(*m_quote_table, j) == mpid) {
                  i->second.m_price = get_price(*m_quote_table, j);
                  i->second.m_size = get_size(*m_quote_table, j);
                  break;
                }
              }
              if(j != m_quote_table->get_row_size()) {
                if(auto level = get_highlight_level(mpid); level &&
                    *level == BookViewHighlightProperties::MarketHighlightLevel::TOP) {
                  match_market_highlight(*m_row_observer->get(j), mpid, i->second.m_price, i->second.m_size, *level);
                }
              } else {
                m_highlight_tops.erase(i);
              }
            }
          }
        },
        [&] (const TableModel::UpdateOperation& operation) {
        if(operation.m_column == static_cast<int>(BookViewColumns::SIZE)) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_row);
          auto& price = get_price(*m_quote_table, operation.m_row);
          if(auto i = m_highlight_tops.find(mpid); i != m_highlight_tops.end()) {
            if(i->second.m_price == price) {
              i->second.m_size = get_size(*m_quote_table, operation.m_row);
            }
          }
        }
      });
    }

    void on_properties_update(const BookViewHighlightProperties& properties) {
      if(m_market_highlights != properties.m_market_highlights) {
        for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
          auto& mpid = get_mpid(*m_quote_table, i);
          if(m_row_observer->get(i) && m_indicators.contains(mpid)) {
            unmatch(*m_row_observer->get(i), *m_indicators[mpid]);
          }
        }
        m_indicators.clear();
        auto highlight_levels = std::unordered_map<std::string,
          BookViewHighlightProperties::MarketHighlightLevel>();
        for(auto& highlight : properties.m_market_highlights) {
          auto& name = m_markets.FromCode(highlight.m_market).m_displayName;
          m_indicators[name] = std::make_unique<MarketHighlightIndicator>(name);
          highlight_levels[name] = highlight.m_level;
        }
        for(auto i = 0; i < m_quote_table->get_row_size(); ++i) {
          auto& mpid = get_mpid(*m_quote_table, i);
          auto& price = get_price(*m_quote_table, i);
          auto& size = get_size(*m_quote_table, i);
          if(auto row = m_row_observer->get(i)) {
            match_market_highlight(*row, mpid, price, size, highlight_levels[mpid]);
          }
        }
        m_market_highlights = properties.m_market_highlights;
      }
    }
  };
  
  struct OrderHighlightModel {
    struct OrderEntry {
      QWidget* m_row;
      OrderStatus m_status;
    };
    std::shared_ptr<TableModel> m_quote_table;
    std::shared_ptr<ListModel<BookViewModel::UserOrder>> m_orders;
    std::shared_ptr<RowObserver> m_row_observer;
    std::shared_ptr<HighlightPropertiesModel> m_properties;
    std::unordered_map<OrderKey, OrderEntry, OrderKeyHash> m_order_entries;
    BookViewHighlightProperties::OrderVisibility m_order_visibility;
    connection m_row_connection;
    connection m_table_operation_connection;
    connection m_order_connection;
    connection m_properties_connection;

    OrderHighlightModel(std::shared_ptr<TableModel> quote_table,
      std::shared_ptr<ListModel<BookViewModel::UserOrder>> orders,
      std::shared_ptr<RowObserver> row_observer,
      std::shared_ptr<HighlightPropertiesModel> properties)
        : m_quote_table(std::move(quote_table)),
          m_orders(std::move(orders)),
          m_row_observer(std::move(row_observer)),
          m_properties(std::move(properties)) {
      on_properties_update(m_properties->get());
      m_row_connection = m_row_observer->m_rows.connect_operation_signal(
        std::bind_front(&OrderHighlightModel::on_row_operation_update, this));
      m_table_operation_connection = m_quote_table->connect_operation_signal(
        std::bind_front(&OrderHighlightModel::on_table_operation, this));
      m_order_connection = m_orders->connect_operation_signal(
        std::bind_front(&OrderHighlightModel::on_order_operation, this));
      m_properties_connection = m_properties->connect_update_signal(
        std::bind_front(&OrderHighlightModel::on_properties_update, this));
    }

    void highlight(QWidget& widget, const std::string& mpid, const Money& price) {
      if(mpid.front() == '@') {
        if(m_properties->get().m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
          auto key = OrderKey(mpid, price);
          if(auto i = m_order_entries.find(key); i != m_order_entries.end()) {
            match(*i->second.m_row, OrderIndicator());
            match_order(i->second);
          }
        }
      }
    }

    void unmatch_order(const OrderEntry& entry) {
      if(entry.m_status == OrderStatus::CANCELED) {
        unmatch(*entry.m_row, CanceledOrderIndicator());
      } else if(entry.m_status == OrderStatus::FILLED) {
        unmatch(*entry.m_row, FilledOrderIndicator());
      } else if(entry.m_status == OrderStatus::REJECTED) {
        unmatch(*entry.m_row, RejectedOrderIndicator());
      }
    }

    void unmatch_order(const OrderKey& key) {
      auto i = m_order_entries.find(key);
      if(i != m_order_entries.end() && i->second.m_row) {
        unmatch_order(i->second);
      }
    }

    void match_order(const OrderEntry& entry) {
      if(entry.m_status == OrderStatus::CANCELED) {
        match(*entry.m_row, CanceledOrderIndicator());
      } else if(entry.m_status == OrderStatus::FILLED) {
        match(*entry.m_row, FilledOrderIndicator());
      } else if(entry.m_status == OrderStatus::REJECTED) {
        match(*entry.m_row, RejectedOrderIndicator());
      }
    }

    void match_order(const OrderKey& key) {
      auto i = m_order_entries.find(key);
      if(i != m_order_entries.end() && i->second.m_row) {
        match_order(i->second);
      }
    }

    void on_row_operation_update(const ArrayListModel<TextBox*>::Operation& operation) {
      visit(operation,
        [&] (const ArrayListModel<TextBox*>::UpdateOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          if(auto i = m_order_entries.find(OrderKey(mpid, price)); i != m_order_entries.end()) {
            i->second.m_row = m_row_observer->get(operation.m_index);
          }
        });
    }

    void on_order_operation(const ListModel<BookViewModel::UserOrder>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<BookViewModel::UserOrder>::AddOperation& operation) {
        },
        [&] (const ListModel<BookViewModel::UserOrder>::RemoveOperation& operation) {
        },
        [&] (const ListModel<BookViewModel::UserOrder>::UpdateOperation& operation) {
          auto& order = m_orders->get(operation.m_index);
          auto key = OrderKey('@' + order.m_destination, order.m_price);
          if(auto i = m_order_entries.find(key); i != m_order_entries.end()) {
            if(i->second.m_row) {
              unmatch_order(i->second);
            }
            i->second.m_status = order.m_status;
            if(m_properties->get().m_order_visibility ==
                BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
              if(i->second.m_row) {
                match_order(i->second);
              }
            }
          }
        });
    }

    void on_table_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          if(mpid.front() != '@') {
            return;
          }
          auto& price = get_price(*m_quote_table, operation.m_index);
          auto key = OrderKey(mpid, price);
          if(auto i = m_order_entries.find(key); i != m_order_entries.end()) {
            if(i->second.m_row) {
              unmatch_order(i->second);
            }
            i->second.m_status = OrderStatus::PENDING_NEW;
          } else {
            m_order_entries[key] = OrderEntry{nullptr, OrderStatus::PENDING_NEW};
          }
          if(m_properties->get().m_order_visibility ==
              BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
            match_order(key);
          }
        },
        [&] (const TableModel::RemoveOperation& operation) {
          auto& mpid = get_mpid(*m_quote_table, operation.m_index);
          auto& price = get_price(*m_quote_table, operation.m_index);
          m_order_entries.erase(OrderKey(mpid, price));
        });
    }

    void on_properties_update(const BookViewHighlightProperties& properties) {
      if(m_order_visibility != properties.m_order_visibility) {
        if(properties.m_order_visibility ==
          BookViewHighlightProperties::OrderVisibility::VISIBLE) {
          for(auto& entry : m_order_entries) {
            if(entry.second.m_row) {
              unmatch(*entry.second.m_row, OrderIndicator());
              unmatch_order(entry.second);
            }
          }
        } else if(properties.m_order_visibility ==
          BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
          for(auto& entry : m_order_entries) {
            if(entry.second.m_row) {
              match(*entry.second.m_row, OrderIndicator());
              match_order(entry.second);
            }
          }
        }
        m_order_visibility = properties.m_order_visibility;
      }
    }
  };

  QWidget* item_builder(std::shared_ptr<RowObserver> row_observer,
    std::shared_ptr<LevelQuoteModel> level_quote_model,
    std::shared_ptr<MarketHighlightModel> market_highlight_model,
    std::shared_ptr<OrderHighlightModel> order_highlight_model,
    const std::shared_ptr<TableModel>& table,
    int row, int column) {
    auto column_id = static_cast<BookViewColumns>(column);
    auto cell = [&] {
      if(column_id == BookViewColumns::MPID) {
        auto mpid_cell = make_label(QString::fromStdString(table->get<std::string>(row, column)));
        update_style(*mpid_cell, [] (auto& style) {
          style.get(Any()).
            set(PaddingLeft(scale_width(6))).
            set(PaddingRight(scale_width(2)));
        });
        return mpid_cell;
      } else if(column_id == BookViewColumns::PRICE) {
        auto money_cell = make_label(to_text(table->get<Money>(row, column)));
        update_style(*money_cell, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(horizontal_padding(scale_width(2)));
        });
        return money_cell;
      } else if(column_id == BookViewColumns::SIZE) {
        auto quantity_cell = make_label(to_text(table->get<Quantity>(row, column)));
        update_style(*quantity_cell, [] (auto& style) {
          style.get(Any()).
            set(TextAlign(Qt::AlignRight | Qt::AlignVCenter)).
            set(PaddingLeft(scale_width(2))).
            set(PaddingRight(scale_width(6)));
        });
        return quantity_cell;
      }
      return make_label("");
    }();
    if(column == static_cast<int>(BookViewColumns::SIZE)) {
      row_observer->set(row, cell);
      level_quote_model->highlight(*cell, row);
      market_highlight_model->highlight(*cell, table->get<std::string>(row, 0),
        table->get<Money>(row, 1), table->get<Quantity>(row, 2));
      order_highlight_model->highlight(*cell, table->get<std::string>(row, 0),
        table->get<Money>(row, 1));
    }
    return cell;
  }

  struct BookViewTableViewObserver : QObject {
    TableView* m_table_view;
    std::shared_ptr<ListModel<Nexus::BookQuote>> m_book_quotes;
    std::shared_ptr<ListModel<BookViewModel::UserOrder>> m_orders;
    std::shared_ptr<BookViewPropertiesModel> m_properties;
    std::shared_ptr<LevelQuoteModel> m_level_quote_model;
    std::shared_ptr<MarketHighlightModel> m_market_highlight_model;
    std::shared_ptr<OrderHighlightModel> m_order_highlight_model;
    std::shared_ptr<LevelPropertiesModel> m_level_properties;
    std::shared_ptr<HighlightPropertiesModel> m_highlight_properties;
    connection m_level_properties_connection;
    connection m_highlight_properties_connection;

    BookViewTableViewObserver(TableView& table_view,
       std::shared_ptr<ListModel<Nexus::BookQuote>> book_quotes,
       std::shared_ptr<ListModel<BookViewModel::UserOrder>> orders,
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
        m_level_properties(make_field_value_model(m_properties, &BookViewProperties::m_level_properties)),
        m_highlight_properties(make_field_value_model(m_properties, &BookViewProperties::m_highlight_properties)) {
      m_table_view->installEventFilter(this);
      m_table_view->get_body().installEventFilter(this);
      on_level_properties_update(m_level_properties->get());
      on_highlight_properties_update(m_highlight_properties->get());
      m_level_properties_connection = m_level_properties->connect_update_signal(
        std::bind_front(&BookViewTableViewObserver::on_level_properties_update, this));
      m_highlight_properties_connection = m_highlight_properties->connect_update_signal(
        std::bind_front(&BookViewTableViewObserver::on_highlight_properties_update, this));
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Resize && watched == m_table_view) {
        auto& resize_event = *static_cast<QResizeEvent*>(event);
        auto column_width = resize_event.size().width() / 3;
        m_table_view->get_header().get_widths()->set(0, column_width);
        m_table_view->get_header().get_widths()->set(1, column_width);
      } else if(event->type() == QEvent::Show && watched == m_table_view) {
        //update_level_styles();
        //m_level_quote_model.update_level();
        //update_styles();
      } else if(event->type() == QEvent::ChildAdded && watched == &m_table_view->get_body()) {
        //auto& child_event = *static_cast<QChildEvent*>(event);
        //qDebug() << "Child Added";
        //if(child_event.child()->isWidgetType()) {
        //  auto child = child_event.child();
        //  child->installEventFilter(this);
        //  qDebug() << "Created row:" << child;
        //  m_rows.insert(static_cast<QWidget*>(child));
        //}
      }
      return QObject::eventFilter(watched, event);
    }

    void on_level_properties_update(const BookViewLevelProperties& properties) {
      if(properties.m_is_grid_enabled) {
        match(*m_table_view, ShowGrid());
      } else {
        unmatch(*m_table_view, ShowGrid());
      }
      update_style(*m_table_view, [&] (auto& style) {
        auto item_selector = Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
        style.get(item_selector > is_a<TextBox>()).
          set(Font(properties.m_font));
        for(auto i = 0; i < std::ssize(m_level_quote_model->m_indicators); ++i) {
          style.get(item_selector > (m_level_quote_model->m_indicators[i] < is_a<TableItem>() < Row())).
            set(BackgroundColor(properties.m_color_scheme[i]));
            //style.get(item_selector > (m_level_quote_model->m_indicators[i] < is_a<TableItem>() < Row()) > is_a<TableItem>() > is_a<TextBox>()).
            //set(TextColor(get_text_color(properties.m_color_scheme[i])));
        }
      });
      m_table_view->get_body().update();
    }

    void on_highlight_properties_update(const BookViewHighlightProperties& properties) {
      update_style(*m_table_view, [&] (auto& style) {
        auto item_selector = Any() > is_a<TableBody>() > Row() > is_a<TableItem>();
        for(auto& highlight : properties.m_market_highlights) {
          auto name = m_market_highlight_model->m_markets.FromCode(highlight.m_market).m_displayName;
          if(m_market_highlight_model->m_indicators.contains(name)) {
            style.get(item_selector > (*m_market_highlight_model->m_indicators.at(name) < is_a<TableItem>() < Row())).
              set(BackgroundColor(highlight.m_color.m_background_color));
            style.get(item_selector > (*m_market_highlight_model->m_indicators.at(name) < is_a<TableItem>() < Row()) > is_a<TableItem>() > is_a<TextBox>()).
              set(TextColor(highlight.m_color.m_text_color));
          }
        }
        if(properties.m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED) {
          auto active_order_highlight = properties.m_order_highlights[static_cast<int>(BookViewHighlightProperties::OrderHighlightState::ACTIVE)];
          apply_highlight_style(style, item_selector, OrderIndicator(), active_order_highlight);
          auto filled_order_highlight = properties.m_order_highlights[static_cast<int>(BookViewHighlightProperties::OrderHighlightState::FILLED)];
          apply_highlight_animation_style(style, item_selector, FilledOrderIndicator(), filled_order_highlight);
          auto canceled_order_highlight = properties.m_order_highlights[static_cast<int>(BookViewHighlightProperties::OrderHighlightState::CANCELED)];
          apply_highlight_animation_style(style, item_selector, CanceledOrderIndicator(), canceled_order_highlight);
          auto rejected_order_highlight = properties.m_order_highlights[static_cast<int>(BookViewHighlightProperties::OrderHighlightState::REJECTED)];
          apply_highlight_animation_style(style, item_selector, RejectedOrderIndicator(), rejected_order_highlight);
        }
      });
      m_table_view->get_body().update();
    }

  void apply_highlight_style(StyleSheet& style, const Selector& selector, const Selector& highlight_selector,
      const HighlightColor& highlight_color) {
      style.get(selector > (highlight_selector < is_a<TableItem>() < Row())).
        set(BackgroundColor(highlight_color.m_background_color));
      style.get(selector > (highlight_selector < is_a<TableItem>() < Row()) > is_a<TableItem>() > is_a<TextBox>()).
        set(TextColor(highlight_color.m_text_color));
    }

  void apply_highlight_animation_style(StyleSheet& style, const Selector& selector, const Selector& highlight_selector,
      const HighlightColor& highlight_color) {
    style.get(selector > (highlight_selector < is_a<TableItem>() < Row())).
      set(BackgroundColor(chain(timeout(highlight_color.m_background_color, milliseconds(100)),
        linear(highlight_color.m_background_color, revert, milliseconds(300)))));
    style.get(selector > (highlight_selector < is_a<TableItem>() < Row()) > is_a<TableItem>() > is_a<TextBox>()).
      set(TextColor(chain(timeout(highlight_color.m_text_color, milliseconds(400)), revert)));
   }
  };

struct OrderFilteredListModel : FilteredListModel<BookQuote> {
  std::shared_ptr<HighlightPropertiesModel> m_highlight_properties;
  BookViewHighlightProperties::OrderVisibility m_order_visibility;
  connection m_connection;

  OrderFilteredListModel(std::shared_ptr<ListModel<BookQuote>> source,
    std::shared_ptr<HighlightPropertiesModel> highlight_properties)
    : FilteredListModel<BookQuote>(std::move(source), nullptr),
      m_highlight_properties(std::move(highlight_properties)),
      m_order_visibility(BookViewHighlightProperties::OrderVisibility::VISIBLE),
      m_connection(m_highlight_properties->connect_update_signal(
        std::bind_front(&OrderFilteredListModel::on_properties, this))) {
    on_properties(m_highlight_properties->get());
  }

  bool filter(const ListModel<BookQuote>& list, int index) {
    if(m_highlight_properties->get().m_order_visibility == BookViewHighlightProperties::OrderVisibility::HIDDEN) {
      return list.get(index).m_mpid.front() == '@';
    }
    return false;
  }

  void on_properties(const BookViewHighlightProperties& properties) {
    if(m_order_visibility != properties.m_order_visibility) {
      set_filter(std::bind_front(&OrderFilteredListModel::filter, this));
      m_order_visibility = properties.m_order_visibility;
    }
  }
};
}

TableView* Spire::make_book_view_table_view(std::shared_ptr<BookViewModel> model,
    std::shared_ptr<BookViewPropertiesModel> properties, Nexus::Side side,
    const Nexus::MarketDatabase& markets, QWidget* parent) {
  auto [book_quotes, orders] = [&] {
    if(side == Side::BID) {
      return std::tuple(model->get_bids(), model->get_bid_orders());
    }
    return std::tuple(model->get_asks(), model->get_ask_orders());
  }();
  auto column_orders = std::vector<SortedTableModel::ColumnOrder>{{1, SortedTableModel::Ordering::DESCENDING}};
  auto table = std::make_shared<SortedTableModel>(make_book_view_table_model(std::make_shared<OrderFilteredListModel>(book_quotes,
    make_field_value_model(properties, &BookViewProperties::m_highlight_properties))), column_orders);
  auto row_observer = std::make_shared<RowObserver>(table);
  auto level_quote_model = std::make_shared<LevelQuoteModel>(table, row_observer, properties);
  auto market_highlight_model = std::make_shared<MarketHighlightModel>(table,
    row_observer, make_field_value_model(properties, &BookViewProperties::m_highlight_properties), markets);
  auto order_highlight_model = std::make_shared<OrderHighlightModel>(table,
    orders, row_observer,
    make_field_value_model(properties, &BookViewProperties::m_highlight_properties));
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_item_builder(std::bind_front(item_builder, row_observer, level_quote_model, market_highlight_model, order_highlight_model)).make();
  table_view->get_header().setVisible(false);
  make_menu(*table_view, properties);
  auto observer = new BookViewTableViewObserver(*table_view, book_quotes, orders, properties, level_quote_model, market_highlight_model, order_highlight_model, table_view);
  update_style(*table_view, [=] (auto& style) {
    auto body_selector = Any() > is_a<TableBody>();
    auto item_selector = body_selector > Row() > is_a<TableItem>();
    style.get(item_selector > is_a<TextBox>()).
      set(Font(properties->get().m_level_properties.m_font)).
      //set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_width(1.5)));
    style.get(body_selector).
      set(HorizontalSpacing(0)).
      set(VerticalSpacing(0)).
      set(grid_color(QColor(0xE0E0E0)));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > CurrentRow()).
      set(BackgroundColor(QColor(0x8D78EC))).
      set(TextColor(QColor(0xFFFFFF))).
      set(border_color(QColor(0x4B23A0)));
    style.get(body_selector > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    //style.get(body_selector > Row()).
    //  set(border(scale_width(1), QColor(Qt::transparent))).
    //  set(horizontal_padding(scale_width(4)));
    style.get(ShowGrid() > is_a<TableBody>()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_height(1)));
  });

  //table_view->get_selection()->get_row_selection()->connect_operation_signal(
  //  [] (const ListModel<int>::Operation& operation) {
  //  visit(operation,
  //    [&] (const ListModel<int>::AddOperation& operation) {
  //      auto& selection =
  //        m_selection_controller.get_selection()->get_row_selection();
  //      if(auto row = find_row(selection->get(operation.m_index))) {
  //        match(*row, Selected());
  //      }
  //    },
  //    [&] (const ListModel<int>::UpdateOperation& operation) {
  //      if(auto previous = find_row(operation.get_previous())) {
  //        unmatch(*previous, Selected());
  //      }
  //      if(auto row = find_row(operation.get_value())) {
  //        match(*row, Selected());
  //      }
  //    });
  //  });
  return table_view;
}
