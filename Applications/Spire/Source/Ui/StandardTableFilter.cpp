#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace boost::posix_time;
using namespace Nexus;
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

template<typename T>
struct StandardTableFilter::ScalarColumnFilter : ColumnFilter {
  using InputBox = T;
  using FilterPanel = ScalarFilterPanel<InputBox>;
  using Scalar = typename FilterPanel::Type;
  using Range = typename FilterPanel::Range;
  mutable FilterSignal m_filter_signal;
  std::shared_ptr<LocalValueModel<Range>> m_range;

  ScalarColumnFilter()
    : m_range(std::make_shared<LocalValueModel<Range>>()) {}

  TableFilter::Filter get_filter() const override {
    if(m_range->get() == Range(none, none)) {
      return TableFilter::Filter::UNFILTERED;
    }
    return TableFilter::Filter::FILTERED;
  }

  QWidget* make_filter_widget(QWidget& parent) override {
    auto panel = new FilterPanel(m_range, "Filter", parent);
    panel->connect_submit_signal(
      std::bind_front(&ScalarColumnFilter::on_submit, this));
    return panel;
  }

  bool is_filtered(const TableModel& model, int row, int column) const
      override {
    auto& value = model.get<Scalar>(row, column);
    auto& current = m_range->get();
    return current.m_min && value < current.m_min ||
      current.m_max && value > current.m_max;
  }

  connection connect_filter_signal(const FilterSignal::slot_type& slot) const
      override {
    return m_filter_signal.connect(slot);
  }

  void on_submit(const Range& range) {
    if(range.m_min || range.m_max) {
      m_filter_signal(Filter::FILTERED);
    } else {
      m_filter_signal(Filter::UNFILTERED);
    }
  }
};

StandardTableFilter::StandardTableFilter(std::vector<std::type_index> types) {
  for(auto column = 0; column != static_cast<int>(types.size()); ++column) {
    auto& type = types[column];
    auto filter = [&] () -> std::unique_ptr<ColumnFilter> {
      if(type == typeid(Decimal)) {
        return std::make_unique<ScalarColumnFilter<DecimalBox>>();
      } else if(type == typeid(time_duration)) {
        return std::make_unique<ScalarColumnFilter<DurationBox>>();
      } else if(type == typeid(int)) {
        return std::make_unique<ScalarColumnFilter<IntegerBox>>();
      } else if(type == typeid(Money)) {
        return std::make_unique<ScalarColumnFilter<MoneyBox>>();
      } else if(type == typeid(Quantity)) {
        return std::make_unique<ScalarColumnFilter<QuantityBox>>();
      } else {
        return std::make_unique<EmptyColumnFilter>();
      }
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
