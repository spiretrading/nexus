#include "Spire/CanvasView/PasteNodeCommand.hpp"
#include <QApplication>
#include <QClipboard>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"
#include "Spire/UI/IgnoreCommandException.hpp"
#include "Spire/UI/MacroUndoCommand.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

PasteNodeCommand::PasteNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& coordinate)
    : m_view(view.Get()),
      m_coordinate(coordinate) {}

PasteNodeCommand::~PasteNodeCommand() {}

void PasteNodeCommand::undo() {
  m_placeCommand->undo();
}

void PasteNodeCommand::redo() {
  if(m_placeCommand == nullptr) {
    auto clipboardData = QApplication::clipboard()->mimeData();
    auto copiedNodes = DecodeFromMimeData(*clipboardData);
    if(copiedNodes.empty()) {
      BOOST_THROW_EXCEPTION(IgnoreCommandException());
    }
    m_placeCommand = std::make_unique<MacroUndoCommand>();
    for(const auto& node : copiedNodes) {
      auto command = new PlaceNodeCommand(Ref(*m_view), m_coordinate, *node);
      m_placeCommand->AddCommand(std::move(command));
    }
  }
  m_placeCommand->redo();
}
