#include "spire/charting/trend_line_model.hpp"

using namespace boost::signals2;
using namespace Spire;

int TrendLineModel::add(const TrendLine& line) {

}

TrendLine TrendLineModel::get(int id) const {

}

std::vector<TrendLine> TrendLineModel::get_lines() const {

}

std::vector<int> TrendLineModel::get_selected() const {

}

int TrendLineModel::intersects(const ChartPoint& point) const {

}

void TrendLineModel::remove(int id) {

}

void TrendLineModel::set_selected(int id) {

}

void TrendLineModel::unset_selected(int id) {

}

void TrendLineModel::update(const TrendLine& line, int id) {

}

connection TrendLineModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}
