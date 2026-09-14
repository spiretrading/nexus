#ifndef NEXUS_TEST_CXA_PITCH_GAP_CLIENT_HPP
#define NEXUS_TEST_CXA_PITCH_GAP_CLIENT_HPP
#include <variant>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"

namespace Nexus::Tests {

  /** Records gap-client operations and lets tests supply their results. */
  class TestCxaPitchGapClient {
    public:

      /** Records a call to request(). */
      struct RequestOperation {
        std::uint8_t m_unit;
        CxaPitchGap m_gap;
        std::uint32_t m_live;
        Beam::Tests::ServiceResult<std::uint32_t> m_result;
      };

      /** Records a call to is_recoverable(). */
      struct IsRecoverableOperation {
        CxaPitchGap m_gap;
        std::uint32_t m_live;
        Beam::Tests::ServiceResult<bool> m_result;
      };

      /** The operations performed by the client. */
      using Operation = std::variant<RequestOperation, IsRecoverableOperation>;

      /** The queue receiving recorded operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestCxaPitchGapClient.
       * @param operations The queue receiving operations.
       */
      explicit TestCxaPitchGapClient(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations);

      ~TestCxaPitchGapClient();

      std::uint32_t request(
        std::uint8_t unit, const CxaPitchGap& gap, std::uint32_t live);
      bool is_recoverable(const CxaPitchGap& gap, std::uint32_t live) const;
      const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
        get_responses() const;
      void close();

    private:
      mutable Beam::Tests::TestServiceClientOperationQueue<Operation>
        m_operations;
      std::shared_ptr<Beam::Queue<CxaPitchGapResponse>> m_responses;

      TestCxaPitchGapClient(const TestCxaPitchGapClient&) = delete;
      TestCxaPitchGapClient& operator =(const TestCxaPitchGapClient&) = delete;
  };

  inline TestCxaPitchGapClient::TestCxaPitchGapClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
    : m_operations(std::move(operations)),
      m_responses(std::make_shared<Beam::Queue<CxaPitchGapResponse>>()) {}

  inline TestCxaPitchGapClient::~TestCxaPitchGapClient() {
    close();
  }

  inline std::uint32_t TestCxaPitchGapClient::request(
      std::uint8_t unit, const CxaPitchGap& gap, std::uint32_t live) {
    return m_operations.append_result<RequestOperation, std::uint32_t>(
      unit, gap, live);
  }

  inline bool TestCxaPitchGapClient::is_recoverable(
      const CxaPitchGap& gap, std::uint32_t live) const {
    return m_operations.append_result<IsRecoverableOperation, bool>(gap, live);
  }

  inline const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
      TestCxaPitchGapClient::get_responses() const {
    return m_responses;
  }

  inline void TestCxaPitchGapClient::close() {
    m_operations.close();
    m_responses->close();
  }
}

#endif
