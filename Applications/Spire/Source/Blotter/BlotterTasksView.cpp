#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

BlotterTasksView::BlotterTasksView(
    std::shared_ptr<BlotterModel> model, QWidget* parent)
    : QWidget(parent) {
  auto layout = make_vbox_layout(this);
  layout->addWidget(new BlotterTasksCommandBar(std::move(model)));
  layout->addWidget(new BlotterTasksTableView(), 1);
}
