#ifndef NEXUS_TELEMETRY_CLIENT_HPP
#define NEXUS_TELEMETRY_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include "Nexus/TelemetryService/TelemetryService.hpp"
#include "Nexus/TelemetryService/TelemetryServices.hpp"

namespace Nexus::TelemetryService {

  /**
   * Client used to access the telemetry services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class TelemetryClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a TelemetryClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit TelemetryClient(BF&& clientBuilder);

      ~TelemetryClient();

      void Close();

    private:
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;

      TelemetryClient(const TelemetryClient&) = delete;
      TelemetryClient& operator =(const TelemetryClient&) = delete;
  };

  template<typename B>
  template<typename BF>
  TelemetryClient<B>::TelemetryClient(BF&& clientBuilder)
      try : m_clientHandler(std::forward<BF>(clientBuilder)) {
    RegisterTelemetryServices(Beam::Store(m_clientHandler.GetSlots()));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the telemetry server."));
  }

  template<typename B>
  TelemetryClient<B>::~TelemetryClient() {
    Close();
  }

  template<typename B>
  void TelemetryClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_openState.Close();
  }
}

#endif
