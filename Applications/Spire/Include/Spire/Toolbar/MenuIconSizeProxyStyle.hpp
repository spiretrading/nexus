#ifndef SPIRE_MENU_ICON_SIZE_PROXY_STYLE_HPP
#define SPIRE_MENU_ICON_SIZE_PROXY_STYLE_HPP
#include <QProxyStyle>

namespace Spire {

  class MenuIconSizeProxyStyle : public QProxyStyle {
    public:

      MenuIconSizeProxyStyle(int size);

      int pixelMetric(QStyle::PixelMetric metric,
        const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr) const override;

    private:
      int m_size;
  };
}

#endif
