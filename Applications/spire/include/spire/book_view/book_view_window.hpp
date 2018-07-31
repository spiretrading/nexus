#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <boost/optional.hpp>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/book_view/book_view_properties.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a book view window.
  class book_view_window : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using change_security_signal = signal<void (const Nexus::Security& s)>;

      //! Signals that the window closed.
      using closed_signal = signal<void ()>;

      //! Constructs a book view window.
      /*!
        \param properties The display properties.
        \param input_model The security_input_model to use for autocomplete.
        \param parent The parent widget.
      */
      book_view_window(const book_view_properties& properties,
        security_input_model& input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<book_view_model> model);

      //! Returns the display properties.
      const book_view_properties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const book_view_properties& properties);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const change_security_signal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable change_security_signal m_change_security_signal;
      mutable closed_signal m_closed_signal;
      book_view_properties m_properties;
      security_input_model* m_input_model;
      security_stack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_body;
      QVBoxLayout* m_layout;
      technicals_panel* m_header_widget;
      std::unique_ptr<QWidget> m_overlay_widget;
      std::unique_ptr<QLabel> m_empty_window_label;

      void set_current(const Nexus::Security& s);
      void show_context_menu(const QPoint& pos);
      void show_overlay_widget();
      void show_properties_dialog();
      void on_security_input_accept(security_input_dialog* dialog);
      void on_security_input_reject(security_input_dialog* dialog);
  };
}

#endif
