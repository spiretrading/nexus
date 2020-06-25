#ifndef SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#define SPIRE_TIME_AND_SALES_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/Definitions.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"

namespace Spire {

  //! Manages a time and sales window.
  class TimeAndSalesController : private boost::noncopyable {
    public:

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a time and sales controller.
      /*!
        \param definitions The set of definitions to use.
        \param security_input_model The model used to autocomplete securities.
        \param service_clients The service clients logged into Spire.
      */
      TimeAndSalesController(Definitions definitions,
        Beam::Ref<SecurityInputModel> security_input_model,
        Beam::Ref<Nexus::VirtualServiceClients> service_clients);

      ~TimeAndSalesController();

      //! Displays the time and sales window.
      void open();

      //! Closes the time and sales window.
      void close();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      mutable ClosedSignal m_closed_signal;
      Definitions m_definitions;
      SecurityInputModel* m_security_input_model;
      Nexus::VirtualServiceClients* m_service_clients;
      TimeAndSalesWindow* m_window;

      void on_change_security(const Nexus::Security& security);
      void on_closed();
  };
}

#endif
