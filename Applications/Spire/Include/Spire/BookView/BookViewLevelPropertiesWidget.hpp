#ifndef BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays/modifies the properties used to display a book view's price
  //! levels.
  class BookViewLevelPropertiesWidget : public QWidget {
    public:

      //! Constructs a BookViewLevelPropertiesWidget.
      /*!
        \param properties The properties to display.
        \param parent The parent widget.
      */
      explicit BookViewLevelPropertiesWidget(
        const BookViewProperties& properties, QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(BookViewProperties& properties) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      QListWidget* m_band_list_widget;
      QString m_band_list_stylesheet;
      ColorSelectorButton* m_band_color_button;
      ColorSelectorButton* m_gradient_start_button;
      ColorSelectorButton* m_gradient_end_button;
      Checkbox* m_show_grid_lines_checkbox;

      void update_band_list_font(const QFont& font);
      void update_band_list_gradient();
      void update_band_list_height();
      void update_band_list_stylesheet(int index);
      void on_band_color_selected(const QColor& color);
      void on_gradient_apply_button_clicked();
      void on_number_of_bands_spin_box_changed(int value);
  };
}

#endif
