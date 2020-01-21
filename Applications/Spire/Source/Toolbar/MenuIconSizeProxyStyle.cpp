#include "Spire/Toolbar/MenuIconSizeProxyStyle.hpp"

using namespace Spire;

MenuIconSizeProxyStyle::MenuIconSizeProxyStyle(int size)
  : m_size(size) {}

int MenuIconSizeProxyStyle::pixelMetric(QStyle::PixelMetric metric,
    const QStyleOption* option, const QWidget* widget) const {
  if(metric == QStyle::PM_SmallIconSize) {
    return m_size;
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}
