#ifndef SPIRE_CHARTING_WINDOW_HPP
#define SPIRE_CHARTING_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/charting/charting.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a financial chart.
  class ChartingWindow : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param security The security to display.
      */
      using ChangeSecuritySignal =
        Signal<void (const Nexus::Security& security)>;

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a charting window.
      /*!
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      ChartingWindow(Beam::Ref<SecurityInputModel> input_model,
        QWidget* parent = nullptr);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable ChangeSecuritySignal m_change_security_signal;
      mutable ClosedSignal m_closed_signal;
      SecurityInputModel* m_input_model;
      SecurityStack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_body;
      QWidget* m_button_header_widget;
      QLineEdit* m_period_line_edit;
      DropdownMenu* m_period_dropdown;
      std::unique_ptr<QLabel> m_empty_window_label;
      std::unique_ptr<QLabel> m_overlay_widget;

      void set_current(const Nexus::Security& s);
      void show_security_input_dialog(const QString& text);
      void show_overlay_widget();
      void on_period_line_edit_changed();
      void on_security_input_accept(SecurityInputDialog* dialog);
      void on_security_input_reject(SecurityInputDialog* dialog);
  };
}

#endif
