#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <boost/optional/optional.hpp>
#include <QHeaderView>
#include <QImage>
#include <QLineEdit>
#include <QRect>
#include <QSize>
#include <QString>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class Box;
  class Button;
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
  class FilteredTableModel;
  class FlatButton;
  class Icon;
  class IconButton;
  class IntegerBox;
  class IntegerSpinBox;
  class KeySequenceInputField;
  class LayeredWidget;
  template<typename T> class LocalScalarValueModel;
  template<typename T> class LocalValueModel;
  using LocalBooleanModel = LocalValueModel<bool>;
  using LocalIntegerModel = LocalScalarValueModel<int>;
  using LocalOptionalIntegerModel = LocalScalarValueModel<boost::optional<int>>;
  using LocalTextModel = LocalValueModel<QString>;
  class MoneyInputWidget;
  class MonthAndYearSpinBox;
  class PropertiesWindowButtonsWidget;
  class QuantitySpinBox;
  class RangeInputSlider;
  class RangeInputWidget;
  class RealSpinBox;
  template<typename T> class ScalarValueModel;
  using OptionalIntegerModel = ScalarValueModel<boost::optional<int>>;
  using IntegerModel = ScalarValueModel<int>;
  class ScalarWidget;
  class ScrollArea;
  class ScrollBar;
  class ScrollBox;
  class ScrollableLayer;
  class SecurityStack;
  class SecurityWidget;
  class TabWidget;
  class TableModel;
  class TableModelTransactionLog;
  class TextBox;
  class TitleBar;
  class Tooltip;
  class TransitionWidget;
  class TranslatedTableModel;
  template<typename T> class ValueModel;
  using BoolModel = ValueModel<bool>;
  using TextModel = ValueModel<QString>;
  class Window;

  //! Draws a border inside the given region.
  /*!
    \param region The region to draw a border inside of.
    \param color The border's color.
    \param painter The QPainter instance that draws the border.
  */
  void draw_border(const QRect& region, const QColor& color,
    QPainter* painter);

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
