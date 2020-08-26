#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <QHeaderView>
#include <QImage>
#include <QLineEdit>
#include <QRect>
#include <QSize>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class CalendarDayWidget;
  class CalendarModel;
  class CalendarWidget;
  class CheckBox;
  class ColonWidget;
  class DateInputWidget;
  class DecimalSpinBox;
  class Dialog;
  class DropDownMenu;
  class DropDownMenuItem;
  class DropDownMenuList;
  class DropShadow;
  class FlatButton;
  class IconButton;
  class IntegerSpinBox;
  class MoneyInputWidget;
  class MonthAndYearSpinBox;
  class PropertiesWindowButtonsWidget;
  class QuantitySpinBox;
  class RangeInputSlider;
  class RangeInputWidget;
  class ScalarWidget;
  class ScrollArea;
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

  //! Constructs a horizontal Spire-styled table header with fixed size,
  //! immovable, column headers.
  /*
    \param parent The parent widget.
  */
  QHeaderView* make_fixed_header(QWidget* parent);

  //! Constructs a horizontal, Spire-styled, table header.
  /*
    \param parent The parent widget.
  */
  QHeaderView* make_header(QWidget* parent);
}

#endif
