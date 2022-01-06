#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"
#include "Spire/Ui/TableModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

struct StandardTableFilter::ColumnFilter {
  ~ColumnFilter() = default;
  virtual TableFilter::Filter get_filter() const = 0;
  virtual QWidget* make_filter_widget(QWidget& parent) = 0;
  virtual bool is_filtered(const TableModel& model, int row, int column) const =
    0;
};

struct StandardTableFilter::EmptyColumnFilter : ColumnFilter {
  TableFilter::Filter get_filter() const override {
    return TableFilter::Filter::NONE;
  }

  QWidget* make_filter_widget(QWidget& parent) override {
    return nullptr;
  }

  bool is_filtered(const TableModel& model, int row, int column) const
      override {
    return false;
  }
};

struct StandardTableFilter::QuantityColumnFilter : ColumnFilter {
  std::shared_ptr<LocalValueModel<QuantityFilterPanel::Range>> m_range;

  QuantityColumnFilter()
    : m_range(
        std::make_shared<LocalValueModel<QuantityFilterPanel::Range>>()) {}

  TableFilter::Filter get_filter() const override {
    if(m_range->get() == QuantityFilterPanel::Range(none, none)) {
      return TableFilter::Filter::UNFILTERED;
    }
    return TableFilter::Filter::FILTERED;
  }

  QWidget* make_filter_widget(QWidget& parent) override {
    return new QuantityFilterPanel(m_range, "Filter quantity.", parent);
  }

  bool is_filtered(const TableModel& model, int row, int column) const
      override {
    auto& value = model.get<Quantity>(row, column);
    auto& current = m_range->get();
    return current.m_min && value < current.m_min ||
      current.m_max && value > current.m_max;
  }
};

StandardTableFilter::StandardTableFilter(std::vector<std::type_index> types) {
  for(auto& type : types) {
    if(type == typeid(Quantity)) {
      m_column_filters.push_back(std::make_unique<QuantityColumnFilter>());
    } else {
      m_column_filters.push_back(std::make_unique<EmptyColumnFilter>());
    }
  }
}

TableFilter::Filter StandardTableFilter::get_filter(int column) {
  return m_column_filters[column]->get_filter();
}

QWidget* StandardTableFilter::make_filter_widget(int column, QWidget& parent) {
  return m_column_filters[column]->make_filter_widget(parent);
}

bool StandardTableFilter::is_filtered(const TableModel& model, int row) const {
  for(auto column = 0; column != static_cast<int>(m_column_filters.size());
      ++column) {
    if(m_column_filters[column]->is_filtered(model, row, column)) {
      return true;
    }
  }
  return false;
}
