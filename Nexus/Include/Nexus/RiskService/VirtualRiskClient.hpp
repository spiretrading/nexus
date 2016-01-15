#ifndef NEXUS_VIRTUALRISKCLIENT_HPP
#define NEXUS_VIRTUALRISKCLIENT_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus {
namespace RiskService {

  /*! \class VirtualRiskClient
      \brief Provides a pure virtual interface to an RiskClient.
   */
  class VirtualRiskClient : private boost::noncopyable {
    public:
      virtual ~VirtualRiskClient();

      virtual const RiskPortfolioUpdatePublisher&
        GetRiskPortfolioUpdatePublisher() = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualRiskClient.
      VirtualRiskClient();
  };

  /*! \class WrapperRiskClient
      \brief Wraps a RiskClient providing it with a virtual interface.
      \tparam ClientType The type of RiskClient to wrap.
   */
  template<typename ClientType>
  class WrapperRiskClient : public VirtualRiskClient {
    public:

      //! The RiskClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperRiskClient.
      /*!
        \param client The RiskClient to wrap.
      */
      template<typename RiskClientForward>
      WrapperRiskClient(RiskClientForward&& client);

      virtual ~WrapperRiskClient();

      virtual const RiskPortfolioUpdatePublisher&
        GetRiskPortfolioUpdatePublisher();

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a RiskClient into a VirtualRiskClient.
  /*!
    \param client The client to wrap.
  */
  template<typename RiskClient>
  std::unique_ptr<VirtualRiskClient> MakeVirtualRiskClient(
      RiskClient&& client) {
    return std::make_unique<WrapperRiskClient<RiskClient>>(
      std::forward<RiskClient>(client));
  }

  inline VirtualRiskClient::~VirtualRiskClient() {}

  inline VirtualRiskClient::VirtualRiskClient() {}

  template<typename ClientType>
  template<typename RiskClientForward>
  WrapperRiskClient<ClientType>::WrapperRiskClient(RiskClientForward&& client)
      : m_client(std::forward<RiskClientForward>(client)) {}

  template<typename ClientType>
  WrapperRiskClient<ClientType>::~WrapperRiskClient() {}

  template<typename ClientType>
  const RiskPortfolioUpdatePublisher& WrapperRiskClient<ClientType>::
      GetRiskPortfolioUpdatePublisher() {
    return m_client->GetRiskPortfolioUpdatePublisher();
  }

  template<typename ClientType>
  void WrapperRiskClient<ClientType>::Open() {
    return m_client->Open();
  }

  template<typename ClientType>
  void WrapperRiskClient<ClientType>::Close() {
    return m_client->Close();
  }
}
}

#endif
