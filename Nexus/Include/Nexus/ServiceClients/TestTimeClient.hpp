#ifndef NEXUS_TESTTIMECLIENT_HPP
#define NEXUS_TESTTIMECLIENT_HPP
#include <boost/noncopyable.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class TestTimeClient
      \brief A TimeClient used by the TestEnvironment.
   */
  class TestTimeClient : private boost::noncopyable {
    public:

      //! Constructs a TestTimeClient.
      /*!
        \param environment The TestEnvironment this client belongs to.
      */
      TestTimeClient(Beam::RefType<TestEnvironment> environment);

      ~TestTimeClient();

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      friend class TestEnvironment;
      TestEnvironment* m_environment;
      Beam::TimeService::FixedTimeClient m_timeClient;

      void SetTime(boost::posix_time::ptime time);
  };

  inline TestTimeClient::TestTimeClient(
      Beam::RefType<TestEnvironment> environment)
      : m_environment{environment.Get()} {
    m_environment->Add(this);
  }

  inline TestTimeClient::~TestTimeClient() {
    m_environment->Remove(this);
  }

  inline boost::posix_time::ptime TestTimeClient::GetTime() {
    return m_timeClient.GetTime();
  }

  inline void TestTimeClient::Open() {
    m_timeClient.Open();
  }

  inline void TestTimeClient::Close() {
    m_timeClient.Close();
  }

  inline void TestTimeClient::SetTime(boost::posix_time::ptime time) {
    m_timeClient.SetTime(time);
  }

  inline void TestEnvironment::Add(TestTimeClient* timeClient) {
    m_timeClients.With(
      [&] (auto& timeClients) {
        timeClients.push_back(timeClient);
        if(m_currentTime != boost::posix_time::not_a_date_time) {
          timeClient->SetTime(m_currentTime);
        }
      });
  }
}

#endif
