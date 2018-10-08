#include "Spire/Dashboard/DashboardSelectionController.hpp"
#include <limits>
#include <QKeyEvent>
#include <QMouseEvent>
#include "Spire/Dashboard/DashboardSelectionModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

DashboardSelectionController::DashboardSelectionController(
    Ref<DashboardSelectionModel> model)
    : m_model{model.Get()},
      m_mouseState{MouseState::NONE},
      m_selectState{SelectState::NONE},
      m_multiSelectRange{1, 0} {}

bool DashboardSelectionController::HandleKeyEvent(const QKeyEvent& event) {
  if(event.type() == QEvent::Type::KeyPress) {
    if(event.key() == Qt::Key_Control) {
      m_selectState = SelectState::SINGLE;
      return true;
    } else if(event.key() == Qt::Key_Shift) {
      m_selectState = SelectState::MULTIPLE;
      return true;
    } else if(event.key() == Qt::Key_Down) {
      auto currentActiveRow = m_model->GetActiveRow();
      if(!currentActiveRow.is_initialized()) {
        m_model->Reset();
        m_model->AddRow(0);
        SetActiveRow(0);
      } else {
        auto nextActiveRow = *currentActiveRow + 1;
        if(event.modifiers() & Qt::ShiftModifier ||
            event.modifiers() & Qt::ControlModifier) {
          if(m_model->IsRowSelected(nextActiveRow)) {
            m_model->RemoveRow(*currentActiveRow);
          } else {
            m_model->AddRow(*currentActiveRow);
          }
        } else {
          m_model->Reset();
        }
        m_model->AddRow(nextActiveRow);
        SetActiveRow(nextActiveRow);
      }
      return true;
    } else if(event.key() == Qt::Key_Up) {
      auto currentActiveRow = m_model->GetActiveRow();
      if(!currentActiveRow.is_initialized()) {
        m_model->Reset();
        SetActiveRow(0);
      } else if(*currentActiveRow != 0) {
        auto nextActiveRow = *currentActiveRow - 1;
        if(event.modifiers() & Qt::ShiftModifier ||
            event.modifiers() & Qt::ControlModifier) {
          if(m_model->IsRowSelected(nextActiveRow)) {
            m_model->RemoveRow(*currentActiveRow);
          } else {
            m_model->AddRow(*currentActiveRow);
          }
        } else {
          m_model->Reset();
        }
        m_model->AddRow(nextActiveRow);
        SetActiveRow(nextActiveRow);
      }
      return true;
    }
    return false;
  } else if(event.type() == QEvent::Type::KeyRelease) {
    m_selectState = SelectState::NONE;
    return true;
  }
  return false;
}

bool DashboardSelectionController::HandleMouseEvent(const QMouseEvent& event,
    int rowIndex) {
  if(event.type() == QEvent::MouseButtonPress) {
    m_mouseState = MouseState::PRESSED;
    if(m_selectState == SelectState::SINGLE) {
      auto isSelected = m_model->IsRowSelected(rowIndex);
      if(rowIndex == m_model->GetActiveRow()) {
        if(isSelected) {
          SetActiveRow(none);
        }
      } else {
        SetActiveRow(rowIndex);
      }
      if(isSelected) {
        m_model->RemoveRow(rowIndex);
      } else {
        m_model->AddRow(rowIndex);
      }
    } else if(m_selectState == SelectState::MULTIPLE) {
      auto activeRow = m_model->GetActiveRow();
      if(!activeRow.is_initialized()) {
        m_model->AddRow(rowIndex);
        SetActiveRow(rowIndex);
        return true;
      }
      for(auto row = get<0>(m_multiSelectRange);
          row <= get<1>(m_multiSelectRange); ++row) {
        m_model->RemoveRow(row);
      }
      for(auto row : m_multiSelectedRows) {
        m_model->AddRow(row);
      }
      auto startRow = std::min(*activeRow, rowIndex);
      auto endRow = std::max(*activeRow, rowIndex);
      m_multiSelectedRows = m_model->GetSelectedRows();
      m_multiSelectRange = make_tuple(startRow, endRow);
      for(auto row = startRow; row <= endRow; ++row) {
        if(row != *activeRow) {
          m_model->AddRow(row);
        }
      }
      return true;
    } else {
      if(rowIndex == m_model->GetActiveRow()) {
        return true;
      }
      m_model->Reset();
      m_model->AddRow(rowIndex);
      SetActiveRow(rowIndex);
    }
    return true;
  } else if(event.type() == QEvent::MouseButtonRelease) {
    m_mouseState = MouseState::NONE;
    return true;
  } else if(event.type() == QEvent::MouseButtonDblClick) {
    m_mouseState = MouseState::NONE;
    if(m_selectState != SelectState::NONE) {
      return true;
    }
  }
  return false;
}

void DashboardSelectionController::SetActiveRow(
    const boost::optional<int>& row) {
  m_multiSelectRange = make_tuple(1, 0);
  m_multiSelectedRows.clear();
  m_model->SetActiveRow(row);
}
