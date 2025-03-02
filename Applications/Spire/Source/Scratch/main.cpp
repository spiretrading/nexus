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
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"

using namespace Spire;

class TableViewModel : public QAbstractItemModel {
    
public:
    using QAbstractItemModel::QAbstractItemModel;

    // Standard model API from QAbstractItemModel
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override = 0;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override = 0;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override = 0;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override = 0;
    virtual QModelIndex parent(const QModelIndex& index) const override = 0;
};

class SimpleTableViewModel : public TableViewModel {
    
public:
    explicit SimpleTableViewModel(QObject* parent = nullptr) : TableViewModel(parent) {}
    
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
            int j = index.row();
            int i = index.column();
            
            return QVariant(5 * j + i);
        }
        
        return QVariant();
    }
    
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid())
            return QModelIndex();
        
        if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount())
            return QModelIndex();
        
        return createIndex(row, column);
    }
    
    QModelIndex parent(const QModelIndex&) const override {
        return QModelIndex();
    }
};

class TableViewDelegate : public QObject {
    
public:
    explicit TableViewDelegate(QObject* parent = nullptr) : QObject(parent) {}
    
    // Widget factory and update methods
    virtual QWidget* createWidgetForCell(const QModelIndex& index, QWidget* parent) const = 0;
    virtual void updateWidget(QWidget* widget, const QModelIndex& index) const = 0;
    
    // Widget recycling
    virtual bool canReuseWidget(QWidget* widget, const QModelIndex& index) const = 0;
    virtual void prepareForReuse(QWidget* widget, const QModelIndex& index) const {
        // Default implementation does nothing, derived classes can override
    }
};

class LabelTableViewDelegate : public TableViewDelegate {
    
public:
    explicit LabelTableViewDelegate(QObject* parent = nullptr) : TableViewDelegate(parent) {}
    
    QWidget* createWidgetForCell(const QModelIndex& index, QWidget* parent) const override {
        auto label = make_label(std::make_shared<LocalTextModel>());
        
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


class VTableView : public QAbstractScrollArea {

public:
    explicit VTableView(QWidget* parent = nullptr);
    ~VTableView() override;

    void setModel(TableViewModel* model);
    void setDelegate(TableViewDelegate* delegate);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    // Key structures
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
    
    QWidget* createOrUpdateWidgetAt(int row, int column);
    void recycleWidget(QWidget* widget);
    CellGeometry cellGeometryAt(int row, int column) const;
    QPair<CellPosition, CellPosition> visibleCellRange() const;
    
    void cleanupInvisibleWidgets();
    void layoutVisibleWidgets();

    // Core data
    TableViewModel* m_model = nullptr;
    TableViewDelegate* m_delegate = nullptr;
    
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

// Implementation

VTableView::VTableView(QWidget* parent) : QAbstractScrollArea(parent) {
    // Set scroll policy and initialize viewport
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Set viewport background
    viewport()->setBackgroundRole(QPalette::Base);
    setFrameStyle(QFrame::NoFrame);
}

VTableView::~VTableView() {
    // Clean up all widgets
    qDeleteAll(m_visibleWidgets);
    qDeleteAll(m_recycledWidgets);
}

void VTableView::setModel(TableViewModel* model) {
    if (m_model == model)
        return;
        
    // Clean up existing widgets
    qDeleteAll(m_visibleWidgets);
    m_visibleWidgets.clear();
    qDeleteAll(m_recycledWidgets);
    m_recycledWidgets.clear();
    
    m_model = model;
    
    if (m_model) {
        // Connect model signals for updates
        connect(m_model, &QAbstractItemModel::dataChanged, 
                this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
            // Update affected widgets
            for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                for (int col = topLeft.column(); col <= bottomRight.column(); ++col) {
                    CellPosition pos{row, col};
                    if (m_visibleWidgets.contains(pos)) {
                        QModelIndex index = m_model->index(row, col);
                        m_delegate->updateWidget(m_visibleWidgets[pos], index);
                    }
                }
            }
        });
        
        connect(m_model, &QAbstractItemModel::rowsInserted, 
                this, [this]() { updateContentsSize(); updateVisibleRange(); });
        connect(m_model, &QAbstractItemModel::rowsRemoved, 
                this, [this]() { updateContentsSize(); updateVisibleRange(); });
        connect(m_model, &QAbstractItemModel::columnsInserted, 
                this, [this]() { updateContentsSize(); updateVisibleRange(); });
        connect(m_model, &QAbstractItemModel::columnsRemoved, 
                this, [this]() { updateContentsSize(); updateVisibleRange(); });
        connect(m_model, &QAbstractItemModel::modelReset, 
                this, [this]() { updateContentsSize(); updateVisibleRange(); });
    }
    
    updateContentsSize();
    updateVisibleRange();
    viewport()->update();
}

void VTableView::setDelegate(TableViewDelegate* delegate) {
    if (m_delegate == delegate)
        return;
        
    m_delegate = delegate;
    
    // Update all visible widgets with the new delegate
    if (m_delegate && m_model) {
        for (auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end(); ++it) {
            int row = it.key().row;
            int col = it.key().column;
            QModelIndex index = m_model->index(row, col);
            m_delegate->updateWidget(it.value(), index);
        }
    }
}

void VTableView::paintEvent(QPaintEvent* event) {
    if (!m_model || !m_delegate)
        return;
    
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(event->rect(), viewport()->palette().brush(QPalette::Base));
    
    // Draw grid lines if needed
    if (m_borderWidth > 0) {
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        
        auto range = visibleCellRange();
        for (int row = range.first.row; row <= range.second.row + 1; ++row) {
            for (int col = range.first.column; col <= range.second.column + 1; ++col) {
                auto geometry = cellGeometryAt(row, col);
                
                // Draw horizontal line
                painter.drawLine(0, geometry.y, viewport()->width(), geometry.y);
                
                // Draw vertical line
                painter.drawLine(geometry.x, 0, geometry.x, viewport()->height());
            }
        }
    }
    
    // The widgets are already in place, let them draw themselves
}

void VTableView::resizeEvent(QResizeEvent* event) {
    QAbstractScrollArea::resizeEvent(event);
    updateVisibleRange();
    updateScrollBarRanges();
}

void VTableView::scrollContentsBy(int dx, int dy) {
    QAbstractScrollArea::scrollContentsBy(dx, dy);
    updateVisibleRange();
}

void VTableView::updateContentsSize() {
    if (!m_model)
        return;
        
    int rows = m_model->rowCount();
    int cols = m_model->columnCount();
    
    // Calculate total content size
    int contentWidth = cols * (m_defaultCellSize.width() + m_cellSpacing) + m_cellSpacing;
    int contentHeight = rows * (m_defaultCellSize.height() + m_cellSpacing) + m_cellSpacing;
    
    // Update scrollbars
    horizontalScrollBar()->setRange(0, qMax(0, contentWidth - viewport()->width()));
    verticalScrollBar()->setRange(0, qMax(0, contentHeight - viewport()->height()));
    
    // Update page steps
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setPageStep(viewport()->height());
}

void VTableView::updateScrollBarRanges() {
    updateContentsSize();
}

QPair<VTableView::CellPosition, VTableView::CellPosition> VTableView::visibleCellRange() const {
    if (!m_model) {
        return {{0, 0}, {0, 0}};
    }
    
    int xOffset = horizontalScrollBar()->value();
    int yOffset = verticalScrollBar()->value();
    
    int cellWidth = m_defaultCellSize.width() + m_cellSpacing;
    int cellHeight = m_defaultCellSize.height() + m_cellSpacing;
    
    int firstVisibleRow = qMax(0, yOffset / cellHeight);
    int firstVisibleCol = qMax(0, xOffset / cellWidth);
    
    int lastVisibleRow = qMin(m_model->rowCount() - 1, 
                          (yOffset + viewport()->height()) / cellHeight + 1);
    int lastVisibleCol = qMin(m_model->columnCount() - 1, 
                          (xOffset + viewport()->width()) / cellWidth + 1);
    
    return {{firstVisibleRow, firstVisibleCol}, {lastVisibleRow, lastVisibleCol}};
}

void VTableView::updateVisibleRange() {
    if (!m_model || !m_delegate)
        return;
    
    auto visibleRange = visibleCellRange();
    
    // Create set of currently visible positions
    QSet<CellPosition> newVisiblePositions;
    for (int row = visibleRange.first.row; row <= visibleRange.second.row; ++row) {
        for (int col = visibleRange.first.column; col <= visibleRange.second.column; ++col) {
            newVisiblePositions.insert({row, col});
        }
    }
    
    // Find positions that are no longer visible
    QList<CellPosition> positionsToRemove;
    for (auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end(); ++it) {
        if (!newVisiblePositions.contains(it.key())) {
            positionsToRemove.append(it.key());
        }
    }
    
    // Recycle widgets for positions no longer visible
    for (const auto& pos : positionsToRemove) {
        recycleWidget(m_visibleWidgets.take(pos));
    }
    
    // Create widgets for newly visible positions
    for (int row = visibleRange.first.row; row <= visibleRange.second.row; ++row) {
        for (int col = visibleRange.first.column; col <= visibleRange.second.column; ++col) {
            CellPosition pos{row, col};
            if (!m_visibleWidgets.contains(pos)) {
                createOrUpdateWidgetAt(row, col);
            }
        }
    }
    
    // Update positions of all visible widgets
    layoutVisibleWidgets();
}

QWidget* VTableView::createOrUpdateWidgetAt(int row, int column) {
    if (!m_model || !m_delegate)
        return nullptr;
    
    QModelIndex index = m_model->index(row, column);
    if (!index.isValid())
        return nullptr;
    
    CellPosition pos{row, column};
    
    // Check if we already have a widget for this position
    if (m_visibleWidgets.contains(pos)) {
        m_delegate->updateWidget(m_visibleWidgets[pos], index);
        return m_visibleWidgets[pos];
    }
    
    // Try to reuse a recycled widget
    QWidget* widget = nullptr;
    
    // Find a widget we can reuse
    for (int i = 0; i < m_recycledWidgets.size(); ++i) {
        if (m_delegate->canReuseWidget(m_recycledWidgets[i], index)) {
            widget = m_recycledWidgets.takeAt(i);
            m_delegate->prepareForReuse(widget, index);
            break;
        }
    }
    
    // If no reusable widget, create a new one
    if (!widget) {
        widget = m_delegate->createWidgetForCell(index, viewport());
    }
    
    // Update with current data
    m_delegate->updateWidget(widget, index);
    
    // Store and show the widget
    widget->setParent(viewport());
    m_visibleWidgets[pos] = widget;
    
    // Position will be set in layoutVisibleWidgets
    
    return widget;
}

void VTableView::recycleWidget(QWidget* widget) {
    if (!widget)
        return;
    
    widget->hide();
    m_recycledWidgets.append(widget);
}

VTableView::CellGeometry VTableView::cellGeometryAt(int row, int column) const {
    int xOffset = horizontalScrollBar()->value();
    int yOffset = verticalScrollBar()->value();
    
    int x = column * (m_defaultCellSize.width() + m_cellSpacing) + m_cellSpacing - xOffset;
    int y = row * (m_defaultCellSize.height() + m_cellSpacing) + m_cellSpacing - yOffset;
    
    return {x, y, m_defaultCellSize.width(), m_defaultCellSize.height()};
}

void VTableView::layoutVisibleWidgets() {
    for (auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end(); ++it) {
        CellPosition pos = it.key();
        QWidget* widget = it.value();
        
        auto geometry = cellGeometryAt(pos.row, pos.column);
        widget->setGeometry(geometry.x, geometry.y, geometry.width, geometry.height);
        widget->show();
    }
}

void VTableView::cleanupInvisibleWidgets() {
    auto visibleRange = visibleCellRange();
    
    for (auto it = m_visibleWidgets.begin(); it != m_visibleWidgets.end();) {
        CellPosition pos = it.key();
        if (pos.row < visibleRange.first.row || pos.row > visibleRange.second.row ||
            pos.column < visibleRange.first.column || pos.column > visibleRange.second.column) {
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

auto make_vtable_view(QWidget* parent) {
    // Create our TableView
    VTableView* tableView = new VTableView(parent);
    
    // Create and set the model
    SimpleTableViewModel* model = new SimpleTableViewModel(tableView);
    tableView->setModel(model);
    
    // Create and set the delegate
    LabelTableViewDelegate* delegate = new LabelTableViewDelegate(tableView);
    tableView->setDelegate(delegate);
    return tableView;
}

auto make_stable_view(QWidget* parent) {
  auto table = std::make_shared<ArrayTableModel>();
  for(auto i = 0; i != 10000; ++i) {
    auto row = std::vector<std::any>();
    for(auto j = 0; j != 5; ++j) {
      row.push_back(5 * i + j);
    }
    table->push(row);
  }
  auto builder = TableViewBuilder(table);
  builder.add_header_item("1");
  builder.add_header_item("2");
  builder.add_header_item("3");
  builder.add_header_item("4");
  builder.add_header_item("5");
  builder.set_item_builder(
    [] (const std::shared_ptr<TableModel>& table, int row, int column) {
      auto label = make_label(make_to_text_model(
        make_table_value_model<int>(table, row, column)));
      label->setFixedHeight(40);
      return label;
    });
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
