#ifndef NEXUS_MARKETDATAHISTORICALDATASTOREEXCEPTION_HPP
#define NEXUS_MARKETDATAHISTORICALDATASTOREEXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class HistoricalDataStoreException
      \brief Exception to indicate an operation on a HistoricalDataStore failed.
   */
  class HistoricalDataStoreException : public Beam::IO::IOException {
    public:

      //! Constructs a HistoricalDataStoreException.
      HistoricalDataStoreException();

      //! Constructs a HistoricalDataStoreException.
      /*!
        \param message A message describing the error.
      */
      HistoricalDataStoreException(const std::string& message);

      virtual ~HistoricalDataStoreException() throw();
  };

  inline HistoricalDataStoreException::HistoricalDataStoreException()
      : Beam::IO::IOException("Operation failed") {}

  inline HistoricalDataStoreException::HistoricalDataStoreException(
      const std::string& message)
      : Beam::IO::IOException(message) {}

  inline HistoricalDataStoreException::
      ~HistoricalDataStoreException() throw() {}
}
}

#endif
