#ifndef NEXUS_MARKETDATAMYSQLHISTORICALDATASTORE_HPP
#define NEXUS_MARKETDATAMYSQLHISTORICALDATASTORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/MySql/DatabaseConnectionPool.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreException.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MySqlHistoricalDataStoreDetails.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MySqlHistoricalDataStore
      \brief Stores historical market data in a MySQL database.
   */
  class MySqlHistoricalDataStore : private boost::noncopyable {
    public:

      //! Constructs a MySqlHistoricalDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlHistoricalDataStore(const Beam::Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

      ~MySqlHistoricalDataStore();

      MarketEntry::InitialSequences LoadInitialSequences(MarketCode market);

      SecurityEntry::InitialSequences LoadInitialSequences(
        const Security& security);

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Open();

      void Close();

    private:
      template<typename Query, typename T, typename Row>
      using DataStore = Beam::Queries::SqlDataStore<Query, T, Row,
        Queries::SqlTranslator, Details::SqlFunctor>;
      Beam::Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      Beam::MySql::DatabaseConnectionPool m_readerDatabaseConnectionPool;
      Beam::Threading::Sync<mysqlpp::Connection> m_readerDatabaseConnection;
      Beam::Threading::Sync<mysqlpp::Connection> m_writerDatabaseConnection;
      Beam::Threading::ThreadPool m_readerThreadPool;
      DataStore<MarketWideDataQuery, OrderImbalance, Details::order_imbalances>
        m_orderImbalanceDataStore;
      DataStore<SecurityMarketDataQuery, BboQuote, Details::bbo_quotes>
        m_bboQuoteDataStore;
      DataStore<SecurityMarketDataQuery, MarketQuote, Details::market_quotes>
        m_marketQuoteDataStore;
      DataStore<SecurityMarketDataQuery, BookQuote, Details::book_quotes>
        m_bookQuoteDataStore;
      DataStore<SecurityMarketDataQuery, TimeAndSale, Details::time_and_sales>
        m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OpenDatabaseConnection(mysqlpp::Connection& connection);
  };

  inline MySqlHistoricalDataStore::MySqlHistoricalDataStore(
      const Beam::Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password)
      : m_address(address),
        m_schema(schema),
        m_username(username),
        m_password(password),
        m_readerDatabaseConnection(false),
        m_writerDatabaseConnection(false),
        m_orderImbalanceDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_bboQuoteDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_marketQuoteDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_bookQuoteDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_timeAndSaleDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection),
          Beam::Ref(m_readerThreadPool)) {}

  inline MySqlHistoricalDataStore::~MySqlHistoricalDataStore() {
    Close();
  }

  inline MarketEntry::InitialSequences MySqlHistoricalDataStore::
      LoadInitialSequences(MarketCode market) {
    MarketEntry::InitialSequences initialSequences;
    initialSequences.m_nextOrderImbalanceSequence =
      m_orderImbalanceDataStore.LoadInitialSequence(market);
    return initialSequences;
  }

  inline SecurityEntry::InitialSequences MySqlHistoricalDataStore::
      LoadInitialSequences(const Security& security) {
    SecurityEntry::InitialSequences initialSequences;
    initialSequences.m_nextBboQuoteSequence =
      m_bboQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextBookQuoteSequence =
      m_bookQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextMarketQuoteSequence =
      m_marketQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextTimeAndSaleSequence =
      m_timeAndSaleDataStore.LoadInitialSequence(security);
    return initialSequences;
  }

  inline std::vector<SequencedOrderImbalance> MySqlHistoricalDataStore::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  inline std::vector<SequencedBboQuote> MySqlHistoricalDataStore::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedBookQuote> MySqlHistoricalDataStore::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedMarketQuote> MySqlHistoricalDataStore::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedTimeAndSale> MySqlHistoricalDataStore::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    return m_orderImbalanceDataStore.Store(orderImbalance);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    return m_orderImbalanceDataStore.Store(orderImbalances);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    return m_bboQuoteDataStore.Store(bboQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    return m_bboQuoteDataStore.Store(bboQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    return m_marketQuoteDataStore.Store(marketQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    return m_marketQuoteDataStore.Store(marketQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    return m_bookQuoteDataStore.Store(bookQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    return m_bookQuoteDataStore.Store(bookQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    return m_timeAndSaleDataStore.Store(timeAndSale);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    return m_timeAndSaleDataStore.Store(timeAndSales);
  }

  inline void MySqlHistoricalDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::Threading::With(m_readerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          OpenDatabaseConnection(connection);
          if(!Details::LoadTables(connection, m_schema)) {
            BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
              "Unable to load database tables."));
          }
        });
      Beam::Threading::With(m_writerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          OpenDatabaseConnection(connection);
          if(!Details::LoadTables(connection, m_schema)) {
            BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
              "Unable to load database tables."));
          }
        });
      for(std::size_t i = 0; i <= boost::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<mysqlpp::Connection>(false);
        OpenDatabaseConnection(*connection);
        m_readerDatabaseConnectionPool.Add(std::move(connection));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlHistoricalDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlHistoricalDataStore::Shutdown() {
    m_readerDatabaseConnectionPool.Close();
    Beam::Threading::With(m_writerDatabaseConnection,
      [] (mysqlpp::Connection& connection) {
        connection.disconnect();
      });
    Beam::Threading::With(m_readerDatabaseConnection,
      [] (mysqlpp::Connection& connection) {
        connection.disconnect();
      });
    m_openState.SetClosed();
  }

  inline void MySqlHistoricalDataStore::OpenDatabaseConnection(
      mysqlpp::Connection& connection) {
    bool connectionResult = connection.set_option(
      new mysqlpp::ReconnectOption(true));
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
        "Unable to set MySQL reconnect option."));
    }
    connectionResult = connection.connect(m_schema.c_str(),
      m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
      m_address.GetPort());
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(std::string(
        "Unable to connect to MySQL database - ") + connection.error()));
    }
  }
}
}

#endif
