#ifndef NEXUS_MARKETDATAMARKETORDERIMBALANCEEXPRESSION_HPP
#define NEXUS_MARKETDATAMARKETORDERIMBALANCEEXPRESSION_HPP
#include <deque>
#include <Beam/Expressions/ExpressionInitializingException.hpp>
#include <Beam/Expressions/ExpressionRuntime.hpp>
#include <Beam/Expressions/ExpressionUnavailableException.hpp>
#include <Beam/Expressions/IterationExpression.hpp>
#include <Beam/Expressions/SubexpressionContainer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/Range.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/Rethrow.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketOrderImbalanceExpression
      \brief Publishes market order imbalances in the form of an Expression.
      \tparam MarketDataClientType The type of MarketDataClient used to request
              the imbalance info from.
      \tparam MarketExpressionType The type of Expression evaluating to the
              market to get the imbalances for.
      \tparam TimeRangeExpressionType The type of Expression evaluating to the
              range of time to query.
   */
  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType>
  class MarketOrderImbalanceExpression :
      public Beam::Expressions::IterationExpression<OrderImbalance> {
    public:
      using Type = OrderImbalance;

      //! The type of MarketDataClient used to request market data from.
      using MarketDataClient =
        typename Beam::TryDereferenceType<MarketDataClientType>::type;

      //! The type of Expression evaluating to the market to get the imbalances
      //! for.
      using MarketExpression =
        typename Beam::TryDereferenceType<MarketExpressionType>::type;

      //! The type of Expression evaluating to the range of time to query.
      using TimeRangeExpression =
        typename Beam::TryDereferenceType<TimeRangeExpressionType>::type;

      //! Constructs a MarketOrderImbalanceExpression.
      /*!
        \param marketDataClient The type of MarketDataClient used to request
               market data from.
        \param marketExpression The Expression evaluating to the market to get
               the imbalances for.
        \param timeRangeExpression The Expression evaluating to the range of
               time to query.
        \param expressionRuntime The ExpressionRuntime to use.
      */
      template<typename MarketDataClientForward,
        typename MarketExpressionForward, typename TimeRangeExpressionForward>
      MarketOrderImbalanceExpression(MarketDataClientForward&& marketDataClient,
        MarketExpressionForward&& marketExpression,
        TimeRangeExpressionForward&& timeRangeExpression,
        Beam::Ref<Beam::Expressions::ExpressionRuntime> expressionRuntime);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      typename Beam::OptionalLocalPtr<MarketDataClientType>::type
        m_marketDataClient;
      Beam::Expressions::SubexpressionContainer<MarketExpressionType>
        m_marketExpression;
      Beam::Expressions::SubexpressionContainer<TimeRangeExpressionType>
        m_timeRangeExpression;
      Beam::Expressions::ExpressionRuntime* m_expressionRuntime;
      boost::signals2::scoped_connection m_publisherConnection;
      boost::optional<MarketCode> m_market;
      boost::optional<Beam::Queries::Range> m_timeRange;
      std::shared_ptr<Beam::SnapshotPublisher<OrderImbalance,
        std::vector<OrderImbalance>>> m_publisher;
      std::deque<OrderImbalance> m_values;
      std::exception_ptr m_exception;

      void OnOrderImbalance(const OrderImbalance& imbalance);
  };

  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType>
  template<typename MarketDataClientForward,
    typename MarketExpressionForward, typename TimeRangeExpressionForward>
  MarketOrderImbalanceExpression<MarketDataClientType, MarketExpressionType,
      TimeRangeExpressionType>::MarketOrderImbalanceExpression(
      MarketDataClientForward&& marketDataClient,
      MarketExpressionForward&& marketExpression,
      TimeRangeExpressionForward&& timeRangeExpression,
      Beam::Ref<Beam::Expressions::ExpressionRuntime> expressionRuntime)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : Beam::Expressions::IterationExpression<OrderImbalance>(
          Ref(expressionRuntime)),
        m_marketDataClient(std::forward<MarketDataClientForward>(
          marketDataClient)),
        m_marketExpression(std::forward<MarketExpressionForward>(
          marketExpression), *this),
        m_timeRangeExpression(std::forward<TimeRangeExpressionForward>(
          timeRangeExpression), *this),
        m_expressionRuntime(expressionRuntime.Get()) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType>
  void MarketOrderImbalanceExpression<MarketDataClientType,
      MarketExpressionType, TimeRangeExpressionType>::Commit() {
    m_marketExpression.Commit();
    m_timeRangeExpression.Commit();
    if(this->IsInitializing()) {
      if(!m_marketExpression.IsEvaluated() ||
          !m_timeRangeExpression.IsEvaluated()) {
        return;
      }
    }
    bool marketChanged;
    try {
      MarketCode market = m_marketExpression.Eval();
      if(m_market != market) {
        m_market = market;
        marketChanged = true;
      } else {
        marketChanged = false;
      }
    } catch(const std::exception&) {
      marketChanged = true;
      m_market.reset();
      m_exception = std::current_exception();
    }
    bool timeRangeChanged;
    try {
      auto timeRange = m_timeRangeExpression.Eval();
      if(m_timeRange != timeRange) {
        m_timeRange = timeRange;
        timeRangeChanged = true;
      } else {
        timeRangeChanged = false;
      }
    } catch(const std::exception&) {
      timeRangeChanged = true;
      m_timeRange.reset();
      m_exception = std::current_exception();
    }
    if(marketChanged || timeRangeChanged) {
      m_values.clear();
      if(m_market.is_initialized() && m_timeRange.is_initialized()) {
        m_exception = std::exception_ptr();
        try {
          boost::optional<std::vector<OrderImbalance>> snapshot;
          m_publisherConnection = m_expressionRuntime->Connect(*m_publisher,
            std::bind(&MarketOrderImbalanceExpression::OnOrderImbalance, this,
            std::placeholders::_1), Beam::Store(snapshot));
          if(snapshot.is_initialized() && !snapshot->empty()) {
            m_values.insert(m_values.end(), snapshot->begin(), snapshot->end());
            this->IncrementSequenceNumber();
          }
        } catch(const std::exception&) {
          m_exception = std::current_exception();
          this->IncrementSequenceNumber();
        }
      } else {
        if(!this->IsInitializing()) {
          m_exception = std::make_exception_ptr(
            Beam::Expressions::ExpressionUnavailableException());
          this->IncrementSequenceNumber();
        }
      }
    } else if(m_values.size() > 1) {
      m_values.pop_front();
      this->IncrementSequenceNumber();
      if(m_values.size() > 1) {
        this->Iterate();
      }
    }
  }

  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType>
  typename MarketOrderImbalanceExpression<MarketDataClientType,
      MarketExpressionType, TimeRangeExpressionType>::Type
      MarketOrderImbalanceExpression<MarketDataClientType, MarketExpressionType,
      TimeRangeExpressionType>::Eval() const {
    if(this->IsInitializing()) {
      BOOST_THROW_EXCEPTION(
        Beam::Expressions::ExpressionInitializingException());
    }
    Beam::Rethrow(m_exception);
    return m_values.front();
  }

  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType>
  void MarketOrderImbalanceExpression<MarketDataClientType,
      MarketExpressionType, TimeRangeExpressionType>::OnOrderImbalance(
      const OrderImbalance& imbalance) {
    m_values.push_back(imbalance);
    this->SignalUpdate();
  }
}
}

#endif
