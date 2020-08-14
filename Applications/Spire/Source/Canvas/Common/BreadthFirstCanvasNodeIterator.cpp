#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

BreadthFirstCanvasNodeIterator::BreadthFirstCanvasNodeIterator() {}

BreadthFirstCanvasNodeIterator::BreadthFirstCanvasNodeIterator(
    const CanvasNode& root) {
  m_forwardQueue.push_back(&root);
}

BreadthFirstCanvasNodeIterator::BreadthFirstCanvasNodeIterator(
    const CanvasNode& root, const CanvasNode& current) {
  m_forwardQueue.push_back(&root);
  while(&*(*this) != &current) {
    ++(*this);
  }
}

void BreadthFirstCanvasNodeIterator::decrement() {
  if(m_backwardQueue.empty()) {
    return;
  }
  auto previousNode = m_backwardQueue.back();
  m_forwardQueue.push_front(previousNode);
  m_backwardQueue.pop_back();
  for(const auto& child : previousNode->GetChildren()) {
    m_forwardQueue.pop_back();
  }
}

void BreadthFirstCanvasNodeIterator::increment() {
  if(m_forwardQueue.empty()) {
    return;
  }
  auto currentNode = m_forwardQueue.front();
  m_backwardQueue.push_back(currentNode);
  m_forwardQueue.pop_front();
  for(const auto& child : currentNode->GetChildren()) {
    m_forwardQueue.push_back(&child);
  }
}

bool BreadthFirstCanvasNodeIterator::equal(
    const BreadthFirstCanvasNodeIterator& rhs) const {
  if(m_forwardQueue.empty()) {
    return rhs.m_forwardQueue.empty();
  }
  if(rhs.m_forwardQueue.empty()) {
    return false;
  }
  return m_forwardQueue.front() == rhs.m_forwardQueue.front();
}

void BreadthFirstCanvasNodeIterator::advance(difference_type n) {
  while(n > 0) {
    --n;
    ++*this;
  }
}

const CanvasNode& BreadthFirstCanvasNodeIterator::dereference() const {
  return *m_forwardQueue.front();
}

View<const CanvasNode> Spire::BreadthFirstView(const CanvasNode& node) {
  return View(BreadthFirstCanvasNodeIterator(node),
    BreadthFirstCanvasNodeIterator());
}
