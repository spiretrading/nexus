#include "Spire/Blotter/BlotterTaskView.hpp"
#include <QHeaderView>
#include "Spire/CanvasView/CanvasTableWidget.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

BlotterTaskView::BlotterTaskView(std::shared_ptr<TasksModel> tasks,
    std::shared_ptr<TableView::CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_tasks(std::move(tasks)),
      m_current(std::move(current)),
      m_canvas(new CanvasTableWidget()) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_canvas->setDragEnabled(false);
  m_canvas->setAcceptDrops(false);
  m_canvas->viewport()->setAcceptDrops(false);
  m_canvas->setDragDropMode(QAbstractItemView::NoDragDrop);
  m_canvas->setContextMenuPolicy(Qt::PreventContextMenu);
  m_canvas->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_canvas->setTabKeyNavigation(false);
  m_canvas->setShowGrid(false);
  m_canvas->horizontalHeader()->hide();
  m_canvas->verticalHeader()->hide();
  m_canvas->verticalHeader()->setDefaultSectionSize(
    QFontMetrics(font()).height() + 8);
  m_canvas->ConnectBeginEditSignal([] (const CanvasNode&) { return false; });
  enclose(*this, *m_canvas);
  setFocusProxy(m_canvas);
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&BlotterTaskView::on_current, this));
  m_task_connection = m_tasks->connect_operation_signal(
    std::bind_front(&BlotterTaskView::on_task, this));
  on_current(m_current->get());
}

void BlotterTaskView::on_current(const boost::optional<TableIndex>& current) {
  auto task = std::shared_ptr<Task>();
  if(current) {
    task = m_tasks->get(current->m_row).m_task;
  }
  if(task == m_task) {
    return;
  }
  if(m_task) {
    m_canvas->Remove(*m_canvas->GetRoots().front());
  }
  m_task = std::move(task);
  if(m_task) {
    m_canvas->Add(CanvasNodeModel::Coordinate(1, 0), m_task->GetNode());
  }
}

void BlotterTaskView::on_task(const TasksModel::Operation& operation) {
  visit(operation, [&] (const TasksModel::UpdateOperation& update) {
    auto& current = m_current->get();
    if(current && current->m_row == update.m_index) {
      on_current(current);
    }
  });
}
