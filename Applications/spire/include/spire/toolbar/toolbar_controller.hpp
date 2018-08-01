#ifndef SPIRE_TOOLBAR_CONTROLLER_HPP
#define SPIRE_TOOLBAR_CONTROLLER_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ServiceClients.hpp"
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! Launches the main application windows.
  class toolbar_controller : private boost::noncopyable {
    public:

      //! Signals that this toolbar and all its windows have closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs the toolbar controller.
      /*!
        \param service_clients The service clients logged into Spire.
      */
      toolbar_controller(Nexus::VirtualServiceClients& service_clients);

      ~toolbar_controller();

      //! Begins displaying the toolbar window.
      void open();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      struct base_controller;
      template<typename> struct controller;
      mutable ClosedSignal m_closed_signal;
      Nexus::VirtualServiceClients* m_service_clients;
      recently_closed_model m_model;
      std::unique_ptr<toolbar_window> m_toolbar_window;
      std::vector<std::unique_ptr<base_controller>> m_controllers;

      void on_open_window(recently_closed_model::type w);
      void on_closed();
  };
}

#endif
