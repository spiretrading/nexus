#include "Spire/Ui/StandardTableFilter.hpp"
#include <QPointer>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/DateFilterPanel.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  template<class... Ts>
  struct Overloaded : Ts... {
    using Ts::operator()...;
  };

  template<class... Ts>
  Overloaded(Ts...) -> Overloaded<Ts...>;
}

struct StandardTableFilter::ColumnFilter {
  using FilterSignal = Signal<void (Filter filter)>;
  QPointer<QWidget> m_filter_widget;

  ~ColumnFilter() = default;
  virtual TableFilter::Filter get_filter() const = 0;
  virtual QWidget* make_panel(QWidget& parent) = 0;
  virtual bool is_filtered(const TableModel& model, int row, int column) const =
    0;
  virtual connection connect_filter_signal(const FilterSignal::slot_type& slot)
    const = 0;

  QWidget* make_filter_widget(QWidget& parent) {
    if(!m_filter_widget) {
      m_filter_widget = make_panel(parent);
    }
    return m_filter_widget;
  }
};

struct StandardTableFilter::DateColumnFilter : ColumnFilter {
  using DateRange = DateFilterPanel::DateRange;
  static constexpr auto DEFAULT_RANGE =
    DateRange(DateFilterPanel::AbsoluteDateRange());
  mutable FilterSignal m_filter_signal;
  std::shared_ptr<LocalValueModel<DateRange>> m_current;
  scoped_connection m_current_connection;

  DateColumnFilter()
      : m_current(std::make_shared<LocalValueModel<DateRange>>(DEFAULT_RANGE)) {
    m_current->connect_update_signal(
      std::bind_front(&DateColumnFilter::on_current, this));
  }

  TableFilter::Filter get_filter() const override {
    if(m_current->get() == DEFAULT_RANGE) {
      return TableFilter::Filter::UNFILTERED;
    }
    return TableFilter::Filter::FILTERED;
  }

  QWidget* make_panel(QWidget& parent) override {
    return new DateFilterPanel(m_current, &parent);
  }

  bool is_filtered(const TableModel& model, int row, int column) const
      override {
    auto& value = any_cast<const date>(model.at(row, column));
    auto [start, end] = resolve(m_current->get());
    return !start.is_not_a_date() && value < start ||
      !end.is_not_a_date() && value > end;
  }

  connection connect_filter_signal(
      const FilterSignal::slot_type& slot) const override {
    return m_filter_signal.connect(slot);
  }

  void on_current(const DateRange&) {
    m_filter_signal(get_filter());
  }

  static std::tuple<date, date> resolve(const DateRange& range) {
    return std::visit(Overloaded {
      [] (const DateFilterPanel::AbsoluteDateRange& range) {
        return std::tuple(range.m_start, range.m_end);
      },
      [] (const DateFilterPanel::RelativeDateRange& range) {
        auto today = day_clock::local_day();
        auto start = today;
        if(range.m_unit == DateFilterPanel::DateUnit::DAY) {
          start -= days(range.m_value);
        } else if(range.m_unit == DateFilterPanel::DateUnit::WEEK) {
          start -= weeks(range.m_value);
        } else if(range.m_unit == DateFilterPanel::DateUnit::MONTH) {
          start -= months(range.m_value);
        } else if(range.m_unit == DateFilterPanel::DateUnit::YEAR) {
          start -= years(range.m_value);
        }
        return std::tuple(start, today);
      }}, range);
  }
};

struct StandardTableFilter::EmptyColumnFilter : ColumnFilter {
  TableFilter::Filter get_filter() const override {
    return TableFilter::Filter::NONE;
  }

  QWidget* make_panel(QWidget& parent) override {
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
      : m_range(std::make_shared<LocalValueModel<Range>>()) {
    m_range->connect_update_signal(
      std::bind_front(&ScalarColumnFilter::on_current, this));
  }

  TableFilter::Filter get_filter() const override {
    if(m_range->get() == Range(none, none)) {
      return TableFilter::Filter::UNFILTERED;
    }
    return TableFilter::Filter::FILTERED;
  }

  QWidget* make_panel(QWidget& parent) override {
    return new FilterPanel(m_range, &parent);
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

  void on_current(const Range&) {
    m_filter_signal(get_filter());
  }
};

StandardTableFilter::StandardTableFilter(std::vector<std::type_index> types) {
  for(auto column = 0; column != static_cast<int>(types.size()); ++column) {
    auto& type = types[column];
    auto filter = [&] () -> std::unique_ptr<ColumnFilter> {
      if(type == typeid(date)) {
        return std::make_unique<DateColumnFilter>();
      } else if(type == typeid(Decimal)) {
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
