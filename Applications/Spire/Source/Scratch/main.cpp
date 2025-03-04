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
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"

using namespace Spire;

/*
class LabelTableViewDelegate : public TableViewDelegate {
    
public:
    explicit LabelTableViewDelegate(QObject* parent = nullptr) : TableViewDelegate(parent) {}
    
    QWidget* createWidgetForCell(const QModelIndex& index, QWidget* parent) const override {
        auto label = make_label(std::make_shared<LocalTextModel>());
        label->setFixedHeight(40);
        // Set font
        QFont font = label->font();
        font.setPointSize(10);
        label->setFont(font);
        
        // Update with data
        updateWidget(label, index);
        
        return label;
    }
    
    void updateWidget(QWidget* widget, const QModelIndex& index) const override {
        auto label = static_cast<TextBox*>(widget);
        if (!label || !index.isValid())
            return;
        
        // Get the data from the model and set it to the label
        QVariant data = index.data(Qt::DisplayRole);
        label->get_current()->set(data.toString());
    }
    
    bool canReuseWidget(QWidget* widget, const QModelIndex&) const override {
        // We can reuse any QLabel
        return static_cast<TextBox*>(widget) != nullptr;
    }
    
    void prepareForReuse(QWidget* widget, const QModelIndex&) const override {
        auto label = static_cast<TextBox*>(widget);
        if (!label)
            return;
        
        // Clear the text
        label->get_current()->set("");
    }
};
*/

class VTableView : public QWidget {
  public:
    VTableView(std::shared_ptr<TableModel> table,
      TableViewItemBuilder item_builder, QWidget* parent = nullptr);
    ~VTableView() override;

  protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private:
    struct CellPosition {
      int row;
      int column;
        
      bool operator<(const CellPosition& other) const {
        return row < other.row || (row == other.row && column < other.column);
      }
        
      bool operator==(const CellPosition& other) const {
        return row == other.row && column == other.column;
      }
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
    void onScroll(int position);

    // Core data
    ScrollBox* m_scrollBox;
    std::shared_ptr<TableModel> m_table;
    TableViewItemBuilder m_item_builder;

    // Widget management
    QMap<CellPosition, QWidget*> m_visibleWidgets;
    QList<QWidget*> m_recycledWidgets;
    
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
  auto body = new QWidget();
  body->setBackgroundRole(QPalette::Base);
  body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_scrollBox = new ScrollBox(body);
  m_scrollBox->set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scrollBox->set_horizontal(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scrollBox->get_vertical_scroll_bar().connect_position_signal(
    std::bind_front(&VTableView::onScroll, this));
  enclose(*this, *m_scrollBox);
  m_table->connect_operation_signal(
    std::bind_front(&VTableView::on_operation, this));
  updateContentsSize();
  updateVisibleRange();
}

VTableView::~VTableView() {
  qDeleteAll(m_visibleWidgets);
  qDeleteAll(m_recycledWidgets);
}

void VTableView::paintEvent(QPaintEvent* event) {
/*    
    QPainter painter(&m_scrollBox->get_body());
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(event->rect(), m_scrollBox->get_body().palette().brush(QPalette::Base));
    
    // Draw grid lines if needed
    if (m_borderWidth > 0) {
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        
        auto range = visibleCellRange();
        for (int row = range.first.row; row <= range.second.row + 1; ++row) {
            for (int col = range.first.column; col <= range.second.column + 1; ++col) {
                auto geometry = cellGeometryAt(row, col);
                
                // Draw horizontal line
                painter.drawLine(0, geometry.y, m_scrollBox->get_body().width(), geometry.y);
                
                // Draw vertical line
                painter.drawLine(geometry.x, 0, geometry.x, m_scrollBox->get_body().height());
            }
        }
    }
    */
    // The widgets are already in place, let them draw themselves
}

void VTableView::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  updateVisibleRange();
  updateScrollBarRanges();
}

void VTableView::onScroll(int position) {
  updateVisibleRange();
}

void VTableView::updateContentsSize() {
  int rows = m_table->get_row_size();
  int cols = m_table->get_column_size();
    
  // Calculate total content size
  int contentWidth =
    cols * (m_defaultCellSize.width() + m_cellSpacing) + m_cellSpacing;
  int contentHeight =
    rows * (m_defaultCellSize.height() + m_cellSpacing) + m_cellSpacing;

  // Update scrollbars
  m_scrollBox->get_body().setFixedSize(
    qMax(0, contentWidth), qMax(0, contentHeight));
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
  int xOffset = 0;
  int yOffset = m_scrollBox->get_vertical_scroll_bar().get_position();
  int cellWidth = m_defaultCellSize.width() + m_cellSpacing;
  int cellHeight = m_defaultCellSize.height() + m_cellSpacing;
  int firstVisibleRow = qMax(0, yOffset / cellHeight);
  int firstVisibleCol = qMax(0, xOffset / cellWidth);
  int lastVisibleRow = qMin(m_table->get_row_size() - 1,
    (yOffset + m_scrollBox->height()) / cellHeight + 1);
  int lastVisibleCol = qMin(m_table->get_column_size() - 1,
    (xOffset + m_scrollBox->width()) / cellWidth + 1);
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
// TODO    m_delegate->updateWidget(m_visibleWidgets[pos], index);
    return m_visibleWidgets[pos];
  }

  // Try to reuse a recycled widget
  QWidget* widget = m_item_builder.mount(m_table, row, column);

  // Store and show the widget
  widget->setParent(&m_scrollBox->get_body());
  m_visibleWidgets[pos] = widget;
  return widget;
}

void VTableView::recycleWidget(QWidget* widget) {
  m_item_builder.unmount(widget);
}

VTableView::CellGeometry VTableView::cellGeometryAt(int row, int column) const {
  int xOffset = 0;
  int yOffset = m_scrollBox->get_vertical_scroll_bar().get_position();
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

class SimpleQTableModel : public QAbstractTableModel {
public:
    explicit SimpleQTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;
        
        return 10000; // 10,000 rows
    }
    
    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;
        
        return 5; // 5 columns
    }
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid())
            return QVariant();
        
        if (role == Qt::DisplayRole) {
            int i = index.row();
            int j = index.column();
            
            return QVariant(5 * j + i);
        }
        
        return QVariant();
    }
};

auto make_qtable_view(QWidget* parent) {
    QTableView* tableView = new QTableView(parent);
    
    // Create and set the model
    SimpleQTableModel* model = new SimpleQTableModel(tableView);
    tableView->setModel(model);
    
    // Configure the QTableView
    tableView->horizontalHeader()->setVisible(false);
    tableView->verticalHeader()->setVisible(false);
    tableView->setGridStyle(Qt::SolidLine);
    tableView->setShowGrid(true);
    return tableView;
}

struct VTableViewItemBuilder {
  QWidget* mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    auto label = make_label(make_to_text_model(
      make_table_value_model<int>(table, row, column)));
    label->setFixedHeight(40);
    return label;
  }

  void unmount(QWidget* widget) {
    delete widget;
  }

  void reset(QWidget& widget, const std::shared_ptr<TableModel>& table, int row,
      int column) {
  }
};

auto make_table_model() {
  auto table = std::make_shared<ArrayTableModel>();
  for(auto i = 0; i != 10000; ++i) {
    auto row = std::vector<std::any>();
    for(auto j = 0; j != 5; ++j) {
      row.push_back(5 * i + j);
    }
    table->push(row);
  }
  return table;
}

auto make_vtable_view(QWidget* parent) {
  VTableView* tableView = new VTableView(make_table_model(),
    RecycledTableViewItemBuilder(VTableViewItemBuilder()), parent);
  return tableView;
}

auto make_stable_view(QWidget* parent) {
  auto builder = TableViewBuilder(make_table_model());
  builder.add_header_item("1");
  builder.add_header_item("2");
  builder.add_header_item("3");
  builder.add_header_item("4");
  builder.add_header_item("5");
  builder.set_item_builder(
    RecycledTableViewItemBuilder(VTableViewItemBuilder()));
  auto view = builder.make();
  view->setParent(parent);
  return view;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("QTableView Demo");
    mainWindow.resize(800, 600);
    
    // Create central widget with layout
    QWidget* centralWidget = new QWidget(&mainWindow);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

//    auto tableView = make_qtable_view(centralWidget);
    auto tableView = make_vtable_view(centralWidget);
//    auto tableView = make_stable_view(centralWidget);
    
    // Add TableView to layout
    layout->addWidget(tableView);
    
    // Set central widget and show
    mainWindow.setCentralWidget(centralWidget);
    mainWindow.show();
    
  application.exec();
}
