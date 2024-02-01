#include "Spire/CanvasView/CanvasTableWidget.hpp"
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/CanvasView/LabelCell.hpp"
#include "Spire/CanvasView/LinkCell.hpp"
#include "Spire/CanvasView/MoveNodeCommand.hpp"
#include "Spire/CanvasView/PopulateCanvasNodeContextMenu.hpp"
#include "Spire/CanvasView/StandardCanvasTableItemDelegate.hpp"
#include "Spire/CanvasView/ValueCell.hpp"
#include "Spire/LegacyUI/CommandAction.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  int nextModelId = 1;

  const int COLUMN_SPACING = 20;
  const int ROW_SPACING = 10;
  const int PASSED = 0;
  const int FIXED_COLLISION = 1;
  const int SELF_COLLISION = 2;

  CanvasNodeModel::Coordinate Align(CanvasNodeModel::Coordinate coordinate) {
    while((coordinate.m_row - 1) % 3 != 0) {
      ++coordinate.m_row;
    }
    if(coordinate.m_column % 2 != 0) {
      ++coordinate.m_column;
    }
    return coordinate;
  }
}

CanvasTableWidget::NodeEntry::NodeEntry(const CanvasNode& node,
    const Coordinate& coordinate)
    : m_node(CanvasNode::Clone(node)),
      m_coordinate(coordinate) {}

CanvasTableWidget::NodeEntry::NodeEntry(const NodeEntry& entry)
    : m_node(CanvasNode::Clone(*entry.m_node)),
      m_coordinate(entry.m_coordinate) {}

CanvasTableWidget::NodeEntry& CanvasTableWidget::NodeEntry::operator =(
    const NodeEntry& entry) {
  m_node = CanvasNode::Clone(*entry.m_node);
  m_coordinate = entry.m_coordinate;
  return *this;
}

CanvasTableWidget::NodeCells::NodeCells()
    : m_valueCell(nullptr),
      m_labelCell(nullptr) {}

CanvasTableWidget::CanvasTableWidget(QWidget* parent)
    : QTableWidget(parent),
      m_userProfile(nullptr),
      m_modelId(nextModelId),
      m_nextCanvasNodeId(1),
      m_handlingKeyEvent(false),
      m_editedCell(nullptr),
      m_cellEditor(nullptr) {
  ++nextModelId;
  setRowCount(INITIAL_ROW_COUNT);
  setColumnCount(INITIAL_COLUMN_COUNT);
  setItemDelegate(new StandardCanvasTableItemDelegate(Ref(*this)));
  setEditTriggers(NoEditTriggers);
  for(int i = 1; i < INITIAL_COLUMN_COUNT; i += 2) {
    setColumnWidth(i, COLUMN_SPACING);
  }
  for(int i = 2; i < INITIAL_ROW_COUNT; i += 3) {
    setRowHeight(i, ROW_SPACING);
  }
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &CanvasTableWidget::customContextMenuRequested, this,
    &CanvasTableWidget::OnContextMenu);
}

void CanvasTableWidget::SetUserProfile(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.Get();
}

void CanvasTableWidget::NavigateForward() {
  auto currentNodeCheck = GetCurrentNode();
  if(!currentNodeCheck.is_initialized()) {
    return;
  }
  auto& currentNode = *currentNodeCheck;
  auto& i = GetCurrentNodeIterator();
  do {
    ++i;
  } while(i != BreadthFirstCanvasNodeIterator() && !i->IsVisible());
  if(i == BreadthFirstCanvasNodeIterator()) {
    return;
  }
  auto& nextNode = *i;
  if(&nextNode == &currentNode) {
    return;
  }
  SetNodeSelected(currentNode, false);
  SetCurrent(nextNode);
}

void CanvasTableWidget::NavigateBackward() {
  auto currentNodeCheck = GetCurrentNode();
  if(!currentNodeCheck.is_initialized()) {
    return;
  }
  auto& currentNode = *currentNodeCheck;
  auto& i = GetCurrentNodeIterator();
  do {
    --i;
  } while(i != BreadthFirstCanvasNodeIterator() && !i->IsVisible());
  if(i == BreadthFirstCanvasNodeIterator()) {
    return;
  }
  auto& nextNode = *i;
  if(&nextNode == &currentNode) {
    return;
  }
  SetNodeSelected(currentNode, false);
  SetCurrent(nextNode);
}

vector<const CanvasNode*> CanvasTableWidget::GetSelectedNodes() const {
  auto items = const_cast<CanvasTableWidget*>(this)->selectedItems();
  vector<const CanvasNode*> selectedNodes;
  for(auto i = items.begin(); i != items.end(); ++i) {
    if(dynamic_cast<ValueCell*>(*i) == nullptr) {
      continue;
    }
    auto& node = static_cast<ValueCell*>(*i)->GetNode();
    selectedNodes.push_back(&node);
  }
  return selectedNodes;
}

void CanvasTableWidget::Edit(const CanvasNode& node, QEvent* event) {
  assert(m_nodeCells.find(&node) != m_nodeCells.end());
  if(!m_beginEditSignal(node)) {
    return;
  }
  auto cell = m_nodeCells[&node].m_valueCell;
  m_editor.reset(new CanvasNodeEditor());
  auto editAction = m_editor->GetEditor(Ref(node), Ref(*this),
    Ref(*m_userProfile), event);
  auto visitor = MakeVariantLambdaVisitor<void>(
    [&] (QWidget* cellEditor) {
      m_cellEditor = cellEditor;
      m_cellEditor->installEventFilter(this);
      m_editedCell = cell;
      setCellWidget(m_editedCell->row(), m_editedCell->column(), m_cellEditor);
      m_cellEditor->setFocus();
    },
    [&] (QUndoCommand* command) {
      m_commandSignal(command);
    });
  apply_visitor(visitor, editAction);
}

connection CanvasTableWidget::ConnectBeginEditSignal(
    const BeginEditSignal::slot_type& slot) const {
  return m_beginEditSignal.connect(slot);
}

connection CanvasTableWidget::ConnectCommandSignal(
    const CommandSignal::slot_type& slot) const {
  return m_commandSignal.connect(slot);
}

vector<const CanvasNode*> CanvasTableWidget::GetRoots() const {
  vector<const CanvasNode*> roots;
  transform(m_roots.begin(), m_roots.end(), back_inserter(roots),
    [] (const unique_ptr<CanvasNode>& node) {
      return node.get();
    });
  return roots;
}

boost::optional<const CanvasNode&> CanvasTableWidget::GetNode(
    const Coordinate& coordinate) const {
  auto alignedCoordinate = Align(coordinate);
  auto cell = static_cast<CanvasCell*>(item(alignedCoordinate.m_row,
    alignedCoordinate.m_column));
  if(cell == nullptr) {
    return none;
  }
  return cell->GetNode();
}

CanvasNodeModel::Coordinate CanvasTableWidget::GetCoordinate(
    const CanvasNode& node) const {
  auto cellsIterator = m_nodeCells.find(&node);
  assert(cellsIterator != m_nodeCells.end());
  auto index = indexFromItem(cellsIterator->second.m_valueCell);
  return Coordinate(index.row(), index.column());
}

boost::optional<const CanvasNode&> CanvasTableWidget::GetCurrentNode() const {
  auto currentCell = static_cast<CanvasCell*>(currentItem());
  if(currentCell == nullptr) {
    return none;
  }
  return currentCell->GetNode();
}

boost::optional<CanvasNodeModel::Coordinate> CanvasTableWidget::
    GetCurrentCoordinate() const {
  auto currentCell = static_cast<CanvasCell*>(currentItem());
  if(currentCell == nullptr) {
    return none;
  }
  return Coordinate(currentCell->row(), currentCell->column());
}

void CanvasTableWidget::SetCurrent(const CanvasNode& node) {
  if(!node.IsVisible()) {
    return;
  }
  auto coordinate = GetCoordinate(node);
  setCurrentCell(coordinate.m_row, coordinate.m_column);
}

const CanvasNode& CanvasTableWidget::Add(const Coordinate& coordinate,
    const CanvasNode& node) {
  CanvasNodeBuilder builder(node);
  BreadthFirstCanvasNodeIterator metaDataIterator(node);
  std::unordered_map<int, int> remappedIdentities;
  while(metaDataIterator != BreadthFirstCanvasNodeIterator()) {
    auto& metaDataNode = *metaDataIterator;
    Identity identity(m_modelId, m_nextCanvasNodeId);
    auto previousIdentity = FindIdentity(metaDataNode);
    if(previousIdentity.is_initialized() &&
        previousIdentity->m_modelId == m_modelId) {
      remappedIdentities.insert(
        make_pair(previousIdentity->m_canvasNodeId, identity.m_canvasNodeId));
    }
    ++m_nextCanvasNodeId;
    builder.SetMetaData(metaDataNode, GetIdentityKey(), ToMetaData(identity));
    ++metaDataIterator;
  }
  auto managedNode = builder.Make();
  auto unmanagedNode = managedNode.release();
  InternalAdd(coordinate, *unmanagedNode);
  auto errors = Validate(*unmanagedNode);
  for(const auto& error : errors) {
    auto cellIterator = m_nodeCells.find(&error.GetNode());
    if(cellIterator == m_nodeCells.end()) {
      continue;
    }
    auto& cells = cellIterator->second;
    cells.m_valueCell->AddError(error);
    auto previousNode = &error.GetNode();
    auto traveller = error.GetNode().GetParent();
    while(traveller.is_initialized()) {
      cellIterator = m_nodeCells.find(&*traveller);
      if(cellIterator == m_nodeCells.end()) {
        break;
      }
      auto& cells = cellIterator->second;
      if(cells.m_valueCell->GetErrors().empty()) {
        cells.m_valueCell->AddError(CanvasNodeValidationError(Ref(*traveller),
          "Child '" + previousNode->GetName() + "' contains an error."));
      }
      previousNode = &*traveller;
      traveller = traveller->GetParent();
    }
  }
  return *unmanagedNode;
}

void CanvasTableWidget::Remove(const CanvasNode& node) {
  InternalRemove(GetRoot(node));
}

bool CanvasTableWidget::event(QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent*>(event);
    if(keyEvent->key() == Qt::Key_Tab) {
      NavigateForward();
      return true;
    } else if(keyEvent->key() == Qt::Key_Backtab) {
      NavigateBackward();
      return true;
    }
  }
  return QTableWidget::event(event);
}

bool CanvasTableWidget::eventFilter(QObject* object, QEvent* event) {
 if(event->type() == QEvent::FocusOut && object == m_cellEditor) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->reason() != Qt::PopupFocusReason) {
      CommitEditor();
    }
    if(focusEvent->reason() == Qt::TabFocusReason) {
      NavigateForward();
    } else if(focusEvent->reason() == Qt::BacktabFocusReason) {
      NavigateBackward();
    }
  }
  return QWidget::eventFilter(object, event);
}

void CanvasTableWidget::keyPressEvent(QKeyEvent* event) {
  auto currentCell = dynamic_cast<ValueCell*>(currentItem());
  auto key = event->key();
  if(m_editedCell == nullptr) {
    auto coordinate = Align(Coordinate(currentRow(), currentColumn()));
    if(key == Qt::Key_Down) {
      setCurrentCell(coordinate.m_row + 2, coordinate.m_column);
      return QTableWidget::keyPressEvent(event);
    } else if(key == Qt::Key_Up) {
      setCurrentCell(coordinate.m_row - 2, coordinate.m_column);
      return QTableWidget::keyPressEvent(event);
    } else if(key == Qt::Key_Left) {
      setCurrentCell(coordinate.m_row, coordinate.m_column - 1);
      return QTableWidget::keyPressEvent(event);
    } else if(key == Qt::Key_Right) {
      setCurrentCell(coordinate.m_row, coordinate.m_column + 1);
      return QTableWidget::keyPressEvent(event);
    }
  }
  if(m_handlingKeyEvent || currentCell == nullptr) {
    return QTableWidget::keyPressEvent(event);
  }
  m_handlingKeyEvent = true;
  if(m_editedCell != nullptr) {
    if(key == Qt::Key_Enter || key == Qt::Key_Return) {
      CommitEditor();
      setFocus();
    } else if(key == Qt::Key_Escape) {
      setCellWidget(m_editedCell->row(), m_editedCell->column(), nullptr);
      m_cellEditor = nullptr;
      m_editedCell = nullptr;
      setFocus();
      m_commandSignal(nullptr);
    } else {
      QTableWidget::keyPressEvent(event);
    }
    m_handlingKeyEvent = false;
    return;
  }
  auto& currentNode = currentCell->GetNode();
  if(key == Qt::Key_Home) {
    assert(m_nodeCells.find(&GetRoot(currentNode)) != m_nodeCells.end());
    SetCurrent(GetRoot(currentNode));
  } else if(key != Qt::Key_Escape && !event->text().isEmpty()) {
    QEvent* editEvent = nullptr;
    if(key != Qt::Key_Enter && key != Qt::Key_Return) {
      editEvent = event;
    }
    Edit(currentNode, editEvent);
  } else {
    QTableWidget::keyPressEvent(event);
  }
  m_handlingKeyEvent = false;
}

void CanvasTableWidget::startDrag(Qt::DropActions supportedActions) {
  if(GetSelectedNodes().size() != 1) {
    return;
  }
  return QTableWidget::startDrag(supportedActions);
}

void CanvasTableWidget::dropEvent(QDropEvent* event) {
  auto selectedNodes = GetSelectedNodes();
  if(selectedNodes.size() != 1) {
    event->accept();
    return;
  }
  auto destinationIndex = indexAt(event->pos());
  auto destination = Align(Coordinate(destinationIndex.row(),
    destinationIndex.column()));
  auto source = GetCoordinate(*selectedNodes.front());
  auto command = new MoveNodeCommand(Ref(*this), source, destination);
  m_commandSignal(command);
  event->accept();
}

BreadthFirstCanvasNodeIterator& CanvasTableWidget::GetCurrentNodeIterator() {
  auto current = GetCurrentNode();
  assert(current);
  if(m_currentNodeIterator == BreadthFirstCanvasNodeIterator() ||
      &*m_currentNodeIterator != &*current) {
    m_currentNodeIterator = BreadthFirstCanvasNodeIterator(GetRoot(*current),
      *current);
  }
  return m_currentNodeIterator;
}

void CanvasTableWidget::SetNodeSelected(const CanvasNode& node, bool selected) {
  if(!node.IsVisible()) {
    return;
  }
  auto cellIterator = m_nodeCells.find(&node);
  if(cellIterator == m_nodeCells.end()) {
    return;
  }
  cellIterator->second.m_valueCell->setSelected(selected);
}

void CanvasTableWidget::InternalAdd(Coordinate coordinate,
    const CanvasNode& node) {
  coordinate = Align(coordinate);
  vector<const CanvasNode*> fixedNodes;
  fixedNodes.push_back(&node);
  vector<NodeEntry> displacedNodes;
  InternalInsert(&coordinate, node, &fixedNodes, &displacedNodes, "");
  while(!displacedNodes.empty()) {
    auto displacedNode = displacedNodes.front();
    displacedNodes.erase(displacedNodes.begin());
    auto nextNode = displacedNode.m_node.release();
    fixedNodes.push_back(nextNode);
    vector<NodeEntry> newDisplacedNodes;
    while(InternalInsert(&displacedNode.m_coordinate, *nextNode, &fixedNodes,
        &newDisplacedNodes, "") != PASSED) {
      displacedNode.m_coordinate.m_column += 2;
    }
    displacedNodes.insert(displacedNodes.end(), newDisplacedNodes.begin(),
      newDisplacedNodes.end());
  }
  if(selectionModel()->isSelected(model()->index(coordinate.m_row,
      coordinate.m_column))) {
    SetNodeSelected(node, true);
  }
}

int CanvasTableWidget::InternalInsert(Coordinate* coordinate,
    const CanvasNode& node, vector<const CanvasNode*>* fixedNodes,
    vector<NodeEntry>* displacedNodes, const string& name) {
  if(coordinate->m_row >= rowCount() - 5 ) {
    setRowCount(2 * rowCount());
  }
  if(coordinate->m_column >= columnCount() - 5) {
    setColumnCount(2 * columnCount());
  }
  assert(m_nodeCells.find(&node) == m_nodeCells.end());
  if(!node.IsVisible()) {
    return PASSED;
  }
  auto children = node.GetChildren();
  vector<const CanvasNode*> visibleChildren;
  auto childCoordinate = *coordinate;
  childCoordinate.m_column += 2;
  for(auto i = children.begin(); i != children.end(); ++i) {
    auto& child = *i;
    auto insertChildResult = SELF_COLLISION;
    while(insertChildResult == SELF_COLLISION) {
      insertChildResult = InternalInsert(&childCoordinate, child, fixedNodes,
        displacedNodes, child.GetName());
      if(insertChildResult == SELF_COLLISION) {
        childCoordinate.m_row += 3;
      } else if(insertChildResult != PASSED) {
        for(auto j = children.begin(); j != i; ++j) {
          InternalRemove(*j);
        }
        return insertChildResult;
      }
    }
    if(child.IsVisible()) {
      if(visibleChildren.empty()) {
        coordinate->m_row = childCoordinate.m_row;
      }
      visibleChildren.push_back(&child);
      childCoordinate.m_row += 3;
    }
  }
  auto occupyingCell = static_cast<CanvasCell*>(item(coordinate->m_row,
    coordinate->m_column));
  if(occupyingCell != nullptr) {
    auto& occupyingNode = occupyingCell->GetNode();
    if(&GetRoot(occupyingNode) == &GetRoot(node)) {
      for(auto i = children.begin(); i != children.end(); ++i) {
        InternalRemove(*i);
      }
      return SELF_COLLISION;
    } else if(std::find(fixedNodes->begin(), fixedNodes->end(),
        &GetRoot(occupyingNode)) != fixedNodes->end()) {
      for(auto i = children.begin(); i != children.end(); ++i) {
        InternalRemove(*i);
      }
      return FIXED_COLLISION;
    } else {
      auto& occupyingRoot = GetRoot(occupyingNode);
      assert(m_nodeCells.find(&occupyingRoot) != m_nodeCells.end());
      auto& cells = m_nodeCells[&occupyingRoot];
      auto cellRow = cells.m_valueCell->row();
      auto cellColumn = cells.m_valueCell->column();
      NodeEntry entry(occupyingRoot, Coordinate(cellRow, cellColumn));
      InternalRemove(occupyingRoot);
      displacedNodes->push_back(entry);
    }
  }
  auto& cells = m_nodeCells[&node];
  if(IsRoot(node)) {
    cells.m_labelCell = nullptr;
  } else {
    auto labelCell = new LabelCell(name, Ref(node));
    cells.m_labelCell = labelCell;
    setItem(coordinate->m_row - 1, coordinate->m_column, labelCell);
  }
  cells.m_valueCell = new ValueCell(Ref(node));
  setItem(coordinate->m_row, coordinate->m_column, cells.m_valueCell);
  if(IsRoot(node)) {
    m_roots.emplace_back(const_cast<CanvasNode*>(&node));
  }
  if(!visibleChildren.empty()) {
    auto link = new LinkCell(Qt::Horizontal, Ref(node),
      Ref(*visibleChildren.front()));
    cells.m_linkCells.push_back(link);
    setItem(coordinate->m_row, coordinate->m_column + 1, link);
    if(visibleChildren.size() > 1) {
      assert(m_nodeCells.find(visibleChildren.back()) != m_nodeCells.end());
      auto& childCells = m_nodeCells[visibleChildren.back()];
      auto lastRow = childCells.m_labelCell->row();
      assert(lastRow >= coordinate->m_row);
      auto child = 0;
      for(auto i = coordinate->m_row; i != lastRow; ++i) {
        auto cell = static_cast<CanvasCell*>(item(i, coordinate->m_column + 2));
        if(cell == nullptr) {
          auto link = new LinkCell(Qt::Vertical, Ref(node),
            Ref(*visibleChildren[child]));
          cells.m_linkCells.push_back(link);
          setItem(i, coordinate->m_column + 2, link);
        } else if(dynamic_cast<ValueCell*>(cell) != nullptr) {
          if(!IsSame(cell->GetNode().GetParent(), node)) {
            for(auto j = children.begin(); j != children.end(); ++j) {
              InternalRemove(*j);
            }
            InternalRemove(node);
            if(&GetRoot(cell->GetNode()) == &GetRoot(node)) {
              return SELF_COLLISION;
            } else {
              return FIXED_COLLISION;
            }
          }
          ++child;
        }
      }
    }
  }
  return PASSED;
}

void CanvasTableWidget::InternalRemove(const CanvasNode& node) {
  if(m_currentNodeIterator != BreadthFirstCanvasNodeIterator() &&
      &*m_currentNodeIterator == &node) {
    m_currentNodeIterator = BreadthFirstCanvasNodeIterator();
  }
  auto cellsIterator = m_nodeCells.find(&node);
  unique_ptr<CanvasNode> rootNode;
  if(cellsIterator != m_nodeCells.end()) {
    SetNodeSelected(node, false);
    auto& cells = cellsIterator->second;
    takeItem(cells.m_valueCell->row(), cells.m_valueCell->column());
    delete cells.m_valueCell;
    if(cells.m_labelCell != nullptr) {
      takeItem(cells.m_labelCell->row(), cells.m_labelCell->column());
      delete cells.m_labelCell;
    }
    for(const auto& cell : cells.m_linkCells) {
      takeItem(cell->row(), cell->column());
      delete cell;
    }
    m_nodeCells.erase(cellsIterator);
    if(IsRoot(node)) {
      auto rootIterator = std::find_if(m_roots.begin(), m_roots.end(),
        [&] (unique_ptr<CanvasNode>& i) { return i.get() == &node; });
      if(rootIterator != m_roots.end()) {
        rootNode.swap(*rootIterator);
        m_roots.erase(rootIterator);
      }
    }
  }
  for(const auto& child : node.GetChildren()) {
    InternalRemove(child);
  }
}

void CanvasTableWidget::CommitEditor() {
  assert(m_editedCell != nullptr && m_cellEditor != nullptr);
  auto& editedNode = m_editedCell->GetNode();
  auto editCommand = m_editor->Commit();
  setCellWidget(m_editedCell->row(), m_editedCell->column(), nullptr);
  m_editedCell = nullptr;
  m_cellEditor = nullptr;
  m_commandSignal(editCommand);
}

void CanvasTableWidget::OnContextMenu(const QPoint& position) {
  auto item = dynamic_cast<ValueCell*>(itemAt(position));
  if(item == nullptr) {
    return;
  }
  QMenu contextMenu;
  PopulateCanvasNodeContextMenu(Ref(*this), item->GetNode(),
    Ref(*m_userProfile), Store(contextMenu));
  auto selectedAction = dynamic_cast<CommandAction*>(
    contextMenu.exec(static_cast<QWidget*>(sender())->mapToGlobal(position)));
  if(selectedAction != nullptr) {
    auto command = selectedAction->GetCommand().release();
    m_commandSignal(command);
  }
}
