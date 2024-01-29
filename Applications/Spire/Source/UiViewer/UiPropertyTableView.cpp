#include "Spire/UiViewer/UiPropertyTableView.hpp"
#include <QHeaderView>
#include <QVBoxLayout>

using namespace Spire;

namespace {
  const auto NAME_COLUMN = 0;
  const auto VALUE_COLUMN = 1;
  const auto COLUMNS = 2;
}

UiPropertyTableView::UiPropertyTableView(
    std::vector<std::shared_ptr<UiProperty>> properties, QWidget* parent)
    : QWidget(parent),
      m_properties(std::move(properties)) {
  auto layout = new QVBoxLayout(this);
  m_table = new QTableWidget(m_properties.size(), COLUMNS, this);
  m_table->verticalHeader()->hide();
  m_table->horizontalHeader()->setHighlightSections(false);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->setHorizontalHeaderItem(NAME_COLUMN,
    new QTableWidgetItem(tr("Name")));
  m_table->setHorizontalHeaderItem(VALUE_COLUMN,
    new QTableWidgetItem(tr("Value")));
  for(auto i = 0; i != static_cast<int>(m_properties.size()); ++i) {
    auto& property = *m_properties[i];
    auto name_item = new QTableWidgetItem(property.get_name());
    name_item->setFlags(Qt::ItemIsEnabled);
    m_table->setItem(i, NAME_COLUMN, name_item);
    auto container = new QWidget(m_table);
    auto container_layout = new QHBoxLayout(container);
    container_layout->setAlignment(Qt::AlignCenter);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->addWidget(property.make_setter_widget(container));
    m_table->setCellWidget(i, VALUE_COLUMN, container);
  }
  layout->addWidget(m_table);
  setLayout(layout);
}
