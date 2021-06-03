#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#include <QDialog>
#include <QListWidget>
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/Dialog.hpp"
#include "Spire/Ui/FontSelectorWidget.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the properties for a time and sales window. */
  class TimeAndSalesPropertiesDialog : public Dialog {
    public:

      /** Signals that properties should be applied to the parent window. */
      using ApplySignal = Signal<void ()>;

      /** Signals that properties should be applied to all windows. */
      using ApplyAllSignal = Signal<void ()>;

      /** Signals that properties should be saved as the default. */
      using SaveDefaultSignal = Signal<void ()>;

      /**
       * Constructs a time and sales properties dialog.
       * @param properties The properties to modify.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesPropertiesDialog(
        const TimeAndSalesProperties& properties, QWidget* parent = nullptr);

      /** Returns the properties represented by this dialog. */
      TimeAndSalesProperties get_properties() const;

      /** Connects a slot to the apply signal. */
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

      /** Connects a slot to the apply all signal. */
      boost::signals2::connection connect_apply_all_signal(
        const ApplyAllSignal::slot_type& slot) const;

      /** Connects a slot to the save default signal. */
      boost::signals2::connection connect_save_default_signal(
        const SaveDefaultSignal::slot_type& slot) const;

    private:
      mutable ApplySignal m_apply_signal;
      mutable ApplyAllSignal m_apply_all_signal;
      mutable SaveDefaultSignal m_save_default_signal;
      TimeAndSalesProperties m_properties;
      QListWidget* m_band_list;
      ColorSelectorButton* m_band_color_button;
      ColorSelectorButton* m_text_color_button;
      FontSelectorWidget* m_edit_font_widget;
      CheckBox* m_show_grid_checkbox;

      void set_band_color(const QColor& color);
      void set_color_settings_stylesheet(int band_index);
      void set_properties(const TimeAndSalesProperties& properties);
      void set_text_color(const QColor& color);
      void update_band_list_stylesheet(int highlighted_band_index);
      void on_font_modified(const QFont& font);
      void on_font_preview(const QFont& font);
  };
}

#endif
