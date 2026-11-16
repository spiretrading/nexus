#include "Spire/Blotter/BlotterTasksView.hpp"
#include "Spire/Blotter/BlotterTasksCommandBar.hpp"
#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

BlotterTasksView::BlotterTasksView(QWidget* parent)
    : QWidget(parent) {
  auto layout = make_vbox_layout(this);
  layout->addWidget(new BlotterTasksCommandBar());
  layout->addWidget(new BlotterTasksTableView(), 1);
}
