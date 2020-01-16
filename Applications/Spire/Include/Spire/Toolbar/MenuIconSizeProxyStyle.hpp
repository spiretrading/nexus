#ifndef SPIRE_MENU_ICON_SIZE_PROXY_STYLE_HPP
#define SPIRE_MENU_ICON_SIZE_PROXY_STYLE_HPP
#include <QProxyStyle>

namespace Spire {

  //! Defines a custom icon size for a QMenu style.
  class MenuIconSizeProxyStyle : public QProxyStyle {
    public:

      //! Constructs a MenuIconProxyStyle with the given icon size.
      /*
        \param size The icon size.
      */
      explicit MenuIconSizeProxyStyle(int size);

      int pixelMetric(QStyle::PixelMetric metric,
        const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr) const override;

    private:
      int m_size;
  };
}

#endif
