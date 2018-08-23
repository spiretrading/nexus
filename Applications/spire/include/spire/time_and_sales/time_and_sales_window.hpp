#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <boost/optional.hpp>
#include <QLabel>
#include <QMenu>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a time and sales window.
  class TimeAndSalesWindow : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using ChangeSecuritySignal = Signal<void (const Nexus::Security& s)>;

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a time and sales window.
      /*!
        \param properties The display properties.
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      TimeAndSalesWindow(const TimeAndSalesProperties& properties,
        SecurityInputModel& input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<TimeAndSalesModel> model);

      //! Returns the display properties.
      const TimeAndSalesProperties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const TimeAndSalesProperties& properties);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      void contextMenuEvent(QContextMenuEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable ChangeSecuritySignal m_change_security_signal;
      mutable ClosedSignal m_closed_signal;
      TimeAndSalesProperties m_properties;
      SecurityInputModel* m_input_model;
      boost::optional<TimeAndSalesWindowModel> m_model;
      SecurityStack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_body;
      std::unique_ptr<QLabel> m_empty_window_label;
      std::unique_ptr<QLabel> m_overlay_widget;
      TimeAndSalesTableView* m_table;
      QLabel* m_volume_label;
      CustomVariantItemDelegate* m_item_delegate;

      void export_table();
      void show_overlay_widget();
      void show_properties_dialog();
      void set_current(const Nexus::Security& s);
      void on_security_input_accept(SecurityInputDialog* dialog);
      void on_security_input_reject(SecurityInputDialog* dialog);
      void on_volume(const Nexus::Quantity& volume);
  };
}

#endif
