#include "Spire/Canvas/Records/QueryNode.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  enum {
    RECORD,
  };
}

QueryNode::QueryNode()
    : m_type(UnionType::GetAnyValueType()) {
  SetText("Query");
  SetType(*m_type);
  AddChild(
    "record", std::make_unique<NoneNode>(RecordType::GetEmptyRecordType()));
}

const string& QueryNode::GetField() const {
  return m_field;
}

unique_ptr<QueryNode> QueryNode::SetField(const string& field) const {
  auto& record = static_cast<const RecordType&>(
    GetChildren()[RECORD].GetType());
  for(const auto& recordField : record.GetFields()) {
    if(recordField.m_name == field) {
      if(!IsCompatible(*m_type, *recordField.m_type)) {
        BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
      }
      auto clone = CanvasNode::Clone(*this);
      clone->SetType(*recordField.m_type);
      clone->m_field = recordField.m_name;
      clone->SetText("Query: " + recordField.m_name);
      return clone;
    }
  }
  BOOST_THROW_EXCEPTION(CanvasOperationException("Field not found"));
}

unique_ptr<CanvasNode> QueryNode::Convert(const CanvasType& type) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_type = type;
  auto& record = static_cast<const RecordType&>(
    GetChildren()[RECORD].GetType());
  auto currentField = FindField(record, m_field);
  if(currentField.is_initialized() &&
      IsCompatible(*clone->m_type, *currentField->m_type)) {
    clone->SetType(*currentField->m_type);
    return std::move(clone);
  }
  for(const auto& field : record.GetFields()) {
    if(IsCompatible(*clone->m_type, *field.m_type)) {
      clone->m_field = field.m_name;
      clone->SetText("Query: " + field.m_name);
      clone->SetType(*field.m_type);
      return std::move(clone);
    }
  }
  clone->m_field.clear();
  clone->SetText("Query");
  clone->SetType(type);
  return std::move(clone);
}

unique_ptr<CanvasNode> QueryNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  std::shared_ptr<CanvasType> replacementType = replacement->GetType();
  if(!IsCompatible(GetChildren().front().GetType(), *replacementType)) {
    try {
      auto conversion = Spire::Convert(CanvasNode::Clone(*replacement),
        GetChildren().front().GetType());
      return Replace(child, std::move(conversion));
    } catch(const CanvasOperationException&) {
      if(!IsCompatible(RecordType::GetEmptyRecordType(), *replacementType)) {
        auto conversion = Spire::Convert(std::move(replacement),
          RecordType::GetEmptyRecordType());
        return Replace(child, std::move(conversion));
      }
    }
  }
  auto& recordType = static_cast<const RecordType&>(*replacementType);
  auto currentField = FindField(recordType, m_field);
  if(currentField.is_initialized() &&
      IsCompatible(*m_type, *currentField->m_type)) {
    auto clone = CanvasNode::Clone(*this);
    clone->SetChild(child, std::move(replacement));
    clone->SetType(*currentField->m_type);
    return std::move(clone);
  }
  for(const auto& field : recordType.GetFields()) {
    if(IsCompatible(*m_type, *field.m_type)) {
      auto clone = CanvasNode::Clone(*this);
      clone->m_field = field.m_name;
      clone->SetText("Query: " + field.m_name);
      clone->SetChild(child, std::move(replacement));
      clone->SetType(*field.m_type);
      return std::move(clone);
    }
  }
  auto clone = CanvasNode::Clone(*this);
  clone->m_field.clear();
  clone->SetText("Query");
  clone->SetChild(child, std::move(replacement));
  return std::move(clone);
}

void QueryNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

std::unique_ptr<CanvasNode> QueryNode::Clone() const {
  return std::make_unique<QueryNode>(*this);
}
