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
  using Swatch = StateSelector<void, struct SwatchSelectorTag>;

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
    style.get(Any() > Swatch()).
      set(border(scale_width(1), QColor(0, 0, 0, 51)));
    style.get(Highlighted() > Swatch()).
      set(border_color(
        chain(timeout(QColor(0xF2F2FF), milliseconds(250)),
          linear(QColor(0xF2F2FF), revert, milliseconds(300))))).
      set(BackgroundColor(
        chain(timeout(QColor(0xF2F2FF), milliseconds(250)),
          linear(QColor(0xF2F2FF), revert, milliseconds(300)))));
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
  auto swatch = new Box();
  swatch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*swatch, Swatch());
  auto box = new Box(swatch);
  enclose(*this, *box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  on_current(m_current->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&ColorSwatch::on_current, this));
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
  update_style(*this, [&] (auto& style) {
    style.get(Any() > Swatch()).set(BackgroundColor(color));
    style.get(Drag() > Swatch()).
      set(BackgroundColor(
        QColor(color.red(), color.green(), color.blue(), 204)));
  });
}
