#ifndef BACKTESTER_CUTOFF_HISTORICAL_DATA_STORE_HPP
#define BACKTESTER_CUTOFF_HISTORICAL_DATA_STORE_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus {

  /**
   * Wraps a historical market data store and satisfies queries submitted to it
   * up to a certain date time.
   * @param <D> The underlying data store to wrap.
   */
  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class CutoffHistoricalDataStore {
    public:

      /** The type of underlying data store to wrap. */
      using HistoricalDataStore = Beam::dereference_t<D>;

      /**
       * Constructs a CutoffHistoricalDataStore.
       * @param data_store Initializes the data store to wrap.
       * @param cutoff The date/time to satisfied queries to.
       */
      template<Beam::Initializes<D> DF>
      CutoffHistoricalDataStore(
        DF&& data_store, boost::posix_time::ptime cutoff);

      ~CutoffHistoricalDataStore();

      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);
      void store(const SecurityInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBboQuote& quote);
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBookQuote& quote);
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityTimeAndSale& time_and_sale);
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);
      void close();

    private:
      mutable Beam::Mutex m_mutex;
      Beam::local_ptr_t<D> m_data_store;
      boost::posix_time::ptime m_cutoff;
      std::unordered_map<Venue, Beam::Sequence> m_order_imbalance_cutoffs;
      std::unordered_map<Security, Beam::Sequence> m_bbo_cutoffs;
      std::unordered_map<Security, Beam::Sequence> m_book_quote_cutoffs;
      std::unordered_map<Security, Beam::Sequence> m_time_and_sales_cutoffs;
      Beam::OpenState m_open_state;

      CutoffHistoricalDataStore(const CutoffHistoricalDataStore&) = delete;
      CutoffHistoricalDataStore& operator =(
        const CutoffHistoricalDataStore&) = delete;
      template<typename Query, typename F>
      std::invoke_result_t<F, const Query&> load(const Query& query,
        std::unordered_map<typename Query::Index, Beam::Sequence>& cutoffs,
        F loader);
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  CutoffHistoricalDataStore<D>::CutoffHistoricalDataStore(
    DF&& data_store, boost::posix_time::ptime cutoff)
    : m_data_store(std::forward<DF>(data_store)),
      m_cutoff(cutoff) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  CutoffHistoricalDataStore<D>::~CutoffHistoricalDataStore() {
    close();
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SecurityInfo> CutoffHistoricalDataStore<D>::load_security_info(
      const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(const SecurityInfo& info) {
    m_data_store->store(info);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedOrderImbalance>
      CutoffHistoricalDataStore<D>::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return load(query, m_order_imbalance_cutoffs, [&] (const auto& query) {
      return m_data_store->load_order_imbalances(query);
    });
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_data_store->store(imbalance);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_data_store->store(imbalances);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBboQuote> CutoffHistoricalDataStore<D>::load_bbo_quotes(
      const SecurityMarketDataQuery& query) {
    return load(query, m_bbo_cutoffs, [&] (const auto& query) {
      return m_data_store->load_bbo_quotes(query);
    });
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBookQuote>
      CutoffHistoricalDataStore<D>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return load(query, m_book_quote_cutoffs, [&] (const auto& query) {
      return m_data_store->load_book_quotes(query);
    });
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedTimeAndSale>
      CutoffHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return load(query, m_time_and_sales_cutoffs, [&] (const auto& query) {
      return m_data_store->load_time_and_sales(query);
    });
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void CutoffHistoricalDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_open_state.close();
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<typename Query, typename F>
  std::invoke_result_t<F, const Query&> CutoffHistoricalDataStore<D>::load(
      const Query& query,
      std::unordered_map<typename Query::Index, Beam::Sequence>& cutoffs,
      F loader) {
    if(auto start_timestamp =
        boost::get<boost::posix_time::ptime>(&query.get_range().get_start())) {
      if(*start_timestamp >= m_cutoff) {
        return {};
      }
    }
    auto cutoff_sequence = [&] {
      auto lock = std::lock_guard(m_mutex);
      auto cutoff = cutoffs.find(query.get_index());
      auto range_end = Beam::Range::Point(m_cutoff);
      while(cutoff == cutoffs.end()) {
        auto cutoff_query = Query();
        cutoff_query.set_index(query.get_index());
        cutoff_query.set_range(Beam::Range(Beam::Sequence::FIRST, range_end));
        cutoff_query.set_snapshot_limit(Beam::SnapshotLimit::from_tail(100));
        auto sequences = loader(cutoff_query);
        if(sequences.empty()) {
          cutoff = cutoffs.insert(
            std::pair(query.get_index(), Beam::Sequence::FIRST)).first;
        } else {
          for(auto i = sequences.rbegin(); i != sequences.rend(); ++i) {
            if((*i)->m_timestamp < m_cutoff) {
              cutoff = cutoffs.insert(
                std::pair(query.get_index(), i->get_sequence())).first;
              break;
            }
          }
          if(cutoff != cutoffs.end()) {
            break;
          }
          if(sequences.front().get_sequence() == Beam::Sequence::FIRST) {
            cutoff = cutoffs.insert(
              std::pair(query.get_index(), Beam::Sequence::FIRST)).first;
          } else {
            range_end = Beam::decrement(sequences.front().get_sequence());
          }
        }
      }
      return cutoff->second;
    }();
    if(auto start_sequence =
        boost::get<Beam::Sequence>(&query.get_range().get_start())) {
      if(*start_sequence > cutoff_sequence) {
        return {};
      }
    }
    auto cutoff_query = query;
    if(auto end_timestamp =
        boost::get<boost::posix_time::ptime>(&query.get_range().get_end())) {
      cutoff_query.set_range(
        query.get_range().get_start(), std::min(*end_timestamp, m_cutoff));
    } else {
      cutoff_query.set_range(query.get_range().get_start(),
        std::min(boost::get<Beam::Sequence>(query.get_range().get_end()),
          cutoff_sequence));
    }
    return loader(cutoff_query);
  }
}

#endif
