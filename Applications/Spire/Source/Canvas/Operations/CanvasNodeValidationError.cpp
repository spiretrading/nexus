#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

CanvasNodeValidationError::CanvasNodeValidationError() {}

CanvasNodeValidationError::CanvasNodeValidationError(
    Ref<const CanvasNode> node, string message)
    : m_node(node.get()),
      m_message(std::move(message)) {}

const CanvasNode& CanvasNodeValidationError::GetNode() const {
  return *m_node;
}

const string& CanvasNodeValidationError::GetErrorMessage() const {
  return m_message;
}
