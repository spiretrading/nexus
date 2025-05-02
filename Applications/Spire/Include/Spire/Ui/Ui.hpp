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
  class AdaptiveBox;
  class AnyComboBox;
  class AnyInputBox;
  class AnyTagComboBox;
  class Box;
  class BoxGeometry;
  class BoxPainter;
  class Button;
  class CalendarDatePicker;
  class CheckBox;
  class ClickObserver;
  class ClosedFilterPanel;
  class ColorBox;
  class ColorCodePanel;
  class ColorPicker;
  class ColorSwatch;
  template<typename T> class ComboBox;
  class ContextMenu;
  class DateBox;
  class DateFilterPanel;
  class DecimalBox;
  class DeletableListItem;
  class DestinationListItem;
  class DropDownBox;
  class DropDownList;
  class DropShadow;
  class DurationBox;
  class EditableBox;
  class EditableTableView;
  template<typename T> class EmptySelectionModel;
  class EmptyTableFilter;
  class EnabledObserver;
  template<typename T> class EnumBox;
  class EyeDropper;
  class EyeDropperButton;
  class FilterPanel;
  class FixedHorizontalLayout;
  class FocusObserver;
  class FontBox;
  class GlobalPositionObserver;
  class HexColorBox;
  struct Highlight;
  class HighlightBox;
  class HighlightPicker;
  class HighlightSwatch;
  class HoverObserver;
  class Icon;
  class InfoPanel;
  class InfoTip;
  class KeyInputBox;
  class KeyObserver;
  class LayeredWidget;
  class LineInputForm;
  class ListItem;
  template<typename T> class ListSelectionValueModel;
  class ListView;
  class MenuBox;
  class MenuButton;
  class MoneyBox;
  class MouseMoveObserver;
  class MouseObserver;
  template<typename T> class MultiSelectionModel;
  class NavigationView;
  class OverlayPanel;
  class PercentBox;
  class PopupBox;
  class PressObserver;
  class ProgressBar;
  class RegionBox;
  class RegionListItem;
  class ResponsiveLabel;
  template<typename T> class ScalarFilterPanel;
  class ScrollBar;
  class ScrollBox;
  class ScrollableLayer;
  class ScrollableListBox;
  class SearchBox;
  class SecurityBox;
  class SecurityDialog;
  class SecurityListItem;
  class SecurityView;
  template<typename T> class SingleSelectionModel;
  class Slider;
  class Slider2D;
  class SortedTableModel;
  template<typename T> class StagingValueModel;
  class StandardTableFilter;
  class SplitView;
  class SubmenuItem;
  class TabView;
  class TableBody;
  class TableCurrentController;
  class TableHeader;
  class TableHeaderItem;
  class TableFilter;
  class TableItem;
  class TableSelectionModel;
  class TableView;
  class Tag;
  class TagBox;
  template<typename T> class TagComboBox;
  class TextAreaBox;
  class TextBox;
  class TimeBox;
  class TitleBar;
  class ToggleButton;
  class Tooltip;
  class TransitionView;
  class TransitionWidget;
  class Window;
  class WindowObserver;

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
   * Find the focus proxy that is at the bottom of the proxy chain of a widget.
   * @param widget The widget with a focus proxy.
   * @returns The focus proxy widget.
   */
  QWidget* find_focus_proxy(QWidget& widget);

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
   * Invalidates the layout of all descendants of the widget.
   * @param widget The widget whose descendants' layout will be invalidated.
   */
  void invalidate_descendant_layouts(QWidget& widget);
}

#endif
