#include "Spire/Blotter/TaskWidget.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "ui_TaskWidget.h"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  const CanvasNodeModel::Coordinate DISPLAY_COORDINATE(1, 0);
}

TaskWidget::TaskWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_TaskWidget>()) {
  m_ui->setupUi(this);
  m_ui->m_taskTable->installEventFilter(this);
  m_ui->m_taskTable->setDragEnabled(false);
  m_ui->m_taskTable->setAcceptDrops(false);
  QFontMetrics metrics(font());
  m_ui->m_taskTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 8);
  std::function<bool (const CanvasNode&)> vetoEditSlot =
    [] (const CanvasNode&) {
      return false;
    };
  m_ui->m_taskTable->ConnectBeginEditSignal(vetoEditSlot);
}

TaskWidget::~TaskWidget() {}

bool TaskWidget::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_taskTable) {
    if(event->type() == QEvent::DragEnter || event->type() == QEvent::Drop) {
      return true;
    }
  }
  return QWidget::eventFilter(object, event);
}

void TaskWidget::ResetDisplayedNode() {
  if(m_displayedNode != nullptr) {
    m_ui->m_taskTable->Remove(DISPLAY_COORDINATE);
  }
  m_displayedNode.reset();
}

void TaskWidget::SetDisplayedNode(const CanvasNode& displayedNode) {
  ResetDisplayedNode();
  m_displayedNode = CanvasNode::Clone(displayedNode);
  m_ui->m_taskTable->Add(DISPLAY_COORDINATE, *m_displayedNode);
}
