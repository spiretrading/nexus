#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/BookView/BboQuotePanel.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/SecurityStack.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays a book view window.
  class BookViewWindow : public Window {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param security The security to display.
      */
      using ChangeSecuritySignal =
        Signal<void (const Nexus::Security& security)>;

      //! Signals that the recent colors have changed.
      /*!
        \param recent_colors The updated recent colors.
      */
      using RecentColorsSignal =
        Signal<void (const RecentColors& recent_colors)>;

      //! Constructs a book view window.
      /*!
        \param properties The display properties.
        \param recent_colors The recent colors for the properties dialog.
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      BookViewWindow(const BookViewProperties& properties,
        const RecentColors& recent_colors,
        Beam::Ref<SecurityInputModel> input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<BookViewModel> model);

      //! Returns the display properties.
      const BookViewProperties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const BookViewProperties& properties);

      //! Connects a slot to the recent colors signal.
      boost::signals2::connection connect_recent_colors_signal(
        const RecentColorsSignal::slot_type& slot) const;

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

    protected:
      void contextMenuEvent(QContextMenuEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable RecentColorsSignal m_recent_colors_signal;
      BookViewProperties m_properties;
      RecentColors m_recent_colors;
      SecurityInputModel* m_input_model;
      std::shared_ptr<BookViewModel> m_model;
      SecurityWidget* m_security_widget;
      QVBoxLayout* m_layout;
      TechnicalsPanel* m_technicals_panel;
      BboQuotePanel* m_bbo_quote_panel;
      TransitionWidget* m_transition_widget;
      QWidget* m_quote_widgets_container;
      QVBoxLayout* m_quote_widgets_container_layout;
      BookViewTableWidget* m_table;
      QtPromise<void> m_data_loaded_promise;
      bool m_is_data_loaded;
      boost::signals2::scoped_connection m_dialog_apply_connection;
      boost::signals2::scoped_connection m_dialog_recent_colors_connection;

      void show_context_menu(const QPoint& pos);
      void show_properties_dialog();
      void on_data_loaded(Beam::Expect<void> value);
  };
}

#endif
