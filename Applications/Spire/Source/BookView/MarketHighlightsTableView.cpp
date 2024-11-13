#include "Spire/BookView/MarketHighlightsTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using MarketHighlightLevel =
    BookViewHighlightProperties::MarketHighlightLevel;
  using MarketHighlight = BookViewHighlightProperties::MarketHighlight;
  using EmptyMarket = StateSelector<void, struct EmptyMarketSeletorTag>;

  static const auto COLUMN_COUNT = 3;
  static const auto MARKET_COLUMN = 0;
  static const auto LEVEL_COLUMN = 1;
  static const auto COLOR_COLUMN = 2;

  const auto& NONE_MARKET() {
    static auto NONE = MarketCode();
    return NONE;
  }

  const auto& NONE_LEVEL() {
    static auto NONE = MarketHighlightLevel::TOP;
    return NONE;
  }

  const auto& NONE_COLOR() {
    static auto NONE = HighlightColor(Qt::transparent, Qt::transparent);
    return NONE;
  }

  const auto& DEFAULT_MARKET_HIGHLIGHTS() {
    static const auto default_market_highlights = std::vector<HighlightColor>{
      {QColor(0xFFFFC4), QColor(0x834A2D)},
      {QColor(0xDDF9FF), QColor(0x003698)},
      {QColor(0xFFECFF), QColor(0x76008A)},
      {QColor(0xF4F4FF), QColor(0x4C00DA)},
      {QColor(0xFFE6C9), QColor(0xA00000)},
      {QColor(0xFEEE7F), QColor(0x630000)},
      {QColor(0x92CFE9), QColor(0x0000C6)},
      {QColor(0xF1A6F1), QColor(0x770088)},
      {QColor(0xC1BAFF), QColor(0x4F00D7)},
      {QColor(0xFFBF76), QColor(0xA00000)},
      {QColor(0x8A6729), QColor(0xFFFFFF)},
      {QColor(0x246FBC), QColor(0xFFFFFF)},
      {QColor(0xA937B2), QColor(0xFFFFFF)},
      {QColor(0x6F53C5), QColor(0xFFFFFF)},
      {QColor(0xAE4D1F), QColor(0xFFFFFF)},
      {QColor(0x4D2A00), QColor(0xFFFFFF)},
      {QColor(0x21148C), QColor(0xFFFFFF)},
      {QColor(0x5F006F), QColor(0xFFFFFF)},
      {QColor(0x361976), QColor(0xFFFFFF)},
      {QColor(0x680000), QColor(0xFFFFFF)}
    };
    return default_market_highlights;
  }

  auto apply_table_view_style(StyleSheet& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    auto body_selector = Any() > is_a<TableBody>();
    auto item_selector = body_selector > Row() > is_a<TableItem>();
    auto input_box_selector = item_selector > Any() > Any();
    style.get(Any()).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0)));
    style.get(input_box_selector > Any() > ReadOnly() >
        (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(scale_width(8)));
    style.get(input_box_selector > is_a<HighlightBox>()).
      set(vertical_padding(scale_height(5))).
      set(horizontal_padding(scale_width(8)));
    style.get(input_box_selector > is_a<HighlightBox>() > is_a<TextBox>()).
      set(Font(font));
    style.get(item_selector > EmptyMarket() << Row() >
        is_a<TableItem>() > DeleteButton()).
      set(Visibility(Visibility::INVISIBLE));
  }

  const auto& to_text(MarketHighlightLevel level) {
    if(level == MarketHighlightLevel::TOP) {
      static const auto value = QObject::tr("Top Level");
      return value;
    } else {
      static const auto value = QObject::tr("All Levels");
      return value;
    }
  }

  AnyRef extract(MarketHighlight& highlight, int index) {
    if(index == MARKET_COLUMN) {
      return highlight.m_market;
    } else if(index == LEVEL_COLUMN) {
      return highlight.m_level;
    } else {
      return highlight.m_color;
    }
  }

  auto setup_level_box() {
    static auto settings = [] {
      auto settings = EnumBox<MarketHighlightLevel>::Settings(to_text,
        [] (const auto& value) {
          return make_label(to_text(value));
        });
      auto cases = std::make_shared<ArrayListModel<MarketHighlightLevel>>();
      cases->push(MarketHighlightLevel::TOP);
      cases->push(MarketHighlightLevel::ALL);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }

  struct MarketHighlightListToTableModel : ListToTableModel<MarketHighlight> {
    std::shared_ptr<ListModel<MarketHighlight>> m_list;

    MarketHighlightListToTableModel(
      std::shared_ptr<ListModel<MarketHighlight>> list, int columns,
      Accessor accessor)
      : ListToTableModel(list, columns, accessor),
        m_list(std::move(list)) {}

    QValidator::State push(const MarketHighlight& value) {
      return m_list->push(value);
    }
  };

  struct ExpandableTableModel : TableModel {
    std::shared_ptr<MarketHighlightListToTableModel> m_source;
    scoped_connection m_connection;
    TableModelTransactionLog m_transaction;

    explicit ExpandableTableModel(
        std::shared_ptr<MarketHighlightListToTableModel> source)
        : m_source(std::move(source)) {
      m_connection = m_source->connect_operation_signal(
        std::bind_front(&ExpandableTableModel::on_operation, this));
    }

    int get_row_size() const {
      return m_source->get_row_size() + 1;
    }

    int get_column_size() const {
      return COLUMN_COUNT;
    }

    AnyRef at(int row, int column) const {
      if(row < m_source->get_row_size()) {
        return m_source->at(row, column);
      } else if(row == m_source->get_row_size()) {
        if(column == MARKET_COLUMN) {
          return NONE_MARKET();
        } else if(column == LEVEL_COLUMN) {
          return NONE_LEVEL();
        } else if(column == COLOR_COLUMN) {
          return NONE_COLOR();
        }
      }
      throw std::out_of_range("The row or column is out of range.");
    }

    QValidator::State set(int row, int column, const std::any& value) {
      if(row < m_source->get_row_size()) {
        return m_source->set(row, column, value);
      } else if(row == m_source->get_row_size()) {
        if(column == LEVEL_COLUMN || column == COLOR_COLUMN) {
          return QValidator::Acceptable;
        }
        auto blocker = shared_connection_block(m_connection);
        auto& highligts = DEFAULT_MARKET_HIGHLIGHTS();
        auto& highlight =
          highligts[m_source->get_row_size() % highligts.size()];
        m_source->push({std::any_cast<const MarketCode&>(value), highlight,
          BookViewHighlightProperties::MarketHighlightLevel::TOP});
        m_transaction.transact([&] {
          m_transaction.push(
            UpdateOperation(row, column, NONE_MARKET(), value));
          m_transaction.push(AddOperation(row + 1));
        });
        return QValidator::Acceptable;
      }
      throw std::out_of_range("The row is out of range.");
    }

    QValidator::State remove(int row) {
      if(row < m_source->get_row_size()) {
        return m_source->remove(row);
      } else if(row == m_source->get_row_size()) {
        return QValidator::Invalid;
      }
      throw std::out_of_range("The row is out of range.");
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      m_transaction.push(operation);
    }
  };

  template<typename T, typename M>
  struct InputBoxWrapper : QWidget {
    using InputBox = T;
    using Model = M;
    using SubmitSignal = InputBox::SubmitSignal;
    Model m_model;
    std::shared_ptr<MarketModel> m_market;
    scoped_connection m_connection;

    InputBoxWrapper(Model model, std::shared_ptr<MarketModel> market,
        QWidget* parent = nullptr)
        : m_model(std::move(model)),
        m_market(std::move(market)),
        QWidget(parent) {
      make_vbox_layout(this);
      if(m_market->get().IsEmpty()) {
        m_connection = m_market->connect_update_signal(
          std::bind_front(&InputBoxWrapper::on_market_update, this));
      } else {
        on_market_update(m_market->get());
      }
    }

    const decltype(std::declval<InputBox>().get_current())&
        get_current() const {
      if constexpr(std::is_same_v<InputBox, HighlightBox>) {
        return m_model;
      } else {
        return m_model.m_current;
      }
    }

    bool is_read_only() const {
      if(layout()->count() == 0) {
        return false;
      }
      return get_input_box()->is_read_only();
    }

    void set_read_only(bool is_read_only) {
      if(layout()->count() == 0) {
        return;
      }
      get_input_box()->set_read_only(is_read_only);
    }

    connection connect_submit_signal(const SubmitSignal::slot_type& slot) const {
      if(layout()->count() == 0) {
        return connection();
      }
      return get_input_box()->connect_submit_signal(slot);
    }

    InputBox* get_input_box() const {
      return static_cast<InputBox*>(layout()->itemAt(0)->widget());
    }

    void on_market_update(const MarketCode& market) {
      if(!market.IsEmpty() && layout()->count() == 0) {
        auto input_box = new InputBox(m_model);
        input_box->set_read_only(true);
        layout()->addWidget(input_box);
        setFocusProxy(input_box);
      }
    }
  };

  auto make_item(const MarketDatabase& markets,
    const std::shared_ptr<TableModel>& table, int row, int column) {
    if(column == MARKET_COLUMN) {
      auto market_box = make_market_box(
        make_table_value_model<MarketCode>(table, row, column), markets);
      auto item = new EditableBox(*market_box);
      if(row == table->get_row_size() - 1) {
        match(*item, EmptyMarket());
      }
      market_box->get_current()->connect_update_signal(
        [=] (const auto& market) {
          if(!market.IsEmpty() && is_match(*item, EmptyMarket())) {
            unmatch(*item, EmptyMarket());
          }
        });
      return item;
    } else if(column == LEVEL_COLUMN) {
      auto settings = setup_level_box();
      settings.m_current =
        make_table_value_model<MarketHighlightLevel>(table, row, column);
      return new EditableBox(*new InputBoxWrapper<EnumBox<MarketHighlightLevel>,
        EnumBox<MarketHighlightLevel>::Settings>(std::move(settings),
          make_table_value_model<MarketCode>(table, row, MARKET_COLUMN)));
    } else if(column == COLOR_COLUMN) {
      return new EditableBox(*new InputBoxWrapper<HighlightBox,
        std::shared_ptr<HighlightColorModel>>(
          make_table_value_model<HighlightColor>(table, row, column),
          make_table_value_model<MarketCode>(table, row, MARKET_COLUMN)));
    }
    throw std::runtime_error("Invalid column.");
  }
}

TableView* Spire::make_market_highlights_table_view(
    std::shared_ptr<MarketHighlightListModel> market_highlights,
    MarketDatabase markets, QWidget* parent) {
  auto table = std::make_shared<MarketHighlightListToTableModel>(
    market_highlights, COLUMN_COUNT, &extract);
  auto table_builder = EditableTableViewBuilder(
    std::make_shared<ExpandableTableModel>(std::move(table)));
  table_builder.add_header_item(QObject::tr("Market"), QObject::tr("Mkt"),
    TableHeaderItem::Order::NONE, TableFilter::Filter::NONE);
  table_builder.add_header_item(QObject::tr("Highlight"), QObject::tr("HL"),
    TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE);
  table_builder.add_header_item(QObject::tr("Color"), QObject::tr("Clr"),
    TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE);
  table_builder.set_item_builder(std::bind_front(make_item, markets));
  table_builder.set_comparator([=] (const AnyRef& left, int left_row,
      const AnyRef& right, int right_row, int column) {
    auto& left_market = any_cast<MarketCode>(left);
    auto& right_market = any_cast<MarketCode>(right);
    if(left_market.IsEmpty() || right_market.IsEmpty()) {
      return true;
    }
    return left_market < right_market;
  });
  auto table_view = table_builder.make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  table_view->get_header().get_widths()->set(0, scale_width(26));
  table_view->get_header().get_widths()->set(1, scale_width(92));
  table_view->get_header().get_widths()->set(2, scale_width(92));
  table_view->get_header().get_widths()->set(3, scale_width(138));
  update_style(*table_view, apply_table_view_style);
  return table_view;
}
