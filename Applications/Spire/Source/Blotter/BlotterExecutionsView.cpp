#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/ExecutionReportsToTableModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterExecutionsView::BlotterExecutionsView(
    std::shared_ptr<ExecutionReportListModel> reports, QWidget* parent)
    : QWidget(parent),
      m_reports(std::move(reports)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder =
    TableViewBuilder(std::make_shared<ExecutionReportsToTableModel>(m_reports));
  table_view_builder.add_header_item(tr("Time"));
  table_view_builder.add_header_item(tr("ID"));
  table_view_builder.add_header_item(tr("Side"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Status"));
  table_view_builder.add_header_item(tr("Quantity"), tr("Qty"));
  table_view_builder.add_header_item(tr("Last Quantity"), tr("Last Qty"));
  table_view_builder.add_header_item(tr("Price"), tr("Px"));
  table_view_builder.add_header_item(tr("Last Price"), tr("Last Px"));
  table_view_builder.add_header_item(tr("Market"), tr("Mkt"));
  table_view_builder.add_header_item(tr("Flag"));
  table_view_builder.add_header_item(tr("Execution Fee"), tr("Exec Fee"));
  table_view_builder.add_header_item(tr("Processing Fee"), tr("Proc Fee"));
  table_view_builder.add_header_item(tr("Miscellaneous Fee"), tr("Misc Fee"));
  table_view_builder.add_header_item(tr("Message"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  enclose(*this, *scroll_box);
}

const std::shared_ptr<ExecutionReportListModel>&
    BlotterExecutionsView::get_reports() const {
  return m_reports;
}