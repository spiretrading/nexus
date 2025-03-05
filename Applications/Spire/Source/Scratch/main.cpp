#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QAbstractItemModel>
#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QAbstractScrollArea>
#include <QHeaderView>
#include <QTableView>
#include <QMap>
#include <QSize>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QPair>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsTableModel.hpp"
#include "Spire/KeyBindings/QuantitySettingBox.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"

using namespace Nexus;
using namespace Spire;

class VTableView : public QWidget {
  public:
    VTableView(std::shared_ptr<TableModel> table,
      TableViewItemBuilder item_builder, QWidget* parent = nullptr);
    ~VTableView() override;

    QSize sizeHint() const override;

  protected:
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private:
    struct CellPosition {
      int row;
      int column;

      auto operator <=>(const CellPosition& other) const = default;
    };

    friend uint qHash(const CellPosition&);

    struct CellGeometry {
      int x;
      int y;
      int width;
      int height;
    };

    // Private methods
    void updateVisibleRange();
    void updateScrollBarRanges();
    void updateContentsSize();
    void on_operation(const TableModel::Operation& operation);
    
    QWidget* createOrUpdateWidgetAt(int row, int column);
    void recycleWidget(QWidget* widget);
    CellGeometry cellGeometryAt(int row, int column) const;
    QPair<CellPosition, CellPosition> visibleCellRange() const;
    
    void cleanupInvisibleWidgets();
    void layoutVisibleWidgets();

    // Core data
    QSize m_size_hint;
    std::shared_ptr<TableModel> m_table;
    TableViewItemBuilder m_item_builder;

    // Widget management
    QMap<CellPosition, QWidget*> m_visibleWidgets;

    // Cache for viewport calculations
    CellPosition m_topLeftVisible;
    CellPosition m_bottomRightVisible;
    QSize m_defaultCellSize{120, 40};
    
    // Settings
    int m_cellSpacing = 1;
    int m_borderWidth = 1;
    QColor m_borderColor = Qt::lightGray;
};

uint qHash(const VTableView::CellPosition& t) {
  return 0;
}

VTableView::VTableView(std::shared_ptr<TableModel> table,
    TableViewItemBuilder item_builder, QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_item_builder(std::move(item_builder)) {
  m_table->connect_operation_signal(
    std::bind_front(&VTableView::on_operation, this));
  updateContentsSize();
  updateVisibleRange();
}

VTableView::~VTableView() {
  qDeleteAll(m_visibleWidgets);
}

QSize VTableView::sizeHint() const {
  return m_size_hint;
}

void VTableView::moveEvent(QMoveEvent* event) {
  updateVisibleRange();
}

void VTableView::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  updateVisibleRange();
  updateScrollBarRanges();
}

void VTableView::updateContentsSize() {
  int rows = m_table->get_row_size();
  int cols = m_table->get_column_size();

  // Calculate total content size
  int contentWidth =
    cols * (m_defaultCellSize.width() + m_cellSpacing) + m_cellSpacing;
  int contentHeight =
    rows * (m_defaultCellSize.height() + m_cellSpacing) + m_cellSpacing;
  m_size_hint = QSize(qMax(0, contentWidth), qMax(0, contentHeight));
  updateGeometry();
}

void VTableView::on_operation(const TableModel::Operation& operation) {
  updateContentsSize();
  updateVisibleRange();
}

void VTableView::updateScrollBarRanges() {
  updateContentsSize();
}

QPair<VTableView::CellPosition, VTableView::CellPosition>
    VTableView::visibleCellRange() const {
  auto parent_size = [&] {
    if(auto parent = parentWidget()) {
      return parent->size();
    }
    return QSize(0, 0);
  }();
  int xOffset = 0;
  int yOffset = -pos().y();
  int cellWidth = m_defaultCellSize.width() + m_cellSpacing;
  int cellHeight = m_defaultCellSize.height() + m_cellSpacing;
  int firstVisibleRow = qMax(0, yOffset / cellHeight);
  int firstVisibleCol = qMax(0, xOffset / cellWidth);
  int lastVisibleRow = qMin(m_table->get_row_size() - 1,
    (yOffset + parent_size.height()) / cellHeight + 1);
  int lastVisibleCol = qMin(m_table->get_column_size() - 1,
    (xOffset + parent_size.width()) / cellWidth + 1);
  return {{firstVisibleRow, firstVisibleCol}, {lastVisibleRow, lastVisibleCol}};
}

void VTableView::updateVisibleRange() {
  auto visibleRange = visibleCellRange();

  // Create set of currently visible positions
  QSet<CellPosition> newVisiblePositions;
  for(int row = visibleRange.first.row; row <= visibleRange.second.row; ++row) {
    for(int col = visibleRange.first.column;
        col <= visibleRange.second.column; ++col) {
      newVisiblePositions.insert({row, col});
    }
  }

  // Find positions that are no longer visible
  QList<CellPosition> positionsToRemove;
  for(auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end(); ++it) {
    if(!newVisiblePositions.contains(it.key())) {
      positionsToRemove.append(it.key());
    }
  }

  // Recycle widgets for positions no longer visible
  for(const auto& pos : positionsToRemove) {
    recycleWidget(m_visibleWidgets.take(pos));
  }

  // Create widgets for newly visible positions
  for(int row = visibleRange.first.row; row <= visibleRange.second.row; ++row) {
    for(int col = visibleRange.first.column;
        col <= visibleRange.second.column; ++col) {
      CellPosition pos{row, col};
      if(!m_visibleWidgets.contains(pos)) {
        createOrUpdateWidgetAt(row, col);
      }
    }
  }

  // Update positions of all visible widgets
  layoutVisibleWidgets();
}

QWidget* VTableView::createOrUpdateWidgetAt(int row, int column) {
  CellPosition pos{row, column};

  // Check if we already have a widget for this position
  if(m_visibleWidgets.contains(pos)) {
    return m_visibleWidgets[pos];
  }

  // Try to reuse a recycled widget
  QWidget* widget = m_item_builder.mount(m_table, row, column);

  // Store and show the widget
  widget->setParent(this);
  m_visibleWidgets[pos] = widget;
  return widget;
}

void VTableView::recycleWidget(QWidget* widget) {
  m_item_builder.unmount(widget);
}

VTableView::CellGeometry VTableView::cellGeometryAt(int row, int column) const {
  int xOffset = 0;
  int yOffset = -pos().y();
  int x = column * (m_defaultCellSize.width() + m_cellSpacing) +
    m_cellSpacing - xOffset;
  int y = row * (m_defaultCellSize.height() + m_cellSpacing) + m_cellSpacing;
  return {x, y, m_defaultCellSize.width(), m_defaultCellSize.height()};
}

void VTableView::layoutVisibleWidgets() {
  for(auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end(); ++it) {
    CellPosition pos = it.key();
    QWidget* widget = it.value();
    auto geometry = cellGeometryAt(pos.row, pos.column);
    widget->setGeometry(
      geometry.x, geometry.y, geometry.width, geometry.height);
    widget->show();
  }
}

void VTableView::cleanupInvisibleWidgets() {
  auto visibleRange = visibleCellRange();
  for(auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end();) {
    CellPosition pos = it.key();
    if(pos.row < visibleRange.first.row || pos.row > visibleRange.second.row ||
        pos.column < visibleRange.first.column ||
        pos.column > visibleRange.second.column) {
      recycleWidget(it.value());
      it = m_visibleWidgets.erase(it);
    } else {
      ++it;
    }
  }
}

struct ItemState {
  virtual ~ItemState() = default;
  std::shared_ptr<void> m_proxy;

  ItemState(std::shared_ptr<void> proxy)
    : m_proxy(std::move(proxy)) {}
};

struct DestinationState : ItemState {
  std::shared_ptr<ProxyValueModel<Region>> m_region;

  DestinationState(std::shared_ptr<void> proxy,
    std::shared_ptr<ProxyValueModel<Region>> region)
    : ItemState(std::move(proxy)),
      m_region(std::move(region)) {}
};

struct AdditionalTagsState : ItemState {
  std::shared_ptr<ProxyValueModel<Destination>> m_destination;
  std::shared_ptr<ProxyValueModel<Region>> m_region;

  AdditionalTagsState(std::shared_ptr<void> proxy,
    std::shared_ptr<ProxyValueModel<Destination>> destination,
    std::shared_ptr<ProxyValueModel<Region>> region)
    : ItemState(std::move(proxy)),
      m_destination(std::move(destination)),
      m_region(std::move(region)) {}
};

auto key_input_box_validator(const QKeySequence& sequence) {
  if(sequence.count() == 0) {
    return QValidator::Intermediate;
  } else if(sequence.count() > 1) {
    return QValidator::Invalid;
  }
  auto key = sequence[0];
  auto modifier = key & Qt::KeyboardModifierMask;
  auto new_key = key - modifier;
  if(((modifier == Qt::NoModifier || modifier & Qt::ControlModifier ||
        modifier & Qt::ShiftModifier || modifier & Qt::AltModifier) &&
        new_key >= Qt::Key_F1 && new_key <= Qt::Key_F12) ||
      ((modifier & Qt::ControlModifier || modifier & Qt::ShiftModifier ||
        modifier & Qt::AltModifier) && new_key >= Qt::Key_0 &&
        new_key <= Qt::Key_9)) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

struct TaskKeysTableViewItemBuilder {
  std::shared_ptr<RegionQueryModel> m_regions;
  DestinationDatabase m_destinations;
  MarketDatabase m_markets;
  AdditionalTagDatabase m_additional_tags;
  std::map<QWidget*, std::shared_ptr<ItemState>> m_item_states;

  QWidget* mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto make_proxy = [&] <typename T> () {
      return make_proxy_value_model(
        make_table_value_model<T>(table, row, column));
    };
    auto column_id = static_cast<OrderTaskColumns>(column);
    auto [input_box, proxy] =
      [&] () -> std::tuple<QWidget*, std::shared_ptr<ItemState>> {
        if(column_id == OrderTaskColumns::NAME) {
          auto current = make_proxy.operator ()<QString>();
          return {new TextBox(current), std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::REGION) {
          auto current = make_proxy.operator ()<Region>();
          auto region_box = new RegionBox(m_regions, current);
          region_box->setFixedHeight(scale_height(25));
          region_box->setSizePolicy(
            QSizePolicy::Preferred, QSizePolicy::Fixed);
          return {region_box,
            std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::DESTINATION) {
          auto region = make_proxy_value_model(
            make_table_value_model<Region>(table, row,
              static_cast<int>(OrderTaskColumns::REGION)));
          auto destinations = make_region_filtered_destination_list(
            m_destinations, m_markets, region);
          auto current = make_proxy.operator ()<Destination>();
          return {make_destination_box(current, std::move(destinations)),
            std::make_shared<DestinationState>(current, region)};
        } else if(column_id == OrderTaskColumns::ORDER_TYPE) {
          auto current = make_proxy.operator ()<OrderType>();
          return {make_order_type_box(current),
            std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::SIDE) {
          auto current = make_proxy.operator ()<Side>();
          return {make_side_box(current), std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::QUANTITY) {
          auto current = make_proxy.operator ()<QuantitySetting>();
          return {make_quantity_setting_box(current),
            std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::TIME_IN_FORCE) {
          auto current = make_proxy.operator ()<TimeInForce>();
          return {make_time_in_force_box(current),
            std::make_shared<ItemState>(current)};
        } else if(column_id == OrderTaskColumns::TAGS) {
          auto destination = make_proxy_value_model(
            make_table_value_model<Destination>(
              table, row, static_cast<int>(OrderTaskColumns::DESTINATION)));
          auto region = make_proxy_value_model(make_table_value_model<Region>(
            table, row, static_cast<int>(OrderTaskColumns::REGION)));
          auto current = make_proxy.operator ()<std::vector<AdditionalTag>>();
          return {new AdditionalTagsBox(
              current, m_additional_tags, destination, region),
            std::make_shared<AdditionalTagsState>(
              current, destination, region)};
        } else {
          auto proxy = make_proxy.operator ()<QKeySequence>();
          auto current =
            make_validated_value_model(&key_input_box_validator, proxy);
          return {new KeyInputBox(current), std::make_shared<ItemState>(proxy)};
        }
      }();
    m_item_states[input_box] = std::move(proxy);
    return input_box;
  }

  void reset(QWidget& widget,
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto update_proxy = [&] <typename T> () {
      auto& state = *m_item_states[&widget];
      std::static_pointer_cast<ProxyValueModel<T>>(state.m_proxy)->set_source(
        make_table_value_model<T>(table, row, column));
    };
    auto column_id = static_cast<OrderTaskColumns>(column);
    if(column_id == OrderTaskColumns::NAME) {
      update_proxy.operator ()<QString>();
    } else if(column_id == OrderTaskColumns::REGION) {
      update_proxy.operator ()<Region>();
    } else if(column_id == OrderTaskColumns::DESTINATION) {
      auto& state = static_cast<DestinationState&>(*m_item_states[&widget]);
      auto proxy =
        std::static_pointer_cast<ProxyValueModel<Destination>>(state.m_proxy);
      auto temporary_model =
        std::make_shared<LocalDestinationModel>(proxy->get());
      proxy->set_source(temporary_model);
      state.m_region->set_source(make_table_value_model<Region>(
        table, row, static_cast<int>(OrderTaskColumns::REGION)));
      update_proxy.operator ()<Destination>();
    } else if(column_id == OrderTaskColumns::ORDER_TYPE) {
      update_proxy.operator ()<OrderType>();
    } else if(column_id == OrderTaskColumns::SIDE) {
      update_proxy.operator ()<Side>();
    } else if(column_id == OrderTaskColumns::QUANTITY) {
      update_proxy.operator ()<QuantitySetting>();
    } else if(column_id == OrderTaskColumns::TIME_IN_FORCE) {
      update_proxy.operator ()<TimeInForce>();
    } else if(column_id == OrderTaskColumns::TAGS) {
      auto& state =
        static_cast<AdditionalTagsState&>(*m_item_states[&widget]);
      auto proxy = std::static_pointer_cast<
        ProxyValueModel<std::vector<AdditionalTag>>>(state.m_proxy);
      auto temporary_model =
        std::make_shared<LocalValueModel<std::vector<AdditionalTag>>>(
          proxy->get());
      proxy->set_source(temporary_model);
      state.m_destination->set_source(make_table_value_model<Destination>(
        table, row, static_cast<int>(OrderTaskColumns::DESTINATION)));
      state.m_region->set_source(make_table_value_model<Region>(
        table, row, static_cast<int>(OrderTaskColumns::REGION)));
      update_proxy.operator ()<std::vector<AdditionalTag>>();
    } else if(column_id == OrderTaskColumns::KEY) {
      update_proxy.operator ()<QKeySequence>();
    }
  }

  void unmount(QWidget* widget) {
    delete widget;
  }
};

struct VTableViewItemBuilder {
  QWidget* mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto text = make_proxy_value_model(
      make_to_text_model(make_table_value_model<int>(table, row, column)));
    auto label = make_label(text);
    label->setFixedHeight(40);
    return label;
  }

  void unmount(QWidget* widget) {
    delete widget;
  }

  void reset(QWidget& widget, const std::shared_ptr<TableModel>& table, int row,
      int column) {
    auto& label = static_cast<TextBox&>(widget);
    auto proxy =
      std::static_pointer_cast<ProxyValueModel<QString>>(label.get_current());
    proxy->set_source(
      make_to_text_model(make_table_value_model<int>(table, row, column)));
  }
};

template<typename T>
auto make_vtable_view(
    std::shared_ptr<TableModel> table, T&& item_builder, QWidget* parent) {
  auto table_view = new VTableView(
    std::move(table), std::forward<T>(item_builder));
  table_view->setSizePolicy(QSizePolicy::MinimumExpanding,
    QSizePolicy::MinimumExpanding);
  auto scroll_box = new ScrollBox(table_view, parent);
  scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  scroll_box->set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  scroll_box->set_horizontal(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  return scroll_box;
}

template<typename T>
auto make_stable_view(
    std::shared_ptr<TableModel> table, T&& item_builder, QWidget* parent) {
  auto builder = TableViewBuilder(std::move(table), parent);
  builder.add_header_item("Name");
  builder.add_header_item("Region");
  builder.add_header_item("Destination");
  builder.add_header_item("Order Type");
  builder.add_header_item("Side");
  builder.add_header_item("Quantity");
  builder.add_header_item("Time in Force");
  builder.add_header_item("Tags");
  builder.add_header_item("Key");
  builder.set_item_builder(std::forward<T>(item_builder));
  return builder.make();
}

auto populate_region_query_model(const CountryDatabase& countries,
    const MarketDatabase& markets) {
  auto regions = std::make_shared<LocalQueryModel<Region>>();
  for(auto& country : countries.GetEntries()) {
    auto region = Region(country.m_code);
    region.SetName(country.m_name);
    regions->add(to_text(country.m_code).toLower(), region);
    regions->add(QString::fromStdString(region.GetName()).toLower(), region);
  }
  for(auto& market : markets.GetEntries()) {
    auto region = Region(market);
    region.SetName(market.m_description);
    regions->add(to_text(MarketToken(market.m_code)).toLower(), region);
    regions->add(QString::fromStdString(region.GetName()).toLower(), region);
  }
  return regions;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  QMainWindow mainWindow;
  mainWindow.setWindowTitle("QTableView Demo");
  mainWindow.resize(800, 600);
  QWidget* centralWidget = new QWidget(&mainWindow);
  QVBoxLayout* layout = new QVBoxLayout(centralWidget);
  auto key_bindings =
    std::make_shared<KeyBindingsModel>(GetDefaultMarketDatabase());
  auto task_nodes = make_default_order_task_nodes();
  auto task_arguments = std::make_shared<ArrayListModel<OrderTaskArguments>>();
  for(auto& node : task_nodes) {
    task_arguments->push(to_order_task_arguments(
      *node, GetDefaultMarketDatabase(), GetDefaultDestinationDatabase()));
  }
  auto tasks_table =
    make_order_task_arguments_table_model(std::move(task_arguments));
  auto item_builder = RecycledTableViewItemBuilder(TaskKeysTableViewItemBuilder(
    populate_region_query_model(
      GetDefaultCountryDatabase(), GetDefaultMarketDatabase()),
    GetDefaultDestinationDatabase(), GetDefaultMarketDatabase(),
    get_default_additional_tag_database()));

//  auto tableView = make_vtable_view(tasks_table, item_builder, centralWidget);
  auto tableView = make_stable_view(tasks_table, item_builder, centralWidget);

  // Add TableView to layout
  layout->addWidget(tableView);
  mainWindow.setCentralWidget(centralWidget);
  mainWindow.show();
  application.exec();
}
