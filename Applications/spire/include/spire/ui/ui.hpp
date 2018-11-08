#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <QImage>
#include <QRect>
#include <QSize>
#include "Nexus/Definitions/Quantity.hpp"
#include "spire/spire/spire.hpp"

namespace Spire {
  class CheckBox;
  class DropShadow;
  class FlatButton;
  class IconButton;
  class PropertiesWindowButtonsWidget;
  class SecurityStack;
  class TitleBar;
  class TransitionWidget;
  class Window;

  //! Builds a QImage from an SVG resource where the size of the SVG is equal
  //! to the image as a whole.
  /*!
    \param path The path to the SVG resource.
    \param size The size of the image to render.
    \return An image rendered from the SVG resource to the specified size.
  */
  QImage imageFromSvg(const QString& path, const QSize& size);

  //! Builds a QImage from an SVG resource where the size of the SVG is embedded
  //! within the image.
  /*!
    \param path The path to the SVG resource.
    \param size The size of the image to render.
    \param box The location and size to render the SVG within the image.
    \return An image rendered from the SVG resource to the specified dimensions.
  */
  QImage imageFromSvg(const QString& path, const QSize& size, const QRect& box);

  //! Creates a comma delimited number from the supplied Quantity.
  /*
    \param quantity The quantity to add commas to.
    \param precision The number of decimal places to preserve in the returned
            number. Trailing zeros are removed.
    \param locale The locale to use when converting the number to a string.
  */
  QString displayed_quantity(const Nexus::Quantity& quantity, int precision,
    const QLocale& locale);
}

#endif
