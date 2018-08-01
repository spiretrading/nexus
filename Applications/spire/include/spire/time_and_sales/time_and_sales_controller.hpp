#ifndef SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#define SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"

namespace spire {

  //! Manages a time and sales window.
  class time_and_sales_controller : private boost::noncopyable {
    public:

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a time and sales controller.
      /*!
        \param service_clients The service clients logged into Spire.
      */
      time_and_sales_controller(Nexus::VirtualServiceClients& service_clients);

      ~time_and_sales_controller();

      //! Displays the time and sales window.
      void open();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      mutable ClosedSignal m_closed_signal;
      Nexus::VirtualServiceClients* m_service_clients;
      std::unique_ptr<security_input_model> m_input_model;
      std::unique_ptr<time_and_sales_window> m_window;

      void on_change_security(const Nexus::Security& s);
      void on_closed();
  };
}

#endif
