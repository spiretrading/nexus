#ifndef SPIRE_UI_VIEWER_HPP
#define SPIRE_UI_VIEWER_HPP
#include <QLabel>

namespace Spire {
  class CheckBoxTestWidget;
  class ColorSelectorButtonTestWidget;
  class ComboBoxAdapter;
  class ComboBoxTestWidget;
  class CurrencyComboBoxTestWidget;
  class DateInputTestWidget;
  class DecimalSpinBoxTestWidget;
  class DurationInputTestWidget;
  class FilteredDropDownMenuTestWidget;
  class FlatButtonTestWidget;
  class FontSelectorTestWidget;
  class IconButtonTestWidget;
  class IntegerSpinBoxTestWidget;
  class KeySequenceTestWidget;
  class MarketComboBoxTestWidget;
  class MoneySpinBoxTestWidget;
  class OrderStatusComboBoxTestWidget;
  class OrderTypeComboBoxTestWidget;
  class QuantitySpinBoxTestWidget;
  class RangeInputTestWidget;
  class ScrollAreaTestWidget;
  class SecurityInputTestWidget;
  class SecurityWidgetTestWidget;
  class SideComboBoxTestWidget;
  class SpinBoxAdapter;
  class SpinBoxTestWidget;
  class StaticDropDownMenuTestWidget;
  class TabTestWidget;
  class TextInputTestWidget;
  class TimeInForceComboBoxTestWidget;
  class TimeOfDayTestWidget;
  class ToggleButtonTestWidget;
  class TransitionTestWidget;
  class UiViewerWindow;

  //! Creates a label for separating test widgets from their parameters.
  /*!
    \param parent The parent widget.
  */
  QLabel* make_parameters_label(QWidget* parent);
}

#endif
