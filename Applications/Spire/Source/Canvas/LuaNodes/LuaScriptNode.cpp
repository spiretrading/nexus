#include "Spire/Canvas/LuaNodes/LuaScriptNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace std;

LuaScriptNode::Parameter::Parameter()
    : m_type(IntegerType::GetInstance()) {}

LuaScriptNode::Parameter::Parameter(string name, const NativeType& type)
    : m_name(std::move(name)),
      m_type(type) {}

LuaScriptNode::LuaScriptNode()
    : m_name("Undefined") {
  SetText("Lua: " + m_name);
  SetType(UnionType::GetEmptyType());
}

LuaScriptNode::LuaScriptNode(string name, const NativeType& type,
    const std::filesystem::path& path, vector<Parameter> parameters)
    : m_name(std::move(name)),
      m_path(path),
      m_parameters(std::move(parameters)) {
  SetText("Lua: " + m_name);
  for(const auto& parameter : m_parameters) {
    AddChild(parameter.m_name, MakeDefaultCanvasNode(*parameter.m_type));
  }
  SetType(type);
}

const string& LuaScriptNode::GetName() const {
  return m_name;
}

const std::filesystem::path& LuaScriptNode::GetPath() const {
  return m_path;
}

const vector<LuaScriptNode::Parameter>& LuaScriptNode::GetParameters() const {
  return m_parameters;
}

void LuaScriptNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> LuaScriptNode::Clone() const {
  return make_unique<LuaScriptNode>(*this);
}

LuaScriptNode::LuaScriptNode(ReceiveBuilder) {}
