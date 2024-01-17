#include "Spire/Ui/ColorSwatch.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(1))).
      set(vertical_padding(scale_height(1)));
    style.get(Hover() || Focus()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Drag()).
      set(BackgroundColor(QColor(255, 255, 255, 204))).
      set(border_color(QColor(0x4B23A0)));
    return style;
  }
}

ColorSwatch::ColorSwatch(QWidget* parent)
  : ColorSwatch(std::make_shared<LocalColorModel>(), parent) {}

ColorSwatch::ColorSwatch(std::shared_ptr<ColorModel> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_is_highlighted(false) {
  setFocusPolicy(Qt::StrongFocus);
  m_swatch = new Box();
  m_swatch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_swatch, [] (auto& style) {
    style.get(Any()).
      set(border(scale_width(1), QColor(0, 0, 0, 51)));
  });
  auto box = new Box(m_swatch);
  enclose(*this, *box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  on_current(m_current->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&ColorSwatch::on_current, this));
  find_stylist(*this).connect_match_signal(Drag(),
    std::bind_front(&ColorSwatch::on_drag, this));
  find_stylist(*this).connect_match_signal(Highlighted(),
    std::bind_front(&ColorSwatch::on_highlighted, this));
}

const std::shared_ptr<ColorModel>& ColorSwatch::get_current() const {
  return m_current;
}

bool ColorSwatch::is_highlighted() const {
  return m_is_highlighted;
}

void ColorSwatch::set_highlighted(bool highlighted) {
  if(highlighted == m_is_highlighted) {
    return;
  }
  m_is_highlighted = highlighted;
  if(m_is_highlighted) {
    match(*this, Highlighted());
  } else {
    unmatch(*this, Highlighted());
  }
}

void ColorSwatch::on_current(const QColor& color) {
  update_style(*m_swatch, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(color));
    style.get(Drag()).
      set(BackgroundColor(
        QColor(color.red(), color.green(), color.blue(), 204)));
    style.get(Highlighted()).
      set(border_color(
        chain(timeout(QColor(0xF2F2FF), milliseconds(250)),
          linear(QColor(0xF2F2FF), revert, milliseconds(300))))).
      set(BackgroundColor(
        chain(timeout(QColor(0xF2F2FF), milliseconds(250)),
          linear(QColor(0xF2F2FF), revert, milliseconds(300)))));
  });
}

void ColorSwatch::on_drag(bool is_match) {
  if(is_match) {
    match(*m_swatch, Drag());
  } else {
    unmatch(*m_swatch, Drag());
  }
}

void ColorSwatch::on_highlighted(bool is_match) {
  if(is_match) {
    match(*m_swatch, Highlighted());
  } else {
    unmatch(*m_swatch, Highlighted());
  }
}
