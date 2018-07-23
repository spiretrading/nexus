#include "Spire/Canvas/Types/TaskType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TaskType& TaskType::GetInstance() {
  static auto instance = std::shared_ptr<TaskType>(new TaskType());
  return *instance;
}

string TaskType::GetName() const {
  return "Task";
}

const type_info& TaskType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TaskType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TaskType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
