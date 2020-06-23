#ifndef BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays/modifies the properties used to highlight a book view's quotes.
  class BookViewHighlightPropertiesWidget : public QWidget {
    public:

      //! Signals that the recent colors have changed.
      /*!
        \param recent_colors The updated recent colors.
      */
      using RecentColorsSignal =
        Signal<void (const RecentColors& recent_colors)>;

      //! Constructs a BookViewHighlightPropertiesWidget.
      /*!
        \param properties The properties to display.
        \param recent_colors The recent colors to display.
        \param parent The parent widget.
      */
      explicit BookViewHighlightPropertiesWidget(
        const BookViewProperties& properties,
        const RecentColors& recent_colors, QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(BookViewProperties& properties) const;

      //! Connects a slot to the recent colors signal.
      boost::signals2::connection connect_recent_colors_signal(
        const RecentColorsSignal::slot_type& slot) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      mutable RecentColorsSignal m_recent_colors_signal;
      QListWidget* m_markets_list_widget;
      CheckBox* m_highlight_none_check_box;
      CheckBox* m_highlight_top_level_check_box;
      CheckBox* m_highlight_all_levels_check_box;
      ColorSelectorButton* m_market_highlight_color_button;
      CheckBox* m_hide_orders_check_box;
      CheckBox* m_display_orders_check_box;
      CheckBox* m_highlight_orders_check_box;
      ColorSelectorButton* m_order_highlight_color_button;

      void update_market_widgets();
      void update_market_list_stylesheet(int selected_item_index);
      void on_market_highlight_color_selected(const QColor& color);
      void on_highlight_none_check_box_checked(int state);
      void on_highlight_top_level_check_box_checked(int state);
      void on_highlight_all_levels_check_box_checked(int state);
      void on_recent_colors_changed(const RecentColors& recent_colors);
  };
}

#endif
