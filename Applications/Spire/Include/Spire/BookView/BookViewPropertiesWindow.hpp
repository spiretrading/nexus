#ifndef BOOK_VIEW_PROPERTIES_WINDOW_HPP
#define BOOK_VIEW_PROPERTIES_WINDOW_HPP
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the properties for the book view window. */
  class BookViewPropertiesWindow : public Window {
    public:

      /** Signals that the user submitted the current edits. */
      using SubmitSignal = Signal<void ()>;

      /** Signals that the user discarded the current edits. */
      using CancelSignal = Signal<void ()>;

      /**
       * Constructs a BookViewPropertiesWindow.
       * @param properties The initial properties.
       * @param key_bindings The user's key bindings.
       * @param ticker The ticker whose interactions are to be displayed.
       * @param parent The parent widget.
       */
      BookViewPropertiesWindow(
        std::shared_ptr<BookViewPropertiesModel> properties,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<TickerModel> ticker, QWidget* parent = nullptr);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable CancelSignal m_cancel_signal;
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<TickerModel> m_ticker;
      bool m_is_submitted;
      NavigationView* m_navigation_view;
      QWidget* m_highlights_page;
      boost::signals2::scoped_connection m_level_connection;
      boost::signals2::scoped_connection m_ticker_connection;

      void on_cancel_button_click();
      void on_done_button_click();
      void on_level_update(const BookViewLevelProperties& properties);
      void on_ticker_update(const Nexus::Ticker& ticker);
  };
}

#endif
