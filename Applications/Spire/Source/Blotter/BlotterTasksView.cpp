#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace boost::signals2;
using namespace Spire;

BlotterTasksView::BlotterTasksView(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : QWidget(parent),
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
  layout->addWidget(new BlotterTasksTableView(
    m_tasks, model->get_task_monitors(), m_selection), 1);
}

connection BlotterTasksView::connect_cancel_tasks_signal(
    const CancelTasksSignal::slot_type& slot) const {
  return m_cancel_tasks_signal.connect(slot);
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
  m_cancel_tasks_signal(tasks);
}
