#include "Spire/Canvas/Types/TaskStateType.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

using namespace Spire;
using namespace std;

const TaskStateType& TaskStateType::GetInstance() {
  static auto instance = std::shared_ptr<TaskStateType>(new TaskStateType());
  return *instance;
}

string TaskStateType::GetName() const {
  return "Task State";
}

const type_info& TaskStateType::GetNativeType() const {
  return typeid(Type);
}

CanvasType::Compatibility TaskStateType::GetCompatibility(
    const CanvasType& type) const {
  if(typeid(type) == typeid(*this)) {
    return Compatibility::EQUAL;
  }
  return Compatibility::NONE;
}

void TaskStateType::Apply(CanvasTypeVisitor& visitor) const {
  visitor.Visit(*this);
}
