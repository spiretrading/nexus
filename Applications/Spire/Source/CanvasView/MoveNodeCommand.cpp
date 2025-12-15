#include "Spire/CanvasView/MoveNodeCommand.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"
#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/LegacyUI/IgnoreCommandException.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  void ResetSourceNode(CanvasNodeBuilder& builder, const CanvasNode& source) {
    auto reference = dynamic_cast<const ReferenceNode*>(&source);
    if(reference == nullptr) {
      builder.Reset(source);
      return;
    }
    builder.Replace(source, MakeDefaultCanvasNode(source.GetType()));
  }
}

MoveNodeCommand::MoveNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& source,
    const CanvasNodeModel::Coordinate& destination)
    : m_view(view.get()),
      m_source(source),
      m_destination(destination) {}

void MoveNodeCommand::undo() {
  m_snapshot.Restore(out(*m_view));
}

void MoveNodeCommand::redo() {
  m_snapshot.Save(*m_view);
  auto source = m_view->GetNode(m_source);
  assert(source);
  auto destination = m_view->GetNode(m_destination);
  if(destination.is_initialized()) {
    if(IsRoot(*source)) {
      auto dropNode = CanvasNode::Clone(*source);
      m_view->Remove(*source);
      auto placedNode = PlaceNodeCommand::TryPlaceNode(out(*m_view),
        m_destination, *dropNode, false);
      if(!placedNode.is_initialized()) {
        m_snapshot.Restore(out(*m_view));
        BOOST_THROW_EXCEPTION(IgnoreCommandException());
      }
    } else if(&GetRoot(*source) == &GetRoot(*destination)) {
      auto dropNode = CanvasNode::Clone(*source);
      CanvasNodeBuilder builder(GetRoot(*source));
      unique_ptr<CanvasNode> replacement;
      try {
        if(!IsParent(*m_view->GetNode(m_destination), *source)) {
          ResetSourceNode(builder, *source);
        }
        builder.Replace(*m_view->GetNode(m_destination), std::move(dropNode));
        replacement = builder.Make();
      } catch(const std::exception&) {
        m_snapshot.Restore(out(*m_view));
        BOOST_THROW_EXCEPTION(IgnoreCommandException());
      }
      ReplaceNodeCommand::Replace(out(*m_view), GetRoot(*source),
        *replacement);
    } else {
      auto dropNode = CanvasNode::Clone(*source);
      {
        CanvasNodeBuilder builder(GetRoot(*source));
        unique_ptr<CanvasNode> replacement;
        try {
          ResetSourceNode(builder, *source);
          replacement = builder.Make();
        } catch(const std::exception&) {
          m_snapshot.Restore(out(*m_view));
          BOOST_THROW_EXCEPTION(IgnoreCommandException());
        }
        ReplaceNodeCommand::Replace(out(*m_view), GetRoot(*source),
          *replacement);
      }
      {
        CanvasNodeBuilder builder(GetRoot(*destination));
        unique_ptr<CanvasNode> replacement;
        try {
          builder.Replace(*destination, std::move(dropNode));
          replacement = builder.Make();
        } catch(const std::exception&) {
          m_snapshot.Restore(out(*m_view));
          BOOST_THROW_EXCEPTION(IgnoreCommandException());
        }
        ReplaceNodeCommand::Replace(out(*m_view), GetRoot(*destination),
          *replacement);
      }
    }
  } else {
    if(IsRoot(*source)) {
      auto dropNode = CanvasNode::Clone(*source);
      m_view->Remove(*source);
      auto& placedNode = m_view->Add(m_destination, *dropNode);
      m_view->SetCurrent(placedNode);
    } else if(!source->IsReadOnly()) {
      auto dropNode = CanvasNode::Clone(*source);
      CanvasNodeBuilder builder(GetRoot(*source));
      unique_ptr<CanvasNode> replacement;
      try {
        ResetSourceNode(builder, *source);
        replacement = builder.Make();
      } catch(const std::exception&) {
        m_snapshot.Restore(out(*m_view));
        BOOST_THROW_EXCEPTION(IgnoreCommandException());
      }
      ReplaceNodeCommand::Replace(out(*m_view), GetRoot(*source),
        *replacement);
      m_view->Add(m_destination, *Refresh(std::move(dropNode)));
    } else {
      auto dropNode = CanvasNode::Clone(*source);
      m_view->Add(m_destination, *Refresh(std::move(dropNode)));
    }
  }
}
