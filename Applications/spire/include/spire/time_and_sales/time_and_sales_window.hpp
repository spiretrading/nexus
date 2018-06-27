#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <memory>
#include <boost/optional.hpp>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a time and sales window.
  class time_and_sales_window : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using change_security_signal = signal<void (const Nexus::Security& s)>;

      //! Signals that the window closed.
      using closed_signal = signal<void ()>;

      //! Constructs a time and sales window.
      /*!
        \param properties The display properties.
        \param input_model The security_input_model to use for autocomplete.
        \param parent The parent widget.
      */
      time_and_sales_window(const time_and_sales_properties& properties,
        security_input_model& input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<time_and_sales_model> model);

      //! Returns the display properties.
      const time_and_sales_properties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const time_and_sales_properties& properties);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const change_security_signal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      void contextMenuEvent(QContextMenuEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable change_security_signal m_change_security_signal;
      mutable closed_signal m_closed_signal;
      time_and_sales_properties m_properties;
      security_input_model* m_input_model;
      boost::optional<time_and_sales_window_model> m_model;
      security_stack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_body;
      QLabel* m_empty_window_label;
      std::unique_ptr<QLabel> m_overlay_widget;
      time_and_sales_table_view* m_table;
      QLabel* m_volume_label;

      void export_table();
      void show_transition_widget();
      void show_overlay_widget();
      void show_properties_dialog();
      void set_current(const Nexus::Security& s);
      void on_security_input_accept(security_input_dialog* dialog);
      void on_security_input_reject(security_input_dialog* dialog);
      void on_volume(const Nexus::Quantity& volume);
  };
}

#endif
