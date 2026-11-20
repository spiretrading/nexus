#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace Spire;

BlotterTasksView::BlotterTasksView(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : QWidget(parent),
      m_selection(std::make_shared<TableSelectionModel>(
        std::make_shared<TableEmptySelectionModel>(),
        std::make_shared<ListMultiSelectionModel>(),
        std::make_shared<ListEmptySelectionModel>())) {
  auto layout = make_vbox_layout(this);
  layout->addWidget(new BlotterTasksCommandBar(model));
  layout->addWidget(new BlotterTasksTableView(
    model->get_tasks(), model->get_task_monitors(), m_selection), 1);
}
