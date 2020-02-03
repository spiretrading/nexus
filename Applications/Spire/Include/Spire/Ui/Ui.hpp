#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <QImage>
#include <QLineEdit>
#include <QRect>
#include <QSize>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class CheckBox;
  class Dialog;
  class DropDownMenu;
  class DropDownMenuItem;
  class DropDownMenuList;
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

  //! Applies the Spire line edit style to a line edit, overwriting any
  //! previously set style.
  /*
    \param widget The line edit to apply the style to.
  */
  void apply_line_edit_style(QLineEdit* widget);
}

#endif
