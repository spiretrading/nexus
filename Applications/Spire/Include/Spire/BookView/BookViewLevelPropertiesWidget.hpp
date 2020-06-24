#ifndef BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "spire/Spire/Spire.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays/modifies the properties used to display a book view's price
  //! levels.
  class BookViewLevelPropertiesWidget : public QWidget {
    public:

      //! Signals that the recent colors have changed.
      /*!
        \param recent_colors The updated recent colors.
      */
      using RecentColorsSignal =
        Signal<void (const RecentColors& recent_colors)>;

      //! Constructs a BookViewLevelPropertiesWidget.
      /*!
        \param properties The properties to display.
        \param recent_colors The recent colors to display.
        \param parent The parent widget.
      */
      BookViewLevelPropertiesWidget(
        const BookViewProperties& properties,
        const RecentColors& recent_colors, QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(BookViewProperties& properties) const;

      //! Sets the current recent colors.
      /*!
        \param recent_colors The current recent colors.
      */
      void set_recent_colors(const RecentColors& recent_colors);

      //! Connects a slot to the recent colors signal.
      boost::signals2::connection connect_recent_colors_signal(
        const RecentColorsSignal::slot_type& slot) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      mutable RecentColorsSignal m_recent_colors_signal;
      QListWidget* m_band_list_widget;
      QString m_band_list_stylesheet;
      ColorSelectorButton* m_band_color_button;
      ColorSelectorButton* m_gradient_start_button;
      ColorSelectorButton* m_gradient_end_button;
      CheckBox* m_show_grid_lines_check_box;

      void update_band_list_font(const QFont& font);
      void update_band_list_gradient();
      void update_band_list_stylesheet(int index);
      void on_band_color_selected(const QColor& color);
      void on_change_font_button_clicked();
      void on_gradient_apply_button_clicked();
      void on_number_of_bands_spin_box_changed(int value);
      void on_recent_colors_changed(const RecentColors& recent_colors);
  };
}

#endif
