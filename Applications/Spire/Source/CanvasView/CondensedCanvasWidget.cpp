#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include <Beam/Utilities/AssertionException.hpp>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"
#include "Spire/CanvasView/CondensedCanvasCell.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  bool IsDisplayed(const CanvasNode& node) {
    return dynamic_cast<const BaseValueNode*>(&node) != nullptr &&
      node.IsVisible() && !node.IsReadOnly();
  }

  vector<const CanvasNode*> GetLeaves(const CanvasNode& root) {
    vector<const CanvasNode*> leaves;
    for(const auto& node : BreadthFirstView(root)) {
      if(node.GetChildren().empty() && IsDisplayed(node)) {
        leaves.push_back(&node);
      }
    }
    return leaves;
  }

  bool CheckForLabel(const vector<const CanvasNode*>& leaves) {
    boost::optional<const CanvasNode&> parent;
    for(const auto& leaf : leaves) {
      if(!IsSame(leaf->GetParent(), parent)) {
        if(parent.is_initialized()) {
          return true;
        }
        parent = leaf->GetParent();
      }
    }
    return false;
  }
}

CondensedCanvasWidget::CondensedCanvasWidget(const string& name,
    Ref<UserProfile> userProfile, QWidget* parent)
    : QWidget{parent},
      m_name{name},
      m_userProfile{userProfile.Get()},
      m_node{nullptr},
      m_topLeaf{nullptr},
      m_currentNode{nullptr} {
  m_group = new QGroupBox{this};
  auto layout = new QVBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  layout->addWidget(m_group);
  m_layout = new QGridLayout{};
  m_layout->setContentsMargins(2, 0, 0, 2);
  m_layout->setVerticalSpacing(2);
  m_group->setLayout(m_layout);
}

CondensedCanvasWidget::~CondensedCanvasWidget() {}

void CondensedCanvasWidget::Focus() {
  if(m_topLeaf == nullptr) {
    return;
  }
  auto coordinate = m_nodeToViewCoordinates.at(m_topLeaf);
  auto cell = static_cast<CondensedCanvasCell*>(static_cast<QWidgetItem*>(
    m_layout->itemAtPosition(coordinate.m_row, coordinate.m_column))->widget());
  cell->setFocus();
}

void CondensedCanvasWidget::NavigateForward() {
  auto currentNodeCheck = GetCurrentNode();
  if(!currentNodeCheck.is_initialized()) {
    return;
  }
  auto& currentNode = *currentNodeCheck;
  BreadthFirstCanvasNodeIterator i{*m_node, currentNode};
  do {
    ++i;
    if(i == BreadthFirstCanvasNodeIterator{}) {
      i = BreadthFirstCanvasNodeIterator{GetRoot(*m_node)};
    }
  } while(!IsDisplayed(*i));
  if(i == BreadthFirstCanvasNodeIterator{}) {
    return;
  }
  auto& nextNode = *i;
  if(&nextNode == &currentNode) {
    return;
  }
  SetCurrent(nextNode);
}

void CondensedCanvasWidget::NavigateBackward() {
  auto currentNodeCheck = GetCurrentNode();
  if(!currentNodeCheck.is_initialized()) {
    return;
  }
  auto& currentNode = *currentNodeCheck;
  BreadthFirstCanvasNodeIterator i{*m_node, currentNode};
  do {
    auto& iNode = *i;
    --i;
    if(&iNode == &*i) {
      while(i != BreadthFirstCanvasNodeIterator{}) {
        ++i;
      }
      --i;
    }
  } while(!IsDisplayed(*i));
  if(i == BreadthFirstCanvasNodeIterator{}) {
    return;
  }
  auto& nextNode = *i;
  if(&nextNode == &currentNode) {
    return;
  }
  SetCurrent(nextNode);
}

vector<const CanvasNode*> CondensedCanvasWidget::GetRoots() const {
  vector<const CanvasNode*> roots;
  if(m_node != nullptr) {
    roots.push_back(m_node.get());
  }
  return roots;
}

boost::optional<const CanvasNode&> CondensedCanvasWidget::GetNode(
    const Coordinate& coordinate) const {
  auto node = m_modelCoordinatesToNode.find(coordinate);
  if(node == m_modelCoordinatesToNode.end()) {
    return none;
  }
  return *node->second;
}

CanvasNodeModel::Coordinate CondensedCanvasWidget::GetCoordinate(
    const CanvasNode& node) const {
  auto coordinate = m_nodeToModelCoordinates.find(&node);
  if(coordinate == m_nodeToModelCoordinates.end()) {
    BEAM_ASSERT_MESSAGE(false, "Node not found.");
  }
  return coordinate->second;
}

boost::optional<const CanvasNode&>
    CondensedCanvasWidget::GetCurrentNode() const {
  if(m_currentNode == nullptr) {
    return none;
  }
  return *m_currentNode;
}

void CondensedCanvasWidget::SetCurrent(const Coordinate& coordinate) {
  auto node = m_modelCoordinatesToNode.find(coordinate);
  if(node == m_modelCoordinatesToNode.end()) {
    return;
  }
  auto viewCoordinate = m_nodeToViewCoordinates.find(node->second);
  if(viewCoordinate == m_nodeToViewCoordinates.end()) {
    return;
  }
  auto cell = static_cast<CondensedCanvasCell*>(static_cast<QWidgetItem*>(
    m_layout->itemAtPosition(
    viewCoordinate->second.m_row, viewCoordinate->second.m_column))->widget());
  cell->setFocus();
  m_currentNode = &cell->GetNode();
}

const CanvasNode& CondensedCanvasWidget::Add(const Coordinate& coordinate,
    const CanvasNode& node) {
  assert(m_node == nullptr);
  assert(coordinate.m_row == 0);
  assert(coordinate.m_column == 0);
  Coordinate viewCoordinates{0, 0};
  Coordinate modelCoordinates{0, 0};
  m_node = CanvasNode::Clone(node);
  auto leaves = GetLeaves(*m_node);
  if(leaves.empty()) {
    BOOST_THROW_EXCEPTION(CanvasNodeNotVisibleException());
  }
  m_modelCoordinatesToNode[modelCoordinates] = m_node.get();
  m_nodeToModelCoordinates[m_node.get()] = modelCoordinates;
  m_topLeaf = leaves.front();
  m_group->setTitle(QString::fromStdString(m_name));
  boost::optional<const CanvasNode&> parent;
  View<const CanvasNode>::const_iterator childIterator;
  auto columnCount = 0;
  auto rowCount = -3;
  QSizePolicy sizePolicy{QSizePolicy::Expanding, QSizePolicy::Fixed};
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  auto maxColumns = -1;
  auto rowWithMaxColumns = -1;
  auto hasLabel = CheckForLabel(leaves);
  for(auto i = leaves.begin(); i != leaves.end(); ++i) {
    if(!IsSame((*i)->GetParent(), parent)) {
      if(columnCount > maxColumns) {
        maxColumns = columnCount;
        rowWithMaxColumns = rowCount;
      }
      columnCount = 0;
      rowCount += 3;
      modelCoordinates.m_column = 0;
      ++modelCoordinates.m_row;
      parent = (*i)->GetParent();
      childIterator = parent->GetChildren().begin();
      if(hasLabel) {
        auto label = new QLabel{QString::fromStdString(parent->GetText()),
          this};
        m_layout->addWidget(label, rowCount + 2, 0);
        ++columnCount;
      }
    }
    while(&*childIterator != *i) {
      ++childIterator;
    }
    assert(childIterator != parent->GetChildren().end());
    auto label = new QLabel{QString::fromStdString(childIterator->GetName()),
      this};
    label->setIndent(2);
    m_layout->addWidget(label, rowCount + 1, columnCount);
    auto cellWidget = new CondensedCanvasCell{Ref(*m_userProfile), Ref(*this),
      Ref(**i)};
    sizePolicy.setHeightForWidth(cellWidget->sizePolicy().hasHeightForWidth());
    cellWidget->setSizePolicy(sizePolicy);
    m_layout->addWidget(cellWidget, rowCount + 2, columnCount);
    m_nodeToViewCoordinates[*i] = Coordinate{rowCount + 2, columnCount};
    m_viewCoordinatesToNode[Coordinate{rowCount + 2, columnCount}] = *i;
    m_nodeToModelCoordinates[*i] = modelCoordinates;
    m_modelCoordinatesToNode[modelCoordinates] = *i;
    ++modelCoordinates.m_column;
    ++columnCount;
  }
  if(columnCount > maxColumns) {
    maxColumns = columnCount;
    rowWithMaxColumns = rowCount;
  }
  while(maxColumns < 5) {
    m_layout->addItem(new QSpacerItem{1, 1, QSizePolicy::Expanding,
      QSizePolicy::Fixed}, rowWithMaxColumns, maxColumns);
    ++maxColumns;
  }
  return *m_node;
}

void CondensedCanvasWidget::Remove(const Coordinate& coordinate) {
  assert(coordinate.m_row == 0);
  assert(coordinate.m_column == 0);
  m_currentNode = nullptr;
  while(auto child = m_layout->takeAt(0)) {
    auto widget = child->widget();
    if(widget != nullptr) {
      widget->hide();
      m_deletedCells.insert(std::unique_ptr<QWidget>{widget});
    }
    delete child;
  }
  m_group->setTitle("");
  m_node.reset();
  m_topLeaf = nullptr;
  m_viewCoordinatesToNode.clear();
  m_nodeToViewCoordinates.clear();
  m_nodeToModelCoordinates.clear();
  m_modelCoordinatesToNode.clear();
}

void CondensedCanvasWidget::focusInEvent(QFocusEvent* event) {
  Focus();
}
