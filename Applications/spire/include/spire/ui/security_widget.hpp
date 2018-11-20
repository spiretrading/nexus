#ifndef SPIRE_SECURITY_WIDGET_HPP
#define SPIRE_SECURITY_WIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  /** Encapsulates the common functionality used to display a widget
      representing a security. */
  class SecurityWidget : public QWidget {
    public:

      /** The theme used to display the loading UI. */
      enum class Theme {

        /** Light colored theme. */
        LIGHT,

        /** Dark/inverted color theme. */
        DARK
      };

      //! Signals a request to change the displayed security.
      /*!
        \param security The security to display.
      */
      using ChangeSecuritySignal =
        Signal<void (const Nexus::Security& security)>;

      //! Constructs a SecurityWidget.
      /*!
        \param input_model The SecurityInputModel to use for autocomplete.
        \param theme The widget's theme.
        \param parent The parent widget.
      */
      explicit SecurityWidget(Beam::Ref<SecurityInputModel> input_model,
        Theme theme, QWidget* parent = nullptr);

      //! Sets the widget to display and indicates that loading has completed.
      void set_widget(QWidget* widget);

      //! Displays the overlay widget that covers the specified widget.
      /*
        \param widget The widget to cover.
      */
      void show_overlay_widget(QWidget* widget);

      //! Hides the overlay widget.
      void hide_overlay_widget();

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

    private:
      mutable ChangeSecuritySignal m_security_change_signal;
      SecurityInputModel* m_input_model;
      SecurityStack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_widget;
      QVBoxLayout* m_layout;
      std::unique_ptr<QLabel> m_empty_window_label;
      std::unique_ptr<QLabel> m_overlay_widget;

      void on_security_input_accept(SecurityInputDialog* dialog);
      void on_security_input_reject(SecurityInputDialog* dialog);
  };
}

#endif
