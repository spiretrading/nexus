#include "Spire/CanvasView/PopulateCanvasNodeContextMenu.hpp"
#include <QMenu>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/CanvasView/CopyNodeCommand.hpp"
#include "Spire/CanvasView/CutNodeCommand.hpp"
#include "Spire/CanvasView/PasteNodeCommand.hpp"
#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/CanvasView/SetNodeReadOnlyCommand.hpp"
#include "Spire/CanvasView/SetNodeVisibleCommand.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/UI/CommandAction.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

void Spire::PopulateCanvasNodeContextMenu(Ref<CanvasNodeModel> view,
    const CanvasNode& node, Ref<UserProfile> userProfile, Out<QMenu> menu) {
  auto coordinate = view->GetCoordinate(node);
  auto cutAction = new CommandAction(
    make_unique<CutNodeCommand>(Ref(view), coordinate), menu.get());
  cutAction->setText(QObject::tr("Cut"));
  cutAction->setToolTip(QObject::tr(
    "Removes the selected item and moves it to the clipboard."));
  cutAction->setShortcut(QKeySequence::Cut);
  menu->addAction(cutAction);
  auto copyAction = new CommandAction(make_unique<CopyNodeCommand>(node),
    menu.get());
  copyAction->setText(QObject::tr("Copy"));
  copyAction->setToolTip(QObject::tr(
    "Copies the selected item to the clipboard."));
  copyAction->setShortcut(QKeySequence::Copy);
  menu->addAction(copyAction);
  auto pasteAction = new CommandAction(make_unique<PasteNodeCommand>(
    Ref(view), coordinate), menu.get());
  pasteAction->setText(QObject::tr("Paste"));
  pasteAction->setToolTip(QObject::tr("Pastes an item from the clipboard."));
  pasteAction->setShortcut(QKeySequence::Paste);
  pasteAction->setEnabled(CheckClipboardForCanvasNode());
  menu->addAction(pasteAction);
  menu->addSeparator();
  auto readOnlyAction = new CommandAction(make_unique<SetNodeReadOnlyCommand>(
    Ref(view), coordinate, !node.IsReadOnly()), menu.get());
  if(node.IsReadOnly()) {
    readOnlyAction->setText(QObject::tr("Unset read-only"));
    readOnlyAction->setToolTip(QObject::tr("Unset the read-only property."));
  } else {
    readOnlyAction->setText(QObject::tr("Set read-only"));
    readOnlyAction->setToolTip(QObject::tr("Set to read-only."));
  }
  menu->addAction(readOnlyAction);
  if(node.IsVisible() && !IsRoot(node)) {
    auto parentCoordinate = view->GetCoordinate(*node.GetParent());
    auto visibilityAction = new CommandAction(
      make_unique<SetNodeVisibleCommand>(Ref(view), parentCoordinate,
      node.GetName(), false), menu.get());
    visibilityAction->setText(QObject::tr("Hide"));
    visibilityAction->setToolTip(QObject::tr("Hides this item."));
    menu->addAction(visibilityAction);
  }
  vector<const CanvasNode*> invisibleChildren;
  for(const auto& child : node.GetChildren()) {
    if(!child.IsVisible()) {
      invisibleChildren.push_back(&child);
    }
  }
  if(!invisibleChildren.empty()) {
    auto showMenu = new QMenu(QObject::tr("Show child"), menu.get());
    for(const auto& child : invisibleChildren) {
      auto visibilityAction = new CommandAction(
        make_unique<SetNodeVisibleCommand>(Ref(view), coordinate,
        child->GetName(), true), showMenu);
      visibilityAction->setText(QString::fromStdString(child->GetName()));
      showMenu->addAction(visibilityAction);
    }
    menu->addMenu(showMenu);
  }
}
