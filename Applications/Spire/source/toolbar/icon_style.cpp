#include "spire/toolbar/icon_style.hpp"
#include "spire/spire/dimensions.hpp"

using namespace spire;

int icon_style::pixelMetric(PixelMetric metric,
    const QStyleOption* option,
    const QWidget* widget) {
  if(metric == QStyle::PM_SmallIconSize) {
    return 200;
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}
