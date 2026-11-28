#include "Spire/Ui/Filter.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

Filter<int>::Filter()
  : Filter(std::make_shared<LocalValueModel<Range>>()) {}

Filter<int>::Filter(std::shared_ptr<RangeModel> current) noexcept
  : m_current(std::move(current)) {}

const std::shared_ptr<Filter<int>::RangeModel>&
    Filter<int>::get_current() const {
  return m_current;
}

bool Filter<int>::is_active() const {
  auto& range = m_current->get();
  return range.m_min || range.m_max;
}

bool Filter<int>::is_filtered(const AnyRef& value) const {
  auto item = any_cast<int>(value);
  auto& range = m_current->get();
  return range.m_min && item < *range.m_min ||
    range.m_max && item > *range.m_max;
}

QWidget* Filter<int>::make_widget(QWidget& parent) {
  return new IntegerFilterPanel(m_current, Range(), &parent);
}

void Filter<int>::reset() {
  m_current->set(Range());
}

connection Filter<int>::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_current->connect_update_signal(
    std::bind_front(&Filter::on_current, slot));
}

void Filter<int>::on_current(
    const FilterSignal::slot_type& slot, const Range&) {
  slot();
}
