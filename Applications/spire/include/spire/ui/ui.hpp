#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <QImage>
#include <QRect>
#include <QSize>
#include "spire/spire/spire.hpp"

namespace Spire {
  class CheckBox;
  class ColorPicker;
  class Dialog;
  class DropdownColorPicker;
  class DropdownMenu;
  class DropdownMenuItem;
  class DropdownMenuList;
  class DropShadow;
  class FlatButton;
  class IconButton;
  class PropertiesWindowButtonsWidget;
  class SecurityStack;
  class SecurityWidget;
  class TitleBar;
  class ToggleButton;
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
}

#endif
