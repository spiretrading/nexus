#include "Spire/KeyBindings/OrderTasksPage.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QMouseEvent>
#include <QStringBuilder>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Spire/KeyBindings/OrderTasksTableViewModel.hpp"
#include "Spire/KeyBindings/OrderTasksToTableModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BOLD_LABEL_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Bold);
    font.setPixelSize(scale_width(12));
    style.get(Any()).set(text_style(font, QColor(Qt::black)));
    return style;
  }

  auto TABLE_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomColor(QColor(0xE0E0E0))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(BorderTopSize(scale_height(1)));
    style.get(Any() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get((Any() > Editing()) << Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0x88E2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    style.get(Any() > OutOfRangeRow()).set(BackgroundColor(QColor(0xFDF8DE)));
    style.get(Any() > (DraggingCurrent() && Current())).
      set(border_color(QColor(0xE0E0E0)));
    return style;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({"", "",
      TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE});
    model->push({"Name", "Name",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Region", "Region",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Destination", "Dest",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Order Type", "Ord Type",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Side", "Side",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Quantity", "Qty",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Time in Force", "TIF",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Key", "Key",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    return model;
  }

  auto make_header_widths() {
    auto widths = std::vector<int>();
    widths.push_back(scale_width(20));
    widths.push_back(scale_width(192));
    widths.push_back(scale_width(104));
    widths.push_back(scale_width(110));
    widths.push_back(scale_width(110));
    widths.push_back(scale_width(70));
    widths.push_back(scale_width(90));
    widths.push_back(scale_width(70));
    return widths;
  }

  auto make_help_text_region() {
    auto help_text_body = new QWidget();
    auto help_text_layout = make_hbox_layout(help_text_body);
    auto dash = QString(0x2013);
    help_text_layout->addWidget(make_label(QObject::tr("Allowed keys are: ")));
    auto label1 = make_label("F1" + dash + "F12");
    label1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    update_style(*label1, [] (auto& style) {
      style = BOLD_LABEL_STYLE(style);
    });
    help_text_layout->addWidget(label1);
    help_text_layout->addWidget(make_label(QObject::tr(" and ")));
    auto label2 = make_label("Ctrl, Shift, Alt  +  F1" + dash + "F12");
    label2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    update_style(*label2, [] (auto& style) {
      style = BOLD_LABEL_STYLE(style);
      });
    help_text_layout->addWidget(label2);
    help_text_layout->addWidget(make_label(QObject::tr(" and ")));
    auto label3 = make_label("Ctrl, Shift, Alt  +  0" + dash + "9");
    label3->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    update_style(*label3, [] (auto& style) {
      style = BOLD_LABEL_STYLE(style);
    });
    help_text_layout->addWidget(label3);
    auto help_text_box = new Box(help_text_body);
    help_text_box->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Preferred);
    update_style(*help_text_box, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xF5F5F5))).
        set(horizontal_padding(scale_width(8))).
        set(vertical_padding(scale_width(12)));
    });
    return help_text_box;
  }

  auto make_search_region() {
    auto search_box = new SearchBox();
    search_box->set_placeholder(QObject::tr("Search order tasks"));
    search_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto search_region = new Box(search_box);
    search_region->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Preferred);
    update_style(*search_region, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(horizontal_padding(scale_width(8))).
        set(vertical_padding(scale_height(10)));
    });
    return std::tuple(search_region, search_box);
  }

  auto to_text(const Region& region) {
    auto countries = std::set<QString>();
    for(auto& country : region.GetCountries()) {
      countries.insert(displayText(country));
    }
    auto markets = std::set<QString>();
    for(auto& market : region.GetMarkets()) {
      markets.insert(displayText(MarketToken(market)));
    }
    auto securities = std::set<QString>();
    for(auto& security : region.GetSecurities()) {
      securities.insert(displayText(security));
    }
    auto text = QString();
    for(auto& country : countries) {
      text = text % country;
    }
    for(auto& market : markets) {
      text = text % market;
    }
    for(auto& security : securities) {
      text = text % security;
    }
    return text;
  }

  bool compare_text(const QString& lhs, const QString& rhs) {
    return QString::compare(lhs, rhs, Qt::CaseInsensitive) < 0;
  }

  bool order_tasks_comparator(const AnyRef& lhs, const AnyRef& rhs) {
    if(lhs.get_type() != rhs.get_type()) {
      return false;
    } else if(!lhs.has_value() || !rhs.has_value()) {
      return false;
    } else if(lhs.get_type() == typeid(QString)) {
      return compare_text(any_cast<QString>(lhs), any_cast<QString>(rhs));
    } else if(lhs.get_type() == typeid(optional<Quantity>)) {
      auto& quantity_lhs = any_cast<optional<Quantity>>(lhs);
      auto& quantity_rhs = any_cast<optional<Quantity>>(rhs);
      if(quantity_lhs && quantity_rhs) {
        return *quantity_lhs < *quantity_rhs;
      } else if(quantity_lhs) {
        return false;
      } else if(quantity_rhs) {
        return true;
      }
    } else if(lhs.get_type() == typeid(Side)) {
      auto& side_lhs = any_cast<Side>(lhs);
      auto& side_rhs = any_cast<Side>(rhs);
      if(side_lhs != Side::NONE && side_rhs != Side::NONE) {
        return displayText(side_lhs) < displayText(side_rhs);
      } else if(side_lhs != Side::NONE) {
        return false;
      } else if(side_rhs != Side::NONE) {
        return true;
      }
    } else if(lhs.get_type() == typeid(TimeInForce)) {
      return any_cast<TimeInForce>(lhs).GetType() <
        any_cast<TimeInForce>(rhs).GetType();
    } else if(lhs.get_type() == typeid(OrderType)) {
      return any_cast<OrderType>(lhs) < any_cast<OrderType>(rhs);
    } else if(lhs.get_type() == typeid(Destination)) {
      return any_cast<Destination>(lhs) < any_cast<Destination>(rhs);
    } else if(lhs.get_type() == typeid(Region)) {
      return compare_text(to_text(any_cast<Region>(lhs)),
        to_text(any_cast<Region>(rhs)));
    } else if(lhs.get_type() == typeid(QKeySequence)) {
      return compare_text(any_cast<QKeySequence>(lhs).toString(),
        any_cast<QKeySequence>(rhs).toString());
    }
    return false;
  }

  auto build_search_text(const QString& name) {
    auto texts = std::vector<QString>();
    for(auto i = 0; i < name.length(); ++i) {
      texts.push_back(name.right(name.length() - i));
    }
    return texts;
  }

  auto build_search_text(const Region& region) {
    auto texts = std::vector<QString>();
    auto& country_database = GetDefaultCountryDatabase();
    texts.push_back(QString::fromStdString(region.GetName()));
    for(auto& country : region.GetCountries()) {
      auto& entry = country_database.FromCode(country);
      texts.push_back(QString::fromStdString(entry.m_name));
      texts.push_back(QString::fromStdString(entry.m_twoLetterCode.GetData()));
      auto code = QString::fromStdString(entry.m_threeLetterCode.GetData());
      for(auto i = 0; i < code.length(); ++i) {
        texts.push_back(code.right(code.length() - i));
      }
    }
    auto& market_database = GetDefaultMarketDatabase();
    for(auto& market : region.GetMarkets()) {
      auto& entry = market_database.FromCode(market);
      texts.push_back(QString::fromStdString(entry.m_code.GetData()));
      texts.push_back(QString::fromStdString(entry.m_description));
      auto name = QString::fromStdString(entry.m_displayName);
      for(auto i = 0; i < name.length(); ++i) {
        texts.push_back(name.right(name.length() - i));
      }
    }
    for(auto& security : region.GetSecurities()) {
      auto name = displayText(security);
      for(auto i = 0; i < name.length(); ++i) {
        texts.push_back(name.right(name.length() - i));
      }
      auto& market = market_database.FromCode(security.GetMarket());
      texts.push_back(QString::fromStdString(market.m_description));
      texts.push_back(QString::fromStdString(market.m_displayName));
    }
    return texts;
  }

  auto build_search_text(const Destination& destination) {
    auto texts = std::vector<QString>();
    auto& entry = GetDefaultDestinationDatabase().FromId(destination);
    auto id = QString::fromStdString(entry.m_id);
    for(auto i = 0; i < id.length(); ++i) {
      texts.push_back(id.right(id.length() - i));
    }
    texts.push_back(QString::fromStdString(entry.m_description));
    return texts;
  }

  auto build_search_text(const OrderType& order_type) {
    auto texts = std::vector<QString>();
    if(order_type == OrderType::NONE) {
      return texts;
    }
    auto text = displayText(order_type);
    for(auto i = 0; i < text.size(); ++i) {
      texts.push_back(text.right(text.size() - i));
    }
    return texts;
  }

  auto build_search_text(const Side& side) {
    auto texts = std::vector<QString>();
    if(side == Side::NONE) {
      return texts;
    }
    auto text = displayText(side);
    for(auto i = 0; i < text.length(); ++i) {
      texts.push_back(text.right(text.length() - i));
    }
    auto osstr = std::ostringstream();
    osstr << side;
    texts.push_back(QString::fromStdString(osstr.str()));
    return texts;
  }

  auto build_search_text(const optional<Quantity>& quantity) {
    auto texts = std::vector<QString>();
    if(!quantity) {
      return texts;
    }
    auto osstr = std::ostringstream();
    osstr << *quantity;
    auto string = osstr.str();
    auto text = [&] {
      if(auto pos = string.find('.'); pos != std::string::npos) {
        if(auto last_zero_pos = string.find_last_not_of('0');
            last_zero_pos > pos) {
          return QString::fromStdString(
            string.erase(last_zero_pos + 1, std::string::npos));
        }
      }
      return QString::fromStdString(string);
    }();
    for(auto i = 0; i < text.size(); ++i) {
      texts.push_back(text.right(text.size() - i));
    }
    return texts;
  }

  auto build_search_text(const TimeInForce& time_in_force) {
    auto texts = std::vector<QString>();
    if(time_in_force.GetType() == TimeInForce::Type::NONE) {
      return texts;
    }
    auto text = displayText(time_in_force);
    for(auto i = 0; i < text.length(); ++i) {
      texts.push_back(text.right(text.length() - i));
    }
    auto type = time_in_force.GetType();
    auto values = [&] () -> std::vector<QString> {
      if(type == TimeInForce::Type::FOK) {
        return {"fill", "kill", "fill or kill"};
      } else if(type == TimeInForce::Type::GTC) {
        return {"good", "cancel", "good till cancel"};
      } else if(type == TimeInForce::Type::GTD) {
        return {"good", "date", "good till date"};
      } else if(type == TimeInForce::Type::GTX) {
        return {"good", "crossing", "good till crossing"};
      } else if(type == TimeInForce::Type::IOC) {
        return {"immediate", "cancel", "immediate or cancel"};
      } else if(type == TimeInForce::Type::MOC) {
        return {"close", "at the close"};
      } else if(type == TimeInForce::Type::OPG) {
        return {"opening", "at the opening"};
      } else {
        return {};
      }
    }();
    texts.insert(texts.end(), values.begin(), values.end());
    return texts;
  }

  auto build_search_text(const QKeySequence& key) {
    auto texts = std::vector<QString>();
    auto text = key.toString();
    text.replace('+', ' ');
    for(auto i = 0; i < text.length(); ++i) {
      texts.push_back(text.right(text.length() - i));
    }
    return texts;
  }

  auto build_text(const std::any& value) {
    if(value.type() == typeid(QString)) {
      return build_search_text(std::any_cast<QString>(value));
    } else if(value.type() == typeid(Region)) {
      return build_search_text(std::any_cast<Region>(value));
    } else if(value.type() == typeid(Destination)) {
      return build_search_text(std::any_cast<Destination>(value));
    } else if(value.type() == typeid(OrderType)) {
      return build_search_text(std::any_cast<OrderType>(value));
    } else if(value.type() == typeid(Side)) {
      return build_search_text(std::any_cast<Side>(value));
    } else if(value.type() == typeid(optional<Quantity>)) {
      return build_search_text(std::any_cast<optional<Quantity>>(value));
    } else if(value.type() == typeid(TimeInForce)) {
      return build_search_text(std::any_cast<TimeInForce>(value));
    } else if(value.type() == typeid(QKeySequence)) {
      return build_search_text(std::any_cast<QKeySequence>(value));
    }
    return std::vector<QString>{};
  }

  auto is_in_layout(QLayout* layout, QWidget* widget) {
    for(auto i = 0; i < layout->count(); ++i) {
      if(layout->itemAt(i)->widget() == widget) {
        return true;
      }
    }
    return false;
  }

  QWidget& get_table_item_body(const TableItem& item) {
    return static_cast<Button*>(item.layout()->itemAt(0)->widget())->get_body();
  }

  EditableBox* get_editable_box_from_item(const TableItem& item) {
    return static_cast<EditableBox*>(&get_table_item_body(item));
  }

  EditableBox* get_editable_box_from_popup_item(const TableItem& item) {
    return static_cast<EditableBox*>(&static_cast<PopupBox*>(
      get_table_item_body(item).layout()->itemAt(0)->widget())->get_body());
  }
}

std::size_t OrderTasksPage::RegionKeyHash::operator()(
    const std::pair<Region, QKeySequence>& region_key) const {
  auto seed = std::size_t(0);
  hash_combine(seed, hash_range(region_key.first.GetCountries().begin(),
    region_key.first.GetCountries().end()));
  auto markets = region_key.first.GetMarkets();
  hash_combine(seed, hash_range(markets.begin(), markets.end()));
  hash_combine(seed, hash_range(region_key.first.GetSecurities().begin(),
    region_key.first.GetSecurities().end()));
  hash_combine(seed, qHash(region_key.second));
  return seed;
}

OrderTasksPage::AddedRow::AddedRow()
  : m_source_index(-1),
    m_filter_source_index(-1),
    m_is_filtered(false) {}

OrderTasksPage::OrderTasksPage(
    std::shared_ptr<ComboBox::QueryModel> region_query_model,
    std::shared_ptr<ListModel<OrderTask>> order_tasks,
    DestinationDatabase destinations, MarketDatabase markets, QWidget* parent)
    : QWidget(parent),
      m_region_query_model(std::move(region_query_model)),
      m_order_tasks(std::move(order_tasks)),
      m_destinations(std::move(destinations)),
      m_markets(std::move(markets)),
      m_rows(std::make_shared<ArrayListModel<std::shared_ptr<TableRow>>>()) {
  auto layout = make_vbox_layout(this);
  layout->setContentsMargins({0, 0, 0, scale_height(20)});
  layout->addWidget(make_help_text_region());
  auto [search_region, search_box] = make_search_region();
  m_search_box = search_box;
  m_search_box->get_current()->connect_update_signal(
    std::bind_front(&OrderTasksPage::on_search, this));
  layout->addWidget(search_region);
  m_order_tasks_table = std::make_shared<OrderTasksToTableModel>(m_order_tasks);
  m_source_table_operation_connection =
    m_order_tasks_table->connect_operation_signal(
      std::bind_front(&OrderTasksPage::on_source_table_operation, this));
  m_table_matcher =
    std::make_unique<TableMatcher>(m_order_tasks_table, build_text);
  m_filtered_table = std::make_shared<FilteredTableModel>(
    std::make_shared<OrderTasksTableViewModel>(m_order_tasks_table),
    [] (const TableModel& model, int row) {
      return false;
    });
  m_filtered_table->connect_operation_signal(
    std::bind_front(&OrderTasksPage::on_filter_table_operation, this));
  auto table_view = TableViewBuilder(m_filtered_table).
    set_header(make_header_model()).
    set_view_builder(
      std::bind_front(&OrderTasksPage::table_view_builder, this)).
    set_comparator(order_tasks_comparator).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_current_connection = table_view->get_current()->connect_update_signal(
    std::bind_front(&OrderTasksPage::on_current, this));
  m_sort_connection = table_view->connect_sort_signal(
    std::bind_front(&OrderTasksPage::on_sort, this));
  auto table_header = static_cast<TableHeader*>(static_cast<Box*>(
    table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  auto grab_handle_header =
    static_cast<TableHeaderItem*>(table_header->layout()->itemAt(
      static_cast<int>(Column::GRAB_HANDLE))->widget());
  grab_handle_header->set_is_resizeable(false);
  auto widths = make_header_widths();
  for(auto i = 0; i < std::ssize(widths); ++i) {
    table_header->get_widths()->set(i, widths[i]);
  }
  m_table_body = static_cast<TableBody*>(&static_cast<ScrollBox*>(
    table_view->layout()->itemAt(1)->widget())->get_body());
  m_table_body->installEventFilter(this);
  m_view_operation_connection =
    m_table_body->get_table()->connect_operation_signal(
      std::bind_front(&OrderTasksPage::on_view_table_operation, this));
  auto table_view_box = new Box(table_view);
  table_view_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*table_view_box, [] (auto& style) {
    style = TABLE_VIEW_STYLE(style);
  });
  layout->addWidget(table_view_box);
  m_table_menu = new ContextMenu(*this);
  m_table_menu->add_action(tr("Delete Order"),
    std::bind_front(&OrderTasksPage::on_delete_order, this));
  m_table_body_focus_observer = std::make_unique<FocusObserver>(*m_table_body);
  m_table_body_focus_observer->connect_state_signal(
    std::bind_front(&OrderTasksPage::on_table_body_focus, this));
  auto& children = m_table_body->children();
  auto count = 0;
  for(auto i = children.rbegin(); i != children.rend(); ++i) {
    if(count >= OrderTasksRow::COLUMN_SIZE) {
      break;
    }
    if((*i)->isWidgetType() &&
        !is_in_layout(m_table_body->layout(), static_cast<QWidget*>(*i))) {
      (*i)->installEventFilter(this);
      ++count;
    }
  }
  m_list_operation_connection = m_order_tasks->connect_operation_signal(
    std::bind_front(&OrderTasksPage::on_list_operation, this));
  m_table_row_drag_drop =
    std::make_unique<TableRowDragDrop>(m_order_tasks, m_rows, *table_view);
}

const std::shared_ptr<ComboBox::QueryModel>&
    OrderTasksPage::get_region_query_model() const {
  return m_region_query_model;
}

const std::shared_ptr<ListModel<OrderTask>>&
    OrderTasksPage::get_order_tasks() const {
  return m_order_tasks;
}

bool OrderTasksPage::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table_body) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      switch(key_event.key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Backspace:
          if(auto current = m_table_body->get_current()->get();
              static_cast<Column>(current->m_column) != Column::GRAB_HANDLE) {
            QCoreApplication::sendEvent(find_focus_proxy(
              get_table_item_body(*m_table_body->get_item(*current))), event);
            return true;
          }
        case Qt::Key_A:
          if(key_event.modifiers() & Qt::Modifier::CTRL &&
              !key_event.isAutoRepeat()) {
            return true;
          }
        default:
          if(auto text = key_event.text(); is_a_word(text)) {
            if(auto current = m_table_body->get_current()->get();
                static_cast<Column>(current->m_column) != Column::GRAB_HANDLE) {
              QCoreApplication::sendEvent(find_focus_proxy(
                get_table_item_body(*m_table_body->get_item(*current))), event);
              return true;
            }
          }
      }
    }
  } else if(event->type() == QEvent::MouseButtonPress) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    if(mouse_event.button() == Qt::RightButton) {
      for(auto i = 0; i < m_table_body->layout()->count(); ++i) {
        auto row = m_table_body->layout()->itemAt(i)->widget();
        auto pos = row->mapFromGlobal(mouse_event.globalPos());
        if(row->rect().contains(pos)) {
          for(auto j = 0; j < row->layout()->count(); ++j) {
            auto item = row->layout()->itemAt(j)->widget();
            pos = item->mapFromGlobal(mouse_event.globalPos());
            if(item->rect().contains(pos)) {
              m_table_body->get_current()->set(TableView::Index(i, j));
              break;
            }
          }
          if(i != m_table_body->layout()->count() - 1) {
            m_table_menu->window()->move(QCursor::pos());
            m_table_menu->show();
          }
          break;
        }
      }
      return true;
    }
  } else if(event->type() == QEvent::KeyPress) {
    if(auto current = m_table_body->get_current()->get();
        current && (static_cast<Column>(current->m_column) == Column::REGION ||
        static_cast<Column>(current->m_column) == Column::QUANTITY)) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Tab) {
        focusNextPrevChild(true);
        return true;
      } else if(key_event.key() == Qt::Key_Backtab) {
        focusNextPrevChild(false);
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool OrderTasksPage::focusNextPrevChild(bool next) {
  if(auto current = m_table_body->get_current()->get();
      !current || (static_cast<Column>(current->m_column) != Column::REGION &&
        static_cast<Column>(current->m_column) != Column::QUANTITY)) {
    auto focus_widget = QApplication::focusWidget();
    if(!m_table_body->isAncestorOf(focus_widget) &&
        m_table_body != focus_widget) {
      auto next_focus_widget = [&] {
        if(next) {
          return focus_widget->nextInFocusChain();
        }
        return focus_widget->previousInFocusChain();
      }();
      if(!m_table_body->isAncestorOf(next_focus_widget) &&
          m_table_body != focus_widget) {
        return QWidget::focusNextPrevChild(next);
      }
    }
  }
  auto is_editing = [&] {
    if(auto current = m_table_body->get_current()->get()) {
      if(static_cast<Column>(current->m_column) == Column::GRAB_HANDLE) {
        return false;
      } else if(static_cast<Column>(current->m_column) == Column::REGION ||
          static_cast<Column>(current->m_column) == Column::QUANTITY) {
        return get_editable_box_from_popup_item(
          *m_table_body->get_item(*current))->is_editing();
      }
      return get_editable_box_from_item(
        *m_table_body->get_item(*current))->is_editing();
    }
    return false;
  }();
  m_table_body->setFocus();
  if(next) {
    table_view_navigate_next();
  } else {
    table_view_navigate_previous();
  }
  if(is_editing) {
    if(auto current = m_table_body->get_current()->get()) {
      if(static_cast<Column>(current->m_column) == Column::GRAB_HANDLE) {
        return true;
      } else if(static_cast<Column>(current->m_column) == Column::REGION ||
          static_cast<Column>(current->m_column) == Column::QUANTITY) {
        get_editable_box_from_popup_item(
          *m_table_body->get_item(*current))->set_editing(true);
      } else {
        get_editable_box_from_item(
          *m_table_body->get_item(*current))->set_editing(true);
      }
    }
  }
  return true;
}

QWidget* OrderTasksPage::table_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto column_id = static_cast<Column>(column);
  if(column_id == Column::GRAB_HANDLE) {
    auto source_row = [&] {
      if(m_added_row.m_source_index != -1) {
        return m_added_row.m_source_index;
      }
      if(m_added_row.m_filter_source_index != -1) {
        return m_added_row.m_filter_source_index;
      }
      return row;
    }();
    m_rows->insert(
      std::make_shared<OrderTasksRow>(m_order_tasks, source_row), row);
  }
  auto cell = std::static_pointer_cast<OrderTasksRow>(m_rows->get(row))->
    build_cell(m_region_query_model, m_destinations, m_markets, table, row,
      column);
  if(!cell.m_editor) {
    return cell.m_cell;
  }
  cell.m_editor->connect_start_edit_signal([=] {
    if(column_id == Column::REGION || column_id == Column::QUANTITY) {
      find_focus_proxy(*cell.m_editor)->installEventFilter(this);
    }
  });
  cell.m_editor->connect_end_edit_signal([=] {
    if(column_id == Column::REGION || column_id == Column::QUANTITY) {
      find_focus_proxy(*cell.m_editor)->removeEventFilter(this);
    }
    if(!QApplication::focusWidget()) {
      m_table_body->setFocus();
    }
  });
  auto get_row_index = [=] (TableRow* table_row) {
    for(auto i = 0; i < m_rows->get_size() - 1; ++i) {
      if(m_rows->get(i).get() == table_row) {
        return i;
      }
    }
    return -1;
  };
  if(column_id == Column::REGION) {
    cell.m_editor->get_input_box().connect_submit_signal(
      [=, order_tasks_row = m_rows->get(row).get()] (const auto& submission) {
        if(auto row_index = get_row_index(order_tasks_row); row_index >= 0) {
          update_key(table, row_index, any_cast<Region>(submission),
            table->get<QKeySequence>(row_index, static_cast<int>(Column::KEY)));
        }
    });
  } else if(column_id == Column::KEY) {
    cell.m_editor->get_input_box().connect_submit_signal(
      [=, order_tasks_row = m_rows->get(row).get()] (const auto& submission) {
        if(auto row_index = get_row_index(order_tasks_row); row_index >= 0) {
          update_key(table, row_index,
            table->get<Region>(row_index, static_cast<int>(Column::REGION)),
            any_cast<QKeySequence>(submission));
        }
    });
  }
  return cell.m_cell;
}

void OrderTasksPage::table_view_navigate_next() {
  if(auto current = m_table_body->get_current()->get()) {
    auto column = current->m_column + 1;
    if(column >= m_table_body->get_table()->get_column_size()) {
      auto row = current->m_row + 1;
      if(row >= m_table_body->get_table()->get_row_size()) {
        auto next_focus_widget = nextInFocusChain();
        while(isAncestorOf(next_focus_widget)) {
          next_focus_widget = next_focus_widget->nextInFocusChain();
        }
        next_focus_widget->setFocus();
        m_table_body->get_current()->set(none);
      } else {
        m_table_body->get_current()->set(TableView::Index(row, 0));
      }
    } else {
      m_table_body->get_current()->set(
        TableView::Index(current->m_row, column));
    }
  } else {
    m_table_body->get_current()->set(TableView::Index(0, 0));
  }
}

void OrderTasksPage::table_view_navigate_previous() {
  if(auto current = m_table_body->get_current()->get()) {
    auto column = current->m_column - 1;
    if(column < 0) {
      auto row = current->m_row - 1;
      if(row < 0) {
        QWidget::focusNextPrevChild(false);
        m_table_body->get_current()->set(none);
      } else {
        m_table_body->get_current()->set(TableView::Index(row,
          m_table_body->get_table()->get_column_size() - 1));
      }
    } else {
      m_table_body->get_current()->set(
        TableView::Index(current->m_row, column));
    }
  } else {
    m_table_body->get_current()->set(
      TableView::Index(m_table_body->get_table()->get_row_size() - 1,
        m_table_body->get_table()->get_column_size() - 1));
  }
}

void OrderTasksPage::do_search(const QString& query) {
  auto blocker = shared_connection_block(m_current_connection);
  m_filtered_table->set_filter([=] (const TableModel& model, int row) {
    if(query.isEmpty() || row == model.get_row_size() - 1) {
      return false;
    }
    auto is_filtered = !m_table_matcher->match(row, query);
    if(m_added_row.m_source_index != -1) {
      if(m_added_row.m_source_index == row) {
        m_added_row.m_is_filtered = is_filtered;
      }
      return false;
    }
    for(auto i = 0; i < m_rows->get_size() - 1; ++i) {
      if(m_rows->get(i)->get_row_index() == row &&
          m_rows->get(i)->is_ignore_filters()) {
        m_rows->get(i)->set_out_of_range(is_filtered);
        return false;
      }
    }
    return is_filtered;
  });
}

void OrderTasksPage::do_search_excluding_a_row(int excluding_row) {
  if(m_search_box->get_current()->get().isEmpty()) {
    return;
  }
  auto need_do = false;
  for(auto row = 0; row < m_rows->get_size() - 1; ++row) {
    if(row != excluding_row && m_rows->get(row)->is_out_of_range()) {
      m_rows->get(row)->set_ignore_filters(false);
      need_do = true;
    }
  }
  if(need_do) {
    do_search(m_search_box->get_current()->get());
  }
}

void OrderTasksPage::do_search_on_all_rows() {
  do_search_excluding_a_row(-1);
}

void OrderTasksPage::update_key(const std::shared_ptr<TableModel>& table,
    int row, const Region& region, const QKeySequence& key) {
  if(!m_region_key_set.contains({region, key})) {
    return;
  }
  for(auto i = 0; i < table->get_row_size() - 1; ++i) {
    if(i != row) {
      if(table->get<QKeySequence>(i, static_cast<int>(Column::KEY)) == key &&
          table->get<Region>(i, static_cast<int>(Column::REGION)) == region) {
        table->set(i, static_cast<int>(Column::KEY), QKeySequence());
        return;
      }
    }
  }
}

void OrderTasksPage::on_current(const optional<TableView::Index>& index) {
  if(!index) {
    if(m_previous_index && m_previous_index->m_row < m_rows->get_size() - 1) {
      m_rows->get(m_previous_index->m_row)->set_ignore_filters(false);
    }
    do_search_on_all_rows();
  } else {
    m_rows->get(index->m_row)->set_ignore_filters(true);
    if(!m_previous_index) {
      do_search_excluding_a_row(index->m_row);
    } else if(m_previous_index->m_row != index->m_row) {
      if(m_previous_index->m_row < m_rows->get_size() - 1) {
        m_rows->get(m_previous_index->m_row)->set_ignore_filters(false);
      }
      do_search_excluding_a_row(index->m_row);
    }
  }
  m_previous_index = index;
}

void OrderTasksPage::on_delete_order() {
  if(auto current = m_table_body->get_current()->get()) {
    m_order_tasks->remove(m_rows->get(current->m_row)->get_row_index());
  }
}

void OrderTasksPage::on_search(const QString& value) {
  for(auto row = 0; row < m_rows->get_size() - 1; ++row) {
    m_rows->get(row)->set_ignore_filters(false);
    m_rows->get(row)->set_out_of_range(false);
  }
  do_search(value);
  m_table_body->adjustSize();
  m_table_body->get_current()->set(m_table_body->get_current()->get());
}

void OrderTasksPage::on_sort(int column, TableHeaderItem::Order order) {
  m_table_body->update();
}

void OrderTasksPage::on_table_body_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    if(!QApplication::focusWidget()) {
      return;
    }
    if(auto current = m_table_body->get_current()->get()) {
      m_rows->get(current->m_row)->set_ignore_filters(false);
    }
    do_search_on_all_rows();
  }
}

void OrderTasksPage::on_list_operation(
    const ListModel<OrderTask>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<OrderTask>::AddOperation& operation) {
      m_table_body->adjustSize();
    },
    [&] (const ListModel<OrderTask>::RemoveOperation& operation) {
      m_table_body->adjustSize();
    });
}

void OrderTasksPage::on_source_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_added_row.m_source_index = operation.m_index;
      auto region = std::any_cast<Region>(operation.m_row->get(
        static_cast<int>(OrderTasksToTableModel::Column::REGION)));
      auto key = std::any_cast<QKeySequence>(operation.m_row->get(
        static_cast<int>(OrderTasksToTableModel::Column::KEY)));
      if(!key.isEmpty()) {
        m_region_key_set.insert(std::pair(region, key));
      }
      update_key(m_table_body->get_table(), operation.m_index, region, key);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_region_key_set.erase({
        std::any_cast<Region>(operation.m_row->get(
          static_cast<int>(OrderTasksToTableModel::Column::REGION))),
        std::any_cast<QKeySequence>(operation.m_row->get(
          static_cast<int>(OrderTasksToTableModel::Column::KEY)))});
    },
    [&] (const TableModel::UpdateOperation& operation) {
      if(static_cast<OrderTasksToTableModel::Column>(operation.m_column) ==
          OrderTasksToTableModel::Column::REGION) {
        auto& region = std::any_cast<const Region&>(operation.m_value);
        auto& key = m_order_tasks_table->get<QKeySequence>(operation.m_row,
          static_cast<int>(OrderTasksToTableModel::Column::KEY));
        m_region_key_set.erase(
          {std::any_cast<Region>(operation.m_previous), key});
        if(!key.isEmpty()) {
          m_region_key_set.insert({region, key});
        }
      } else if(static_cast<OrderTasksToTableModel::Column>(operation.m_column)
          == OrderTasksToTableModel::Column::KEY) {
        auto& region = m_order_tasks_table->get<Region>(operation.m_row,
          static_cast<int>(OrderTasksToTableModel::Column::REGION));
        auto& key = std::any_cast<const QKeySequence&>(operation.m_value);
        m_region_key_set.erase(
          {region, std::any_cast<QKeySequence>(operation.m_previous)});
        if(!key.isEmpty()) {
          m_region_key_set.insert({region, key});
        }
      }
    });
}

void OrderTasksPage::on_view_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_rows->get(operation.m_index)->get_row()->show();
      if(m_added_row.m_source_index != -1) {
        if(auto current = m_table_body->get_current()->get()) {
          m_table_body->get_current()->set(
            TableView::Index(operation.m_index, current->m_column));
        }
        m_rows->get(operation.m_index)->set_out_of_range(
          m_added_row.m_is_filtered);
        m_added_row.m_source_index = -1;
        m_added_row.m_is_filtered = false;
      }
      if(m_added_row.m_filter_source_index != -1) {
        m_added_row.m_filter_source_index = -1;
      }
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_rows->remove(operation.m_index);
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_rows->move(operation.m_source, operation.m_destination);
    });
}

void OrderTasksPage::on_filter_table_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_added_row.m_filter_source_index =
        std::any_cast<int>(operation.m_row->get(0));
    });
}
