#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/TableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

struct StandardTableFilter::ColumnFilter {
  using FilterSignal = Signal<void (Filter filter)>;

  ~ColumnFilter() = default;
  virtual TableFilter::Filter get_filter() const = 0;
  virtual QWidget* make_filter_widget(QWidget& parent) = 0;
  virtual bool is_filtered(const TableModel& model, int row, int column) const =
    0;
  virtual connection connect_filter_signal(const FilterSignal::slot_type& slot)
    const = 0;
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

  connection connect_filter_signal(const FilterSignal::slot_type& slot) const
      override {
    return {};
  }
};

StandardTableFilter::StandardTableFilter(std::vector<std::type_index> types) {
  for(auto column = 0; column != static_cast<int>(types.size()); ++column) {
    auto& type = types[column];
    auto filter = [&] () -> std::unique_ptr<ColumnFilter> {
      return std::make_unique<EmptyColumnFilter>();
    }();
    filter->connect_filter_signal(
      std::bind_front(&StandardTableFilter::on_filter, this, column));
    m_column_filters.push_back(std::move(filter));
  }
}

StandardTableFilter::~StandardTableFilter() = default;

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

connection StandardTableFilter::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

void StandardTableFilter::on_filter(int index, Filter filter) {
  m_filter_signal(index, filter);
}
