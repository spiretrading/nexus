#include "Spire/Blotter/BlotterTasksView.hpp"
#include <QKeyEvent>
#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace boost::signals2;
using namespace Spire;

BlotterTasksView::BlotterTasksView(std::shared_ptr<BlotterModel> model,
    std::shared_ptr<CancelKeyBindingsModel> bindings, QWidget* parent)
    : QWidget(parent),
      m_bindings(std::move(bindings)),
      m_tasks(model->get_tasks()),
      m_selection(std::make_shared<TableSelectionModel>(
        std::make_shared<TableEmptySelectionModel>(),
        std::make_shared<ListMultiSelectionModel>(),
        std::make_shared<ListEmptySelectionModel>())) {
  auto layout = make_vbox_layout(this);
  auto commands = new BlotterTasksCommandBar(model);
  commands->connect_cancel_signal(
    std::bind_front(&BlotterTasksView::on_cancel, this));
  layout->addWidget(commands);
  m_table =
    new BlotterTasksTableView(m_tasks, model->get_task_monitors(), m_selection);
  layout->addWidget(m_table, 1);
}

connection BlotterTasksView::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

connection BlotterTasksView::connect_cancel_operation_signal(
    const CancelOperationSignal::slot_type& slot) const {
  return m_cancel_operation_signal.connect(slot);
}

void BlotterTasksView::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape && event->modifiers() == Qt::NoModifier &&
      m_selection->get_row_selection()->get_size() != 0) {
    on_cancel();
    event->accept();
    return;
  }
  if(m_table->hasFocus() || m_table->isAncestorOf(focusWidget())) {
    auto key = QKeySequence(event->modifiers() | event->key());
    if(auto operation = m_bindings->find_operation(key)) {
      event->accept();
      m_cancel_operation_signal(*operation);
      return;
    }
  }
  QWidget::keyPressEvent(event);
}

void BlotterTasksView::on_cancel() {
  auto& rows = std::as_const(*m_selection->get_row_selection());
  if(rows.get_size() == 0) {
    return;
  }
  auto tasks = std::vector<std::shared_ptr<Task>>();
  tasks.reserve(rows.get_size());
  for(auto row : rows) {
    tasks.push_back(m_tasks->get(row).m_task);
  }
  m_cancel_signal(tasks);
}
