#ifndef SPIRE_UI_HPP
#define SPIRE_UI_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <QHeaderView>
#include <QImage>
#include <QPropertyAnimation>
#include <QRect>
#include <QSize>
#include <QString>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class ArrayTableModel;
  class Box;
  class Button;
  class CalendarDatePicker;
  class CheckBox;
  class ClosedFilterPanel;
  class ComboBox;
  class ContextMenu;
  class DateBox;
  class DateFilterPanel;
  class DecimalBox;
  class DestinationBox;
  class DestinationListItem;
  class DropDownBox;
  class DropDownList;
  class DropShadow;
  class DurationBox;
  class EmptyTableFilter;
  template<typename T> class EnumBox;
  class FilteredTableModel;
  class FilterPanel;
  class FocusObserver;
  class GlobalPositionObserver;
  struct Highlight;
  class HoverObserver;
  class Icon;
  class InfoTip;
  class KeyInputBox;
  class LayeredWidget;
  class ListItem;
  class ListView;
  class MoneyBox;
  class NavigationView;
  class OverlayPanel;
  class RegionListItem;
  template<typename T> class ScalarFilterPanel;
  class ScrollBar;
  class ScrollBox;
  class ScrollableLayer;
  class ScrollableListBox;
  class SearchBox;
  class SecurityBox;
  class SecurityListItem;
  class SecurityStack;
  class SortedTableModel;
  template<typename T> class StagingValueModel;
  class StandardTableFilter;
  class SubmenuItem;
  class TableBody;
  class TableHeader;
  class TableHeaderItem;
  class TableFilter;
  class TableItem;
  class TableModel;
  class TableView;
  class Tag;
  class TagBox;
  class TextAreaBox;
  class TextBox;
  class TimeBox;
  class TitleBar;
  class Tooltip;
  class TransitionWidget;
  class TranslatedTableModel;
  class Window;
  class WindowObserver;

  /**
   * Draws a border inside the given region.
   * @param region The region to draw a border inside of.
   * @param color The border's color.
   * @param painter The QPainter instance that draws the border.
   */
  void draw_border(const QRect& region, const QColor& color, QPainter* painter);

  /**
   * Fades a window in by animating its window opacity. The returned animation
   * is deleted on completion.
   * @param target The window to fade in or out. Used as the parent for the
   *               returned QPropertyAnimation.
   * @param reverse True iff the animation should be reversed (fade out).
   * @param fade_speed_ms The fade speed, in milliseconds.
   * @returns The animation assigned to the target.
   */
  QPropertyAnimation* fade_window(
    QObject* target, bool reverse, boost::posix_time::time_duration fade_speed);

  /**
   * Builds a QImage from an SVG resource where the size of the SVG is equal
   * to the image as a whole.
   * @param path The path to the SVG resource.
   * @param size The size of the image to render.
   * @return An image rendered from the SVG resource to the specified size.
   */
  QImage imageFromSvg(const QString& path, const QSize& size);

  /**
   * Builds a QImage from an SVG resource where the size of the SVG is embedded
   * within the image.
   * @param path The path to the SVG resource.
   * @param size The size of the image to render.
   * @param box The location and size to render the SVG within the image.
   * @return An image rendered from the SVG resource to the specified
   *         dimensions.
   */
  QImage imageFromSvg(const QString& path, const QSize& size, const QRect& box);

  /**
   * Constructs a horizontal Spire-styled table header with fixed size,
   * immovable, column headers.
   * @param parent The parent widget.
   */
  QHeaderView* make_fixed_header(QWidget* parent);

  /**
   * Constructs a horizontal, Spire-styled, table header.
   * @param parent The parent widget.
   */
  QHeaderView* make_header(QWidget* parent);
}

#endif
