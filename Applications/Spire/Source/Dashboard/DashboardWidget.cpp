#include "Spire/Dashboard/DashboardWidget.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Dashboard/DashboardModel.hpp"
#include "Spire/Dashboard/DashboardRenderer.hpp"
#include "Spire/Dashboard/DashboardRow.hpp"
#include "Spire/Dashboard/DashboardRowBuilder.hpp"
#include "Spire/Dashboard/DashboardSelectionController.hpp"
#include "Spire/Dashboard/DashboardSelectionModel.hpp"
#include "Spire/Dashboard/DashboardWidgetWindowSettings.hpp"
#include "Spire/Dashboard/DirectionalDashboardCellRenderer.hpp"
#include "Spire/Dashboard/PercentageDashboardCellRenderer.hpp"
#include "Spire/Dashboard/TextDashboardCellRenderer.hpp"
#include "Spire/Dashboard/ValueDashboardCell.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const auto REPAINT_INTERVAL = 300;
}

struct DashboardWidget::RowComparator {
  vector<SortOrder>* m_columnSortOrder;

  RowComparator(vector<SortOrder>* columnSortOrder)
      : m_columnSortOrder(columnSortOrder) {}

  bool operator ()(const DashboardRow& lhs, const DashboardRow& rhs) const {
    for(auto& sortOrder : *m_columnSortOrder) {
      auto& leftValues = lhs.GetCell(sortOrder.m_index).GetValues();
      auto& rightValues = rhs.GetCell(sortOrder.m_index).GetValues();
      if(leftValues.empty()) {
        return false;
      } else if(rightValues.empty()) {
        return true;
      }
      if(sortOrder.m_direction == SortOrder::Order::ASCENDING) {
        if(leftValues.back() < rightValues.back()) {
          return true;
        } else if(leftValues.back() > rightValues.back()) {
          return false;
        }
      } else {
        if(rightValues.back() < leftValues.back()) {
          return true;
        } else if(rightValues.back() > leftValues.back()) {
          return false;
        }
      }
    }
    return false;
  }
};

struct DashboardWidget::RendererComparator {
  DashboardRenderer* m_renderer;
  RowComparator m_comparator;

  RendererComparator(DashboardRenderer* renderer,
      vector<SortOrder>* columnSortOrder)
      : m_renderer{renderer},
        m_comparator(columnSortOrder) {}

  bool operator ()(int lhs, int rhs) const {
    if(!m_renderer->GetRow(lhs).is_initialized()) {
      return false;
    } else if(!m_renderer->GetRow(rhs).is_initialized()) {
      return true;
    }
    auto& leftRow = *m_renderer->GetRow(lhs);
    auto& rightRow = *m_renderer->GetRow(rhs);
    return m_comparator(leftRow, rightRow);
  }
};

DashboardWidget::DashboardWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_model{nullptr},
      m_userProfile{nullptr},
      m_selectionModel{std::make_unique<DashboardSelectionModel>()},
      m_selectionController{std::make_unique<DashboardSelectionController>(
        Ref(*m_selectionModel))},
      m_isHoveringOverColumnResize{false},
      m_mouseState{MouseState::NONE} {
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  connect(&m_repaintTimer, &QTimer::timeout, this,
    &DashboardWidget::OnRepaintTimer);
  m_repaintTimer.start(REPAINT_INTERVAL);
}

DashboardWidget::~DashboardWidget() {}

void DashboardWidget::Initialize(Ref<DashboardModel> model,
    const DashboardRowBuilder& rowBuilder, Ref<UserProfile> userProfile) {
  m_model = model.Get();
  m_rowBuilder = rowBuilder.Clone();
  m_userProfile = userProfile.Get();
  auto rowRenderer = [=] (const DashboardRow& row) {
    auto renderer = std::make_unique<DashboardRowRenderer>(Ref(row),
      [=, &row] (const DashboardCell& cell) ->
          std::unique_ptr<DashboardCellRenderer> {
        if(&cell == &row.GetCell(2)) {
          auto cellRenderer =
            std::make_unique<DirectionalDashboardCellRenderer>(Ref(cell),
            Ref(*m_userProfile));
          auto font = cellRenderer->GetFont();
          if(&cell == &row.GetIndex()) {
            font.setPointSize(12);
          } else {
            font.setPointSize(11);
          }
          cellRenderer->SetFont(font);
          return std::move(cellRenderer);
        } else if(&cell == &row.GetCell(3)) {
          auto cellRenderer =
            std::make_unique<PercentageDashboardCellRenderer>(Ref(cell),
            Ref(*m_userProfile));
          auto font = cellRenderer->GetFont();
          if(&cell == &row.GetIndex()) {
            font.setPointSize(12);
          } else {
            font.setPointSize(11);
          }
          cellRenderer->SetFont(font);
          return std::move(cellRenderer);
        } else {
          auto cellRenderer = std::make_unique<TextDashboardCellRenderer>(
            Ref(cell), Ref(*m_userProfile));
          auto font = cellRenderer->GetFont();
          if(&cell == &row.GetIndex()) {
            font.setPointSize(12);
          } else {
            font.setPointSize(11);
          }
          cellRenderer->SetFont(font);
          return std::move(cellRenderer);
        }
      });
    return renderer;
  };
  m_renderer = std::make_unique<DashboardRenderer>(Ref(*m_model),
    Ref(*m_selectionModel), rowRenderer, Ref(*m_userProfile));
  QPalette p{palette()};
  p.setColor(QPalette::Window, QColor{13, 13, 13});
  setPalette(p);
  resizeEvent(nullptr);
  m_drawConnection = m_renderer->ConnectDrawSignal(
    std::bind(&DashboardWidget::OnDrawSignal, this));
  m_selectedRowsConnection = m_selectionModel->ConnectSelectedRowsUpdatedSignal(
    std::bind(&DashboardWidget::OnSelectedRowsUpdatedSignal, this));
  m_activeRowConnection = m_selectionModel->ConnectActiveRowUpdatedSignal(
    std::bind(&DashboardWidget::OnActiveRowUpdatedSignal, this,
    std::placeholders::_1));
  m_rowAddedConnection = m_model->ConnectRowAddedSignal(
    std::bind(&DashboardWidget::OnRowAddedSignal, this, std::placeholders::_1));
  for(auto i = 0; i < m_model->GetRowCount(); ++i) {
    OnRowAddedSignal(m_model->GetRow(i));
  }
}

const DashboardSelectionModel& DashboardWidget::GetSelectionModel() const {
  return *m_selectionModel;
}

DashboardSelectionModel& DashboardWidget::GetSelectionModel() {
  return *m_selectionModel;
}

const DashboardRowBuilder& DashboardWidget::GetRowBuilder() const {
  return *m_rowBuilder;
}

const DashboardRenderer& DashboardWidget::GetRenderer() const {
  return *m_renderer;
}

boost::optional<int> DashboardWidget::GetRowDisplayIndex(
    const QPoint& position) const {
  if(position.y() < m_renderer->GetMaxRowHeight()) {
    return none;
  }
  return (position.y() / m_renderer->GetMaxRowHeight()) - 1;
}

std::unique_ptr<WindowSettings> DashboardWidget::GetWindowSettings() const {
  return std::make_unique<DashboardWidgetWindowSettings>(*this);
}

void DashboardWidget::keyPressEvent(QKeyEvent* event) {
  if(m_selectionController->HandleKeyEvent(*event)) {
    return;
  } else if(event->key() == Qt::Key_Delete) {
    DeleteSelectedRows();
  } else {
    auto activeRow = m_selectionModel->GetActiveRow();
    if(!activeRow.is_initialized()) {
      return QWidget::keyPressEvent(event);
    }
    auto text = event->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      return QWidget::keyPressEvent(event);
    }
    ActivateRow(*activeRow, text.toStdString());
  }
}

void DashboardWidget::keyReleaseEvent(QKeyEvent* event) {
  m_selectionController->HandleKeyEvent(*event);
}

void DashboardWidget::mouseMoveEvent(QMouseEvent* event) {
  if(m_mouseState == MouseState::RESIZING_COLUMN) {
    ResizeColumn(*event);
  } else if(m_mouseState == MouseState::MOVING_COLUMN) {
    MoveColumn(*event);
  } else {
    TestHoveringColumnExpansion(*event);
  }
}

void DashboardWidget::mousePressEvent(QMouseEvent* event) {
  if(m_isHoveringOverColumnResize) {
    if(event->button() == Qt::LeftButton) {
      m_mouseState = MouseState::RESIZING_COLUMN;
      return;
    }
  }
  auto position = event->pos();
  if(position.y() < m_renderer->GetMaxRowHeight()) {
    if(event->button() == Qt::LeftButton) {
      m_lastMousePressPosition = position;
      m_activeColumnIndex = GetColumnAt(position);
      m_mouseState = MouseState::MOVING_COLUMN;
      setCursor(Qt::ClosedHandCursor);
      return;
    }
  }
  auto rowIndex = GetRowDisplayIndex(position);
  if(!rowIndex.is_initialized()) {
    return;
  }
  if(m_selectionController->HandleMouseEvent(*event, *rowIndex)) {
    return;
  }
}

void DashboardWidget::mouseReleaseEvent(QMouseEvent* event) {
  if(m_mouseState == MouseState::RESIZING_COLUMN) {
    m_mouseState = MouseState::NONE;
    TestHoveringColumnExpansion(*event);
    return;
  } else if(m_mouseState == MouseState::MOVING_COLUMN) {
    m_mouseState = MouseState::NONE;
    setCursor(Qt::ArrowCursor);
    TestHoveringColumnExpansion(*event);
    auto position = event->pos();
    if(std::abs(position.x() - m_lastMousePressPosition.x()) <= 5 &&
        std::abs(position.y() - m_lastMousePressPosition.y()) <= 5) {
      auto column = GetColumnAt(position);
      ModifyColumnSortOrder(column);
    }
    return;
  }
  auto position = event->pos();
  auto rowIndex = GetRowDisplayIndex(position);
  if(!rowIndex.is_initialized()) {
    return;
  }
  if(m_selectionController->HandleMouseEvent(*event, *rowIndex)) {
    return;
  }
}

void DashboardWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  auto position = event->pos();
  auto rowIndex = GetRowDisplayIndex(position);
  if(!rowIndex.is_initialized()) {
    return;
  }
  if(m_selectionController->HandleMouseEvent(*event, *rowIndex)) {
    return;
  }
  ActivateRow(*rowIndex, {});
}

void DashboardWidget::paintEvent(QPaintEvent* event) {
  if(m_model == nullptr) {
    return;
  }
  QRect region{0, 0, width(), height()};
  m_renderer->Draw(*this, region);
}

void DashboardWidget::resizeEvent(QResizeEvent* event) {
  auto width = 0;
  for(auto i = 0; i < m_model->GetColumnCount(); ++i) {
    width += m_renderer->GetColumnWidth(i);
  }
  auto emptySpace = size().width() - width;
  auto lastColumnIndex = m_model->GetColumnCount() - 1;
  if(emptySpace > 0) {
    m_renderer->SetColumnWidth(lastColumnIndex,
      m_renderer->GetColumnWidth(lastColumnIndex) + emptySpace);
  } else if(emptySpace < 0) {
    auto width = m_renderer->GetColumnWidth(lastColumnIndex);
    auto adjustedWidth = std::max(m_renderer->GetDefaultColumnWidth() / 2,
      width + emptySpace);
    if(width != adjustedWidth) {
      m_renderer->SetColumnWidth(lastColumnIndex, adjustedWidth);
    }
  }
}

void DashboardWidget::ModifyColumnSortOrder(int index) {
  auto modelIndex = m_renderer->GetColumnModelIndex(index);
  for(auto i = 0; i < static_cast<int>(m_columnSortOrder.size()); ++i) {
    auto& sortOrder = m_columnSortOrder[i];
    if(sortOrder.m_index == modelIndex) {
      auto revisedSortOrder = sortOrder;
      m_columnSortOrder.erase(m_columnSortOrder.begin() + i);
      if(revisedSortOrder.m_direction == SortOrder::Order::DESCENDING) {
        revisedSortOrder.m_direction = SortOrder::Order::ASCENDING;
        m_columnSortOrder.insert(m_columnSortOrder.begin(), revisedSortOrder);
        SortRows();
      }
      return;
    }
  }
  SortOrder sortOrder;
  sortOrder.m_index = modelIndex;
  sortOrder.m_direction = DashboardWidget::SortOrder::Order::DESCENDING;
  m_columnSortOrder.insert(m_columnSortOrder.begin(), sortOrder);
  SortRows();
}

void DashboardWidget::SortRows() {
  if(m_columnSortOrder.empty()) {
    return;
  }
  vector<int> indicies;
  for(auto i = 0; i < static_cast<int>(m_renderer->GetSize()); ++i) {
    indicies.push_back(i);
  }
  auto comparator = RowComparator{&m_columnSortOrder};
  std::sort(indicies.begin(), indicies.end(),
    RendererComparator{&*m_renderer, &m_columnSortOrder});
  m_renderer->ReorderRows(indicies);
}

void DashboardWidget::ActivateRow(int index, const string& prefix) {
  SecurityInputDialog dialog{Ref(*m_userProfile), prefix, this};
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto security = dialog.GetSecurity();
  if(security == Security{}) {
    return;
  }
  for(auto i = m_renderer->GetSize(); i < index; ++i) {
    m_renderer->InsertEmptyRow(i);
  }
  setUpdatesEnabled(false);
  auto existingRow = m_renderer->GetRow(index);
  if(existingRow.is_initialized()) {
    m_model->Remove(*existingRow);
  }
  auto row = m_rowBuilder->Make(security, Ref(*m_userProfile));
  m_model->Add(std::move(row));
  auto insertIndex = m_renderer->GetSize() - 1;
  m_renderer->MoveRow(insertIndex, index);
  setUpdatesEnabled(true);
}

void DashboardWidget::DeleteSelectedRows() {
  vector<const DashboardRow*> selectedRows;
  for(auto& index : m_selectionModel->GetSelectedRows()) {
    auto row = m_renderer->GetRow(index);
    if(row.is_initialized()) {
      selectedRows.push_back(&*row);
    }
  }
  for(auto& row : selectedRows) {
    m_model->Remove(*row);
  }
}

void DashboardWidget::TestHoveringColumnExpansion(const QMouseEvent& event) {
  const auto WIDTH_ADJUSTMENT_THRESHOLD = 3;
  auto position = event.pos();
  if(position.y() > m_renderer->GetMaxRowHeight()) {
    if(m_isHoveringOverColumnResize) {
      m_isHoveringOverColumnResize = false;
      setCursor(Qt::ArrowCursor);
    }
    return;
  }
  auto columnWidthAccumulator = 0;
  auto isHovering = false;
  int resizeColumnIndex;
  for(auto i = 0; i < m_renderer->GetModel().GetColumnCount(); ++i) {
    columnWidthAccumulator += m_renderer->GetColumnWidth(i);
    if(columnWidthAccumulator > position.x() + WIDTH_ADJUSTMENT_THRESHOLD) {
      break;
    }
    if(position.x() >= columnWidthAccumulator - WIDTH_ADJUSTMENT_THRESHOLD &&
        position.x() <= columnWidthAccumulator + WIDTH_ADJUSTMENT_THRESHOLD) {
      isHovering = true;
      resizeColumnIndex = i;
      break;
    }
  }
  if(isHovering == m_isHoveringOverColumnResize) {
    return;
  }
  m_isHoveringOverColumnResize = isHovering;
  if(m_isHoveringOverColumnResize) {
    m_activeColumnIndex = resizeColumnIndex;
    setCursor(Qt::SplitHCursor);
  } else {
    setCursor(Qt::ArrowCursor);
  }
}

int DashboardWidget::GetColumnAt(const QPoint& point) {
  auto columnWidthAccumulator = 0;
  for(auto i = 0; i <= m_model->GetColumnCount(); ++i) {
    columnWidthAccumulator += m_renderer->GetColumnWidth(i);
    if(columnWidthAccumulator > point.x()) {
      return i;
    }
  }
  return m_model->GetColumnCount() - 1;
}

void DashboardWidget::ResizeColumn(const QMouseEvent& event) {
  if(m_activeColumnIndex == m_model->GetColumnCount() - 1) {
    return;
  }
  auto columnWidthAccumulator = 0;
  for(auto i = 0; i <= m_activeColumnIndex; ++i) {
    columnWidthAccumulator += m_renderer->GetColumnWidth(i);
  }
  auto delta = event.pos().x() - columnWidthAccumulator;
  if(delta < 0) {
    auto currentLeftColumnSize = m_renderer->GetColumnWidth(
      m_activeColumnIndex);
    auto updatedLeftColumnSize = std::max(currentLeftColumnSize + delta,
      m_renderer->GetMinimunColumnWidth());
    auto leftColumnDelta = currentLeftColumnSize - updatedLeftColumnSize;
    if(leftColumnDelta != 0) {
      shared_connection_block block{m_drawConnection};
      m_renderer->SetColumnWidth(m_activeColumnIndex, updatedLeftColumnSize);
      m_renderer->SetColumnWidth(m_activeColumnIndex + 1,
        m_renderer->GetColumnWidth(m_activeColumnIndex + 1) + leftColumnDelta);
    }
    repaint();
  } else if(delta > 0) {
    auto currentRightColumnSize = m_renderer->GetColumnWidth(
      m_activeColumnIndex + 1);
    auto updatedRightColumnSize = std::max(currentRightColumnSize - delta,
      m_renderer->GetMinimunColumnWidth());
    auto rightColumnDelta = currentRightColumnSize - updatedRightColumnSize;
    if(rightColumnDelta != 0) {
      shared_connection_block block{m_drawConnection};
      m_renderer->SetColumnWidth(m_activeColumnIndex,
        m_renderer->GetColumnWidth(m_activeColumnIndex) + rightColumnDelta);
      m_renderer->SetColumnWidth(m_activeColumnIndex + 1,
        updatedRightColumnSize);
    }
    repaint();
  }
}

void DashboardWidget::MoveColumn(const QMouseEvent& event) {
  auto position = event.pos();
  auto currentIndex = GetColumnAt(position);
  if(currentIndex != m_activeColumnIndex) {
    m_renderer->MoveColumn(m_activeColumnIndex, currentIndex);
    m_activeColumnIndex = currentIndex;
    return;
  }
}

void DashboardWidget::OnRowAddedSignal(const DashboardRow& row) {
  for(auto i = 0; i < row.GetSize(); ++i) {
    m_cellUpdateConnections.AddConnection(row.GetCell(i).ConnectUpdateSignal(
      std::bind(&DashboardWidget::OnCellUpdatedSignal, this, std::ref(row),
      std::placeholders::_1)));
  }
}

void DashboardWidget::OnCellUpdatedSignal(const DashboardRow& row,
    const DashboardCell::Value& value) {
  if(m_columnSortOrder.empty()) {
    return;
  }
  auto rowIndex = m_renderer->GetRowDisplayIndex(row);
  auto previousRow = m_renderer->GetRow(rowIndex - 1);
  RowComparator comparator{&m_columnSortOrder};
  if(previousRow.is_initialized()) {
    if(!comparator(*previousRow, row)) {
      auto i = rowIndex - 1;
      while(i >= 0) {
        auto r = m_renderer->GetRow(i);
        if(r.is_initialized()) {
          if(comparator(*r, row)) {
            m_renderer->MoveRow(rowIndex, i);
            return;
          }
        }
        --i;
      }
      m_renderer->MoveRow(rowIndex, 0);
      return;
    }
  }
  auto followingRow = m_renderer->GetRow(rowIndex + 1);
  if(followingRow.is_initialized()) {
    if(!comparator(row, *followingRow)) {
      auto i = rowIndex + 1;
      while(i < m_renderer->GetSize()) {
        auto r = m_renderer->GetRow(i);
        if(r.is_initialized()) {
          if(comparator(row, *r)) {
            m_renderer->MoveRow(rowIndex, i);
            return;
          }
        }
        ++i;
      }
      m_renderer->MoveRow(rowIndex, m_renderer->GetSize() - 1);
      return;
    }
  }
}

void DashboardWidget::OnActiveRowUpdatedSignal(boost::optional<int> activeRow) {
  repaint();
}

void DashboardWidget::OnSelectedRowsUpdatedSignal() {
  repaint();
}

void DashboardWidget::OnDrawSignal() {
  m_hasRepaintEvent = true;
}

void DashboardWidget::OnRepaintTimer() {
  m_hasRepaintEvent = false;
  repaint();
}
