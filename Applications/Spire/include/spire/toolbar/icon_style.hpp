#ifndef SPIRE_ICON_STYLE_HPP
#define SPIRE_ICON_STYLE_HPP
#include <QProxyStyle>
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! \brief Enables a custom style for menu icons.
  class icon_style : public QProxyStyle {
    public:

      int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr);
  };
}

#endif
