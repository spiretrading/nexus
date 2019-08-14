#include <QApplication>
#include <optional>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Charting/ChartValue.hpp"

using namespace Spire;

enum class RangeType : unsigned char {
  LEFT_CLOSED = 0b0001,
  LEFT_OPEN = 0b0010,
  RIGHT_CLOSED = 0b0100,
  RIGHT_OPEN = 0b1000,
  CLOSED = LEFT_CLOSED | RIGHT_CLOSED,
  OPEN = LEFT_OPEN | RIGHT_OPEN
};
struct ChartRange {
  ChartValue m_start;
  ChartValue m_end;
  RangeType m_type;
};

bool is_set(RangeType a, RangeType b) {
  return static_cast<unsigned char>(a) & static_cast<unsigned char>(b);
}

bool is_in_range(ChartValue start, ChartValue end,
    const ChartRange& range) {
  if(range.m_type == RangeType::CLOSED) {
    return range.m_start <= start && end <= range.m_end;
  } else if(range.m_type == RangeType::OPEN) {
    return range.m_start < start && end < range.m_end;
  } else if(is_set(range.m_type, RangeType::LEFT_CLOSED)) {
    return range.m_start <= start && end < range.m_end;
  }
  return range.m_start < start && end <= range.m_end;
}

bool is_before_range(ChartValue value,
    const ChartRange& range) {
  if(is_set(range.m_type, RangeType::LEFT_CLOSED)) {
    return value <= range.m_start;
  }
  return value < range.m_start;
}

bool is_after_range(ChartValue value,
    const ChartRange& range) {
  if(is_set(range.m_type, RangeType::RIGHT_CLOSED)) {
    return value >= range.m_end;
  }
  return value > range.m_end;
}

auto ranges = std::vector<ChartRange>();

void update_ranges(const ChartRange& new_range) {
  //if(ranges.empty() || is_after_range(new_range.m_start, ranges.back())) {
  //  ranges.push_back(new_range);
  //  return;
  //}
  //if(is_before_range(new_range.m_end, ranges.front())) {
  //  ranges.insert(ranges.begin(), new_range);
  //  return;
  //}
  //if(is_in_range(ranges.front().m_start, ranges.back().m_end, new_range)) {
  //  ranges = {new_range};
  //  return;
  //}
  //// the new_range at this point can not come before or after the old ranges.
  //auto new_ranges = std::vector<ChartRange>();
  //for(auto i = ranges.begin(); i != ranges.end(); ++i) {
  //  if(is_in_range(new_range.m_start, new_range.m_end, *i)) {
  //    new_ranges.push_back(*i);
  //  } else if(is_before_range(i->m_end, new_range)) {
  //    // old range comes before new_range
  //    new_ranges.push_back(*i);
  //    if(is_before_range(new_range.m_end, *std::next(i))) {
  //      new_ranges.push_back(new_range);
  //    }
  //  } else if(is_after_range(i->m_start, new_range)) {
  //    // old range comes after new_range
  //    new_ranges.push_back(*i);
  //  } else if(i->m_start == new_range.m_start) {
  //    new_ranges.push_back({new_range.m_start, std::max(i->m_end, new_range.m_end)});
  //    //if(is_set(i->m_type, RangeType::LEFT_OPEN)) {
  //    //  new_ranges.push_back({});
  //    //} else {
  //    //  new_ranges.push_back({});
  //    //}
  //  } else if(i->m_end == new_range.m_end) {
  //    new_ranges.push_back({std::min(i->m_start, new_range.m_start), new_range.m_end});
  //    //if(is_set(i->m_type, RangeType::RIGHT_OPEN)) {
  //    //  
  //    //} else {
  //    //  
  //    //}
  //  } else if(is_before_range(i->m_start, new_range) &&
  //      !is_after_range(i->m_end, new_range)) {
  //    // old start before new_range start
  //    new_ranges.push_back({i->m_start, new_range.m_end});
  //  } else if(!is_before_range(i->m_start, new_range) &&
  //      is_after_range(i->m_end, new_range)) {
  //    // old end after new_range end
  //    new_ranges.push_back({new_range.m_start, i->m_end});
  //  }
  //}
  //ranges = new_ranges;
  ranges.push_back(new_range);
  std::sort(ranges.begin(), ranges.end(),
    [] (const auto& lhs, const auto& rhs) {
      return lhs.m_start <= rhs.m_start;
    });
  auto last_range = ranges.front();
  auto new_ranges = std::vector<ChartRange>();
  new_ranges.push_back(last_range);
  for(auto i = ranges.begin() + 1; i != ranges.end(); ++i) {
    if(is_before_range(i->m_end, last_range) ||
        is_after_range(i->m_start, last_range)) {
      // isn't in or intersects last_range
      new_ranges.push_back(*i);
      last_range = *i;
    } else if(is_in_range(i->m_start, i->m_end, last_range)) {
      // range contained in last_range
      // Do nothing, remove this
    } else if(is_in_range(i->m_start, i->m_start, last_range) ||
        is_in_range(i->m_end, i->m_end, last_range)) {
      // range intersects last_range
      // TODO: add type when creating r
      auto r = ChartRange{std::min(i->m_start, last_range.m_start),
        std::max(i->m_end, last_range.m_end)};
      new_ranges.push_back(r);
      last_range = r;
    } else if(i->m_start == last_range.m_start ||
        i->m_end == last_range.m_end ) {
      // open ranges, start/end matches last_range start/end
      new_ranges.push_back(*i);
      // TODO: account for different range types
    }
  }
  ranges = new_ranges;
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  update_ranges({ChartValue(10), ChartValue(20), RangeType::OPEN});
  update_ranges({ChartValue(30), ChartValue(40), RangeType::OPEN});
  update_ranges({ChartValue(50), ChartValue(60), RangeType::OPEN});
  ranges;
  update_ranges({ChartValue(12), ChartValue(18), RangeType::OPEN}); // Contained in other range, do nothing
  update_ranges({ChartValue(22), ChartValue(28), RangeType::OPEN});
  update_ranges({ChartValue(28), ChartValue(30), RangeType::OPEN}); // overlapping open ranges
  ranges;
  update_ranges({ChartValue(0), ChartValue(100), RangeType::OPEN});
  ranges;
  application->exec();
}
