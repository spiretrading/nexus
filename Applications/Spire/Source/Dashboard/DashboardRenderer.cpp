#include "Spire/Dashboard/DashboardRenderer.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QPainter>
#include "Spire/Dashboard/DashboardModel.hpp"
#include "Spire/Dashboard/DashboardRowRenderer.hpp"
#include "Spire/Dashboard/DashboardSelectionModel.hpp"
#include "Spire/Dashboard/TextDashboardCellRenderer.hpp"
#include "Spire/Dashboard/ValueDashboardCell.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  DashboardRenderer::DashboardRowRendererBuilder
      GetDefaultHeaderRendererBuilder(Ref<UserProfile> userProfile) {
    auto rowRenderer = [=] (const DashboardRow& row) {
      auto renderer = std::make_unique<DashboardRowRenderer>(Ref(row),
        [=] (const DashboardCell& cell) {
          auto nameRenderer = std::make_unique<TextDashboardCellRenderer>(
            Ref(cell), userProfile);
          nameRenderer->SetAlignment(Qt::AlignCenter);
          auto font = nameRenderer->GetFont();
          font.setPointSize(11);
          nameRenderer->SetFont(font);
          return nameRenderer;
        });
      return renderer;
    };
    return rowRenderer;
  }
}

DashboardRenderer::DashboardRenderer(Ref<const DashboardModel> model,
    Ref<const DashboardSelectionModel> selectionModel,
    const DashboardRowRendererBuilder& rowRendererBuilder,
    Ref<UserProfile> userProfile)
    : m_model{model.Get()},
      m_selectionModel{selectionModel.Get()},
      m_rowRendererBuilder{rowRendererBuilder},
      m_userProfile{userProfile.Get()},
      m_header{std::make_unique<DashboardRow>(
        std::make_unique<ValueDashboardCell>())},
      m_headerEntry{std::make_unique<RowEntry>()},
      m_defaultColumnWidth{scale_width(75)},
      m_minimumColumnWidth{scale_width(25)},
      m_maxRowHeight{scale_height(20)} {
  for(auto i = 0; i < model->GetColumnCount(); ++i) {
    ColumnEntry column;
    column.m_index = i;
    column.m_width = m_defaultColumnWidth;
    m_columns.push_back(column);
  }
  SetupHeader();
  for(auto i = 0; i < model->GetRowCount(); ++i) {
    OnRowAddedSignal(m_model->GetRow(i));
  }
  m_rowAddedConnection = m_model->ConnectRowAddedSignal(std::bind(
    &DashboardRenderer::OnRowAddedSignal, this, std::placeholders::_1));
  m_rowRemovedConnection = m_model->ConnectRowRemovedSignal(std::bind(
    &DashboardRenderer::OnRowRemovedSignal, this, std::placeholders::_1));
}

const DashboardModel& DashboardRenderer::GetModel() const {
  return *m_model;
}

int DashboardRenderer::GetSize() const {
  return m_rows.size();
}

const DashboardRowRenderer& DashboardRenderer::GetHeaderRenderer() const {
  return *m_headerEntry->m_renderer;
}

DashboardRowRenderer& DashboardRenderer::GetHeaderRenderer() {
  return *m_headerEntry->m_renderer;
}

int DashboardRenderer::GetMaxRowHeight() const {
  return m_maxRowHeight;
}

void DashboardRenderer::SetMaxRowHeight(int height) {
  m_maxRowHeight = height;
}

int DashboardRenderer::GetMinimunColumnWidth() const {
  return m_minimumColumnWidth;
}

void DashboardRenderer::SetMinimumColumnWidth(int width) {
  m_minimumColumnWidth = width;
}

int DashboardRenderer::GetDefaultColumnWidth() const {
  return m_defaultColumnWidth;
}

void DashboardRenderer::SetDefaultColumnWidth(int value) {
  m_defaultColumnWidth = value;
  m_drawSignal();
}

int DashboardRenderer::GetColumnWidth(int index) const {
  if(index >= static_cast<int>(m_columns.size())) {
    return 0;
  }
  return m_columns[index].m_width;
}

void DashboardRenderer::SetColumnWidth(int index, int width) {
  width = std::max(width, m_minimumColumnWidth);
  if(m_columns[index].m_width == width) {
    return;
  }
  m_columns[index].m_width = width;
  {
    shared_connection_block block{m_headerEntry->m_drawConnection};
    m_headerEntry->m_renderer->SetCellWidth(index, width);
  }
  for(auto& row : m_rows) {
    if(row->m_row == nullptr) {
      continue;
    }
    shared_connection_block block{row->m_drawConnection};
    row->m_renderer->SetCellWidth(index, width);
  }
  m_drawSignal();
}

int DashboardRenderer::GetColumnModelIndex(int index) const {
  return m_columns[index].m_index;
}

boost::optional<const DashboardRow&> DashboardRenderer::GetRow(
    int index) const {
  if(index < 0 || index >= static_cast<int>(m_rows.size()) ||
      m_rows[index]->m_row == nullptr) {
    return none;
  }
  return *m_rows[index]->m_row;
}

int DashboardRenderer::GetRowDisplayIndex(const DashboardRow& row) const {
  for(auto i = 0; i < static_cast<int>(m_rows.size()); ++i) {
    if(m_rows[i]->m_row == &row) {
      return i;
    }
  }
  return -1;
}

void DashboardRenderer::MoveColumn(int sourceIndex, int destinationIndex) {
  if(sourceIndex == destinationIndex) {
    return;
  }
  auto column = m_columns[sourceIndex];
  m_columns.erase(m_columns.begin() + sourceIndex);
  m_columns.insert(m_columns.begin() + destinationIndex, column);
  {
    shared_connection_block block{m_headerEntry->m_drawConnection};
    m_headerEntry->m_renderer->Move(sourceIndex, destinationIndex);
    for(auto& row : m_rows) {
      row->m_renderer->Move(sourceIndex, destinationIndex);
    }
  }
  m_drawSignal();
}

void DashboardRenderer::MoveRow(int sourceIndex, int destinationIndex) {
  if(sourceIndex == destinationIndex) {
    return;
  }
  auto row = std::move(m_rows[sourceIndex]);
  auto isDestinationEmpty = m_rows[destinationIndex]->m_row == nullptr;
  m_rows.erase(m_rows.begin() + sourceIndex);
  auto revisedDestination =
    [&] {
      if(destinationIndex < sourceIndex) {
        return destinationIndex;
      } else {
        return destinationIndex - 1;
      }
    }();
  m_rows.insert(m_rows.begin() + revisedDestination, std::move(row));
  if(isDestinationEmpty) {
    m_rows.erase(m_rows.begin() + revisedDestination + 1);
  }
  if(m_rows.back()->m_row == nullptr) {
    m_rows.pop_back();
  }
  m_drawSignal();
}

void DashboardRenderer::ReorderRows(const vector<int>& indicies) {
  auto rows = std::move(m_rows);
  for(auto i = 0; i < static_cast<int>(indicies.size()); ++i) {
    m_rows.push_back(std::move(rows[indicies[i]]));
  }
  while(!m_rows.empty() && m_rows.back()->m_row == nullptr) {
    m_rows.pop_back();
  }
  m_drawSignal();
}

void DashboardRenderer::InsertEmptyRow(int index) {
  auto rowEntry = std::make_unique<RowEntry>();
  rowEntry->m_row = nullptr;
  m_rows.insert(m_rows.begin() + index, std::move(rowEntry));
  m_drawSignal();
}

void DashboardRenderer::Draw(QPaintDevice& device, const QRect& region) {
  auto height = std::min(GetMaxRowHeight(),
    static_cast<int>(region.height() / (m_rows.size() + 1)));
  QPainter painter;
  auto topPoint = region.top();
  QRect headerRegion{region.left(), topPoint, region.width(), height};
  painter.begin(&device);
  painter.fillRect(headerRegion, QColor{19, 91, 164});
  {
    QPen pen;
    pen.setColor(QColor{35, 35, 35});
    painter.setPen(pen);
    painter.drawRect(headerRegion);
    int verticalBorder = 0;
    for(auto& column : m_columns) {
      painter.drawLine(QPoint{verticalBorder, 0},
        QPoint{verticalBorder, height});
      verticalBorder += column.m_width;
    }
    painter.drawLine(QPoint{verticalBorder, 0},
      QPoint{verticalBorder, height});
  }
  painter.end();
  m_headerEntry->m_renderer->Draw(device, headerRegion);
  topPoint += height;
  auto rowCount = 0;
  auto activeRow = m_selectionModel->GetActiveRow().get_value_or(-1);
  for(auto& row : m_rows) {
    QRect rowRegion{region.left(), topPoint, region.width(), height};
    DrawBackground(painter, device, rowRegion, rowCount);
    if(row->m_row != nullptr) {
      row->m_renderer->Draw(device, rowRegion);
    }
    DrawForeground(painter, device, rowRegion, rowCount);
    topPoint += height;
    ++rowCount;
    if(topPoint > region.bottom()) {
      break;
    }
  }
  if(topPoint > region.bottom()) {
    return;
  }
  painter.begin(&device);
  while(topPoint <= region.bottom()) {
    QRect rowRegion{region.left(), topPoint, region.width(), height};
    DrawBackground(painter, device, rowRegion, rowCount);
    DrawForeground(painter, device, rowRegion, rowCount);
    topPoint += height;
    ++rowCount;
  }
  painter.end();
}

DashboardRendererSettings DashboardRenderer::GetSettings() const {
  DashboardRendererSettings settings;
  settings.m_columns = m_columns;
  for(auto i = 0; i < static_cast<int>(m_rows.size()); ++i) {
    if(m_rows[i]->m_row == nullptr) {
      settings.m_emptyRows.push_back(i);
    }
  }
  settings.m_defaultColumnWidth = m_defaultColumnWidth;
  settings.m_minimumColumnWidth = m_minimumColumnWidth;
  settings.m_maxRowHeight = m_maxRowHeight;
  return settings;
}

void DashboardRenderer::Apply(const DashboardRendererSettings& settings) {
  SetDefaultColumnWidth(settings.m_defaultColumnWidth);
  SetMinimumColumnWidth(settings.m_minimumColumnWidth);
  SetMaxRowHeight(settings.m_maxRowHeight);
  for(auto i = 0; i < static_cast<int>(settings.m_columns.size()); ++i) {
    for(auto j = 0; j < static_cast<int>(m_columns.size()); ++j) {
      if(settings.m_columns[i].m_index == m_columns[j].m_index) {
        MoveColumn(j, i);
        SetColumnWidth(i, settings.m_columns[i].m_width);
        break;
      }
    }
  }
  for(auto& row : settings.m_emptyRows) {
    InsertEmptyRow(row);
  }
  m_drawSignal();
}

connection DashboardRenderer::ConnectDrawSignal(
    const DrawSignal::slot_function_type& slot) const {
  return m_drawSignal.connect(slot);
}

void DashboardRenderer::SetupHeader() {
  auto& indexCell = static_cast<ValueDashboardCell&>(
    const_cast<DashboardCell&>(m_header->GetCell(0)));
  indexCell.SetValue(m_model->GetColumnName(0));
  for(auto i = 1; i < m_model->GetColumnCount(); ++i) {
    auto headerCell = std::make_unique<ValueDashboardCell>();
    headerCell->SetBufferSize(1);
    headerCell->SetValue(m_model->GetColumnName(i));
    m_header->Add(std::move(headerCell));
  }
  m_headerEntry->m_row = m_header.get();
  auto headerRendererBuilder = GetDefaultHeaderRendererBuilder(
    Ref(*m_userProfile));
  m_headerEntry->m_renderer = headerRendererBuilder(*m_headerEntry->m_row);
  m_headerEntry->m_drawConnection =
    m_headerEntry->m_renderer->ConnectDrawSignal(
    std::bind(&DashboardRenderer::OnRowDrawSignal, this));
}

void DashboardRenderer::DrawBackground(QPainter& painter, QPaintDevice& device,
    const QRect& region, int index) {
  if(m_selectionModel->IsRowSelected(index)) {
    painter.begin(&device);
    painter.fillRect(region, QColor{67, 79, 90});
    painter.end();
  } else {
    QColor backgroundColor{129, 147, 163};
    painter.begin(&device);
    painter.fillRect(region, backgroundColor);
    painter.end();
  }
}

void DashboardRenderer::DrawForeground(QPainter& painter, QPaintDevice& device,
    const QRect& region, int index) {
  if(index == m_selectionModel->GetActiveRow().get_value_or(-1)) {
    painter.begin(&device);
    QPen pen;
    pen.setColor(QColor{255, 255, 255});
    painter.setPen(pen);
    auto rectange = region;
    rectange.setWidth(rectange.width() - pen.width());
    rectange.setHeight(rectange.height() - pen.width());
    painter.drawRect(rectange);
    painter.end();
  }
}

void DashboardRenderer::OnRowAddedSignal(const DashboardRow& row) {
  auto rowEntry = std::make_unique<RowEntry>();
  rowEntry->m_row = &row;
  rowEntry->m_renderer = m_rowRendererBuilder(*rowEntry->m_row);
  vector<int> indexes;
  for(auto i = 0; i < static_cast<int>(m_columns.size()); ++i) {
    indexes.push_back(i);
  }
  for(auto i = 0; i < static_cast<int>(indexes.size()); ++i) {
    auto index = m_columns[i].m_index;
    for(auto j = 0; j < static_cast<int>(indexes.size()); ++j) {
      if(indexes[j] == index) {
        rowEntry->m_renderer->Move(j, i);
        indexes.erase(indexes.begin() + j);
        indexes.insert(indexes.begin() + i, index);
      }
    }
  }
  for(auto i = 0; i < rowEntry->m_row->GetSize(); ++i) {
    rowEntry->m_renderer->SetCellWidth(i, m_columns[i].m_width);
  }
  rowEntry->m_drawConnection = rowEntry->m_renderer->ConnectDrawSignal(
    std::bind(&DashboardRenderer::OnRowDrawSignal, this));
  m_rows.push_back(std::move(rowEntry));
  m_drawSignal();
}

void DashboardRenderer::OnRowRemovedSignal(const DashboardRow& row) {
  for(auto& entry : m_rows) {
    if(entry->m_row != &row) {
      continue;
    }
    entry->m_drawConnection.disconnect();
    entry->m_row = nullptr;
    entry->m_renderer.reset();
    m_drawSignal();
    break;
  }
}

void DashboardRenderer::OnRowDrawSignal() {
  m_drawSignal();
}
