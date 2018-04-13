#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the properties for a time and sales window.
  class time_and_sales_properties_dialog : public QDialog {
    public:

      //! Signals that properties should be applied to the parent window.
      using apply_signal = signal<
        void (const time_and_sales_properties& properties)>;

      //! Signals that properties should be applied to all windows.
      using apply_all_signal = signal<
        void (const time_and_sales_properties& properties)>;

      //! Signals that properties should be saved as the default.
      using save_default_signal = signal<
        void (const time_and_sales_properties& properties)>;

      //! Constructs a time and sales properties dialog.
      /*!
        \param properties The properties to modify.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      time_and_sales_properties_dialog(
        const time_and_sales_properties& properties, QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      //! Returns the properties represented by this dialog.
      const time_and_sales_properties& get_properties() const;

      //! Connects a slot to the apply signal.
      boost::signals2::connection connect_apply_signal(
        const apply_signal::slot_type& slot) const;

      //! Connects a slot to the apply all signal.
      boost::signals2::connection connect_apply_all_signal(
        const apply_all_signal::slot_type& slot) const;

      //! Connects a slot to the save default signal.
      boost::signals2::connection connect_save_default_signal(
        const save_default_signal::slot_type& slot) const;

    protected:

    private:
      mutable apply_signal m_apply_signal;
      mutable apply_all_signal m_apply_all_signal;
      mutable save_default_signal m_save_default_signal;
      time_and_sales_properties m_properties;
      QWidget* m_body;
      QListWidget* m_band_list;
      flat_button* m_band_color_button;
      flat_button* m_text_color_button;
      QLabel* m_font_preview_label;
      check_box* m_show_grid_check_box;
      check_box* m_time_check_box;
      check_box* m_price_check_box;
      check_box* m_market_check_box;
      check_box* m_size_check_box;
      check_box* m_condition_check_box;

      void set_band_color();
      void set_color_button_stylesheet(flat_button* button,
        const QColor& color);
      void set_color_settings_stylesheet(int band_index);
      void set_font();
      void set_properties(const time_and_sales_properties& properties);
      void set_text_color();
  };
}

#endif
