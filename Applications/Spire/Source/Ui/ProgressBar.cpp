#include "Spire/Ui/ProgressBar.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/CubicBezierExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  int compute_fill_width(int width, int current) {
    return (width * current) / 100;
  }
}

ProgressBar::ProgressBar()
  : ProgressBar(std::make_shared<LocalProgressModel>(0)) {}

ProgressBar::ProgressBar(std::shared_ptr<ProgressModel> current)
    : m_current(std::move(current)),
      m_last_current(m_current->get()) {
  m_fill = new Box();
  match(*m_fill, Fill());
  m_fill->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  m_fill->setFixedWidth(compute_fill_width(width(), m_current->get()));
  auto box = new Box(m_fill);
  link(*this, *m_fill);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xC8C8C8)));
    style.get(Any() > Fill()).
      set(BackgroundColor(QColor(0x8D78EC)));
  });
  on_current(m_current->get());
  m_connection = m_current->connect_update_signal(
    std::bind_front(&ProgressBar::on_current, this));
}

const std::shared_ptr<ProgressModel>& ProgressBar::get_current() const {
  return m_current;
}

QSize ProgressBar::sizeHint() const {
  return scale(1, 4);
}

void ProgressBar::resizeEvent(QResizeEvent* event) {
  m_fill->setFixedWidth(compute_fill_width(width(), m_current->get()));
}

void ProgressBar::on_current(int current) {
  if(current == m_last_current) {
    return;
  }
  static const auto DURATION_PER_PIXEL = time_duration(milliseconds(2));
  static const auto MIN_DURATION = time_duration(milliseconds(30));
  static const auto MAX_DURATION = time_duration(milliseconds(600));
  auto current_width = compute_fill_width(width(), current);
  auto width_delta =
    current_width - compute_fill_width(width(), m_last_current);
  m_last_current = current;
  auto duration =
    std::clamp(DURATION_PER_PIXEL * width_delta, MIN_DURATION, MAX_DURATION);
  m_fill_width_evaluator.emplace(
    make_evaluator(ease(m_fill->width(), current_width, duration),
      find_stylist(*m_fill)));
  m_fill_width_evaluator->connect_evaluated_signal(
    std::bind_front(&ProgressBar::on_fill_width_evaluated, this));
}

void ProgressBar::on_fill_width_evaluated(int width) {
  m_fill->setFixedWidth(width);
}
