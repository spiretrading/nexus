#include "Spire/Dashboard/DashboardWidget.hpp"
#include <QApplication>
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
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  const auto REPAINT_INTERVAL = 300;
}

class DashboardWidget::Header : public QWidget {
  public:
    explicit Header(DashboardWidget& dashboard)
        : m_dashboard(&dashboard) {
      setMouseTracking(true);
      setFocusPolicy(Qt::NoFocus);
      setAttribute(Qt::WA_OpaquePaintEvent);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    QSize sizeHint() const override {
      if(!m_dashboard->m_renderer) {
        return QWidget::sizeHint();
      }
      return QSize(m_dashboard->GetContentWidth(),
        std::max(1, m_dashboard->m_renderer->GetMaxRowHeight()));
    }

    void Refresh() {
      auto hint = sizeHint();
      if(hint != m_sizeHint) {
        m_sizeHint = hint;
        updateGeometry();
      }
      update();
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
      auto painter = QPainter(this);
      painter.setClipRect(event->rect());
      if(!m_dashboard->m_renderer) {
        painter.fillRect(
          event->rect(), palette().color(QPalette::Window));
        return;
      }
      auto dashboardPainter = DashboardPainter(painter);
      m_dashboard->m_renderer->DrawHeader(
        dashboardPainter, QRect(0, 0, width(), height()));
    }

    void mouseMoveEvent(QMouseEvent* event) override {
      if(m_dashboard->m_mouseState == MouseState::RESIZING_COLUMN) {
        m_dashboard->ResizeColumn(*event);
      } else if(m_dashboard->m_mouseState == MouseState::MOVING_COLUMN) {
        m_dashboard->MoveColumn(*event);
      } else {
        m_dashboard->TestHoveringColumnExpansion(*event);
      }
    }

    void mousePressEvent(QMouseEvent* event) override {
      if(event->button() != Qt::LeftButton) {
        return;
      }
      if(m_dashboard->m_isHoveringOverColumnResize) {
        m_dashboard->m_mouseState = MouseState::RESIZING_COLUMN;
        return;
      }
      m_dashboard->m_lastMousePressPosition = event->pos();
      m_dashboard->m_activeColumnIndex = m_dashboard->GetColumnAt(event->pos());
      m_dashboard->m_mouseState = MouseState::MOVING_COLUMN;
      setCursor(Qt::ClosedHandCursor);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
      if(m_dashboard->m_mouseState == MouseState::RESIZING_COLUMN) {
        m_dashboard->m_mouseState = MouseState::NONE;
        m_dashboard->TestHoveringColumnExpansion(*event);
      } else if(m_dashboard->m_mouseState == MouseState::MOVING_COLUMN) {
        m_dashboard->m_mouseState = MouseState::NONE;
        setCursor(Qt::ArrowCursor);
        m_dashboard->TestHoveringColumnExpansion(*event);
        auto position = event->pos();
        if(std::abs(position.x() -
              m_dashboard->m_lastMousePressPosition.x()) <= 5 &&
            std::abs(position.y() -
              m_dashboard->m_lastMousePressPosition.y()) <= 5) {
          m_dashboard->ModifyColumnSortOrder(
            m_dashboard->GetColumnAt(position));
        }
      }
    }

  private:
    DashboardWidget* m_dashboard;
    QSize m_sizeHint;
};

class DashboardWidget::Body : public QWidget {
  public:
    explicit Body(DashboardWidget& dashboard)
        : m_dashboard(&dashboard) {
      setMouseTracking(true);
      setFocusPolicy(Qt::NoFocus);
      setAttribute(Qt::WA_OpaquePaintEvent);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    }

    QSize sizeHint() const override {
      if(!m_dashboard->m_renderer) {
        return QWidget::sizeHint();
      }
      return QSize(m_dashboard->GetContentWidth(),
        m_dashboard->m_renderer->GetSize() *
          std::max(1, m_dashboard->m_renderer->GetMaxRowHeight()));
    }

    void Refresh() {
      auto hint = sizeHint();
      if(hint != m_sizeHint) {
        m_sizeHint = hint;
        updateGeometry();
      }
      update();
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
      auto painter = QPainter(this);
      painter.setClipRect(event->rect());
      if(!m_dashboard->m_renderer) {
        painter.fillRect(
          event->rect(), palette().color(QPalette::Window));
        return;
      }
      auto dashboardPainter = DashboardPainter(painter);
      m_dashboard->m_renderer->Draw(
        dashboardPainter, QRect(0, 0, width(), height()));
    }

    void mousePressEvent(QMouseEvent* event) override {
      if(auto rowIndex = m_dashboard->GetRowDisplayIndex(event->pos())) {
        m_dashboard->m_selectionController->HandleMouseEvent(
          *event, *rowIndex);
      }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
      if(auto rowIndex = m_dashboard->GetRowDisplayIndex(event->pos())) {
        m_dashboard->m_selectionController->HandleMouseEvent(
          *event, *rowIndex);
      }
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override {
      auto rowIndex = m_dashboard->GetRowDisplayIndex(event->pos());
      if(!rowIndex) {
        return;
      }
      if(m_dashboard->m_selectionController->HandleMouseEvent(
          *event, *rowIndex)) {
        return;
      }
      m_dashboard->ActivateRow(*rowIndex);
    }

    void resizeEvent(QResizeEvent* event) override {
      m_dashboard->StretchLastColumn(width());
    }

  private:
    DashboardWidget* m_dashboard;
    QSize m_sizeHint;
};

struct DashboardWidget::RowComparator {
  std::vector<SortOrder>* m_columnSortOrder;

  RowComparator(std::vector<SortOrder>* columnSortOrder)
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

  RendererComparator(
    DashboardRenderer* renderer, std::vector<SortOrder>* columnSortOrder)
    : m_renderer(renderer),
      m_comparator(columnSortOrder) {}

  bool operator ()(int lhs, int rhs) const {
    auto leftRow = m_renderer->GetRow(lhs);
    if(!leftRow.is_initialized()) {
      return false;
    }
    auto rightRow = m_renderer->GetRow(rhs);
    if(!rightRow.is_initialized()) {
      return true;
    }
    return m_comparator(*leftRow, *rightRow);
  }
};

DashboardWidget::DashboardWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_model(nullptr),
      m_userProfile(nullptr),
      m_tickerDialog(nullptr),
      m_activateRowIndex(0),
      m_selectionModel(std::make_unique<DashboardSelectionModel>()),
      m_selectionController(
        std::make_unique<DashboardSelectionController>(Ref(*m_selectionModel))),
      m_isHoveringOverColumnResize(false),
      m_mouseState(MouseState::NONE),
      m_hasRepaintEvent(false),
      m_isSortOrderStale(false) {
  setFocusPolicy(Qt::StrongFocus);
  m_header = new Header(*this);
  m_body = new Body(*this);
  m_scrollBox = new ScrollBox(m_body);
  m_scrollBox->set(ScrollBox::DisplayPolicy::ON_ENGAGE);
  m_scrollBox->setFocusPolicy(Qt::NoFocus);
  m_scrollBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_vbox_layout(this);
  layout->addWidget(m_header);
  layout->addWidget(m_scrollBox);
  connect(
    &m_repaintTimer, &QTimer::timeout, this, &DashboardWidget::OnRepaintTimer);
  m_repaintTimer.start(REPAINT_INTERVAL);
}

void DashboardWidget::Initialize(Ref<DashboardModel> model,
    const DashboardRowBuilder& rowBuilder, Ref<UserProfile> userProfile) {
  m_model = model.get();
  m_rowBuilder = rowBuilder.Clone();
  m_userProfile = userProfile.get();
  auto rowRenderer = [=] (const DashboardRow& row) {
    auto renderer = std::make_unique<DashboardRowRenderer>(Ref(row),
      [=, &row] (const DashboardCell& cell) ->
          std::unique_ptr<DashboardCellRenderer> {
        if(&cell == &row.GetCell(2)) {
          auto cellRenderer =
            std::make_unique<DirectionalDashboardCellRenderer>(
              Ref(cell), Ref(*m_userProfile));
          auto font = cellRenderer->GetFont();
          if(&cell == &row.GetIndex()) {
            font.setPointSize(12);
          } else {
            font.setPointSize(11);
          }
          cellRenderer->SetFont(font);
          return std::move(cellRenderer);
        } else if(&cell == &row.GetCell(3)) {
          auto cellRenderer = std::make_unique<PercentageDashboardCellRenderer>(
            Ref(cell), Ref(*m_userProfile));
          auto font = cellRenderer->GetFont();
          if(&cell == &row.GetIndex()) {
            font.setPointSize(12);
          } else {
            font.setPointSize(11);
          }
          cellRenderer->SetFont(font);
          return cellRenderer;
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
          return cellRenderer;
        }
      });
    return renderer;
  };
  m_renderer = std::make_unique<DashboardRenderer>(
    Ref(*m_model), Ref(*m_selectionModel), rowRenderer, Ref(*m_userProfile));
  auto p = QPalette(palette());
  p.setColor(QPalette::Window, QColor{13, 13, 13});
  setPalette(p);
  StretchLastColumn(m_body->width());
  m_drawConnection = m_renderer->ConnectDrawSignal(
    std::bind_front(&DashboardWidget::OnDrawSignal, this));
  m_selectedRowsConnection = m_selectionModel->ConnectSelectedRowsUpdatedSignal(
    std::bind_front(&DashboardWidget::OnSelectedRowsUpdatedSignal, this));
  m_activeRowConnection = m_selectionModel->ConnectActiveRowUpdatedSignal(
    std::bind_front(&DashboardWidget::OnActiveRowUpdatedSignal, this));
  m_cellUpdateConnections.disconnect();
  m_rowAddedConnection = m_model->ConnectRowAddedSignal(
    std::bind_front(&DashboardWidget::OnRowAddedSignal, this));
  m_rowRemovedConnection = m_model->ConnectRowRemovedSignal(
    std::bind_front(&DashboardWidget::OnRowRemovedSignal, this));
  for(auto i = 0; i < m_model->GetRowCount(); ++i) {
    OnRowAddedSignal(m_model->GetRow(i));
  }
  m_tickerDialog =
    new TickerDialog(m_userProfile->GetTickerInfoQueryModel(), this);
  m_tickerDialog->connect_submit_signal(
    std::bind_front(&DashboardWidget::OnTickerSubmit, this));
  m_header->Refresh();
  m_body->Refresh();
  m_hasRepaintEvent = true;
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

optional<int> DashboardWidget::GetRowDisplayIndex(
    const QPoint& position) const {
  if(!m_renderer || position.y() < 0) {
    return none;
  }
  return position.y() / std::max(1, m_renderer->GetMaxRowHeight());
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
    if(!activeRow) {
      return QWidget::keyPressEvent(event);
    }
    auto text = event->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      return QWidget::keyPressEvent(event);
    }
    ActivateRow(*activeRow);
    QApplication::sendEvent(find_focus_proxy(*m_tickerDialog), event);
  }
}

void DashboardWidget::keyReleaseEvent(QKeyEvent* event) {
  m_selectionController->HandleKeyEvent(*event);
}

void DashboardWidget::showEvent(QShowEvent* event) {
  m_hasRepaintEvent = true;
  m_repaintTimer.start(REPAINT_INTERVAL);
  QWidget::showEvent(event);
}

void DashboardWidget::hideEvent(QHideEvent* event) {
  m_repaintTimer.stop();
  QWidget::hideEvent(event);
}

int DashboardWidget::GetContentWidth() const {
  if(!m_model) {
    return 0;
  }
  auto width = 0;
  for(auto i = 0; i < m_model->GetColumnCount(); ++i) {
    width += m_renderer->GetColumnWidth(i);
  }
  return width;
}

void DashboardWidget::ScrollToRow(int index) {
  auto height = std::max(1, m_renderer->GetMaxRowHeight());
  auto top = index * height;
  auto& scrollBar = m_scrollBox->get_vertical_scroll_bar();
  auto position = scrollBar.get_position();
  if(top < position) {
    scrollBar.set_position(top);
  } else if(top + height > position + scrollBar.get_page_size()) {
    scrollBar.set_position(top + height - scrollBar.get_page_size());
  }
}

void DashboardWidget::StretchLastColumn(int width) {
  if(!m_model) {
    return;
  }
  auto emptySpace = width - GetContentWidth();
  auto lastColumnIndex = m_model->GetColumnCount() - 1;
  if(emptySpace > 0) {
    m_renderer->SetColumnWidth(lastColumnIndex,
      m_renderer->GetColumnWidth(lastColumnIndex) + emptySpace);
  } else if(emptySpace < 0) {
    auto lastColumnWidth = m_renderer->GetColumnWidth(lastColumnIndex);
    auto adjustedWidth = std::max(
      m_renderer->GetDefaultColumnWidth() / 2, lastColumnWidth + emptySpace);
    if(lastColumnWidth != adjustedWidth) {
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
  auto sortOrder = SortOrder();
  sortOrder.m_index = modelIndex;
  sortOrder.m_direction = DashboardWidget::SortOrder::Order::DESCENDING;
  m_columnSortOrder.insert(m_columnSortOrder.begin(), sortOrder);
  SortRows();
}

void DashboardWidget::SortRows() {
  m_isSortOrderStale = false;
  if(m_columnSortOrder.empty()) {
    return;
  }
  auto indicies = std::vector<int>();
  indicies.reserve(m_renderer->GetSize());
  for(auto i = 0; i < static_cast<int>(m_renderer->GetSize()); ++i) {
    indicies.push_back(i);
  }
  std::stable_sort(indicies.begin(), indicies.end(),
    RendererComparator(&*m_renderer, &m_columnSortOrder));
  m_renderer->ReorderRows(indicies);
}

void DashboardWidget::ActivateRow(int index) {
  m_activateRowIndex = index;
  m_tickerDialog->show();
}

void DashboardWidget::OnTickerSubmit(const Ticker& ticker) {
  m_tickerDialog->hide();
  if(!ticker) {
    return;
  }
  auto index = m_activateRowIndex;
  for(auto i = m_renderer->GetSize(); i < index; ++i) {
    m_renderer->InsertEmptyRow(i);
  }
  setUpdatesEnabled(false);
  if(auto existingRow = m_renderer->GetRow(index)) {
    m_model->Remove(*existingRow);
  }
  auto row = m_rowBuilder->Make(ticker, Ref(*m_userProfile));
  m_model->Add(std::move(row));
  auto insertIndex = m_renderer->GetSize() - 1;
  m_renderer->MoveRow(insertIndex, index);
  setUpdatesEnabled(true);
}

void DashboardWidget::DeleteSelectedRows() {
  auto selectedRows = std::vector<const DashboardRow*>();
  for(auto& index : m_selectionModel->GetSelectedRows()) {
    if(auto row = m_renderer->GetRow(index)) {
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
      m_header->setCursor(Qt::ArrowCursor);
    }
    return;
  }
  auto columnWidthAccumulator = 0;
  auto isHovering = false;
  auto resizeColumnIndex = 0;
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
    m_header->setCursor(Qt::SplitHCursor);
  } else {
    m_header->setCursor(Qt::ArrowCursor);
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
      auto block = shared_connection_block(m_drawConnection);
      m_renderer->SetColumnWidth(m_activeColumnIndex, updatedLeftColumnSize);
      m_renderer->SetColumnWidth(m_activeColumnIndex + 1,
        m_renderer->GetColumnWidth(m_activeColumnIndex + 1) + leftColumnDelta);
    }
    m_hasRepaintEvent = true;
  } else if(delta > 0) {
    auto currentRightColumnSize =
      m_renderer->GetColumnWidth(m_activeColumnIndex + 1);
    auto updatedRightColumnSize = std::max(
      currentRightColumnSize - delta, m_renderer->GetMinimunColumnWidth());
    auto rightColumnDelta = currentRightColumnSize - updatedRightColumnSize;
    if(rightColumnDelta != 0) {
      auto block = shared_connection_block(m_drawConnection);
      m_renderer->SetColumnWidth(m_activeColumnIndex,
        m_renderer->GetColumnWidth(m_activeColumnIndex) + rightColumnDelta);
      m_renderer->SetColumnWidth(
        m_activeColumnIndex + 1, updatedRightColumnSize);
    }
    m_hasRepaintEvent = true;
  }
}

void DashboardWidget::MoveColumn(const QMouseEvent& event) {
  auto position = event.pos();
  auto currentIndex = GetColumnAt(position);
  if(currentIndex != m_activeColumnIndex) {
    m_renderer->MoveColumn(m_activeColumnIndex, currentIndex);
    m_activeColumnIndex = currentIndex;
  }
}

void DashboardWidget::OnRowAddedSignal(const DashboardRow& row) {
  for(auto i = 0; i < row.GetSize(); ++i) {
    m_cellUpdateConnections.add(&row,
      row.GetCell(i).ConnectUpdateSignal(std::bind_front(
        &DashboardWidget::OnCellUpdatedSignal, this, std::ref(row))));
  }
}

void DashboardWidget::OnRowRemovedSignal(const DashboardRow& row) {
  m_cellUpdateConnections.disconnect(&row);
}

void DashboardWidget::OnCellUpdatedSignal(
    const DashboardRow& row, const DashboardCell::Value& value) {
  if(m_columnSortOrder.empty()) {
    return;
  }
  m_isSortOrderStale = true;
}

void DashboardWidget::OnActiveRowUpdatedSignal(optional<int> activeRow) {
  if(activeRow) {
    ScrollToRow(*activeRow);
  }
  m_body->update();
}

void DashboardWidget::OnSelectedRowsUpdatedSignal() {
  m_body->update();
}

void DashboardWidget::OnDrawSignal() {
  m_hasRepaintEvent = true;
}

void DashboardWidget::OnRepaintTimer() {
  if(m_isSortOrderStale) {
    SortRows();
  }
  if(!m_hasRepaintEvent) {
    return;
  }
  m_hasRepaintEvent = false;
  m_header->Refresh();
  m_body->Refresh();
}
