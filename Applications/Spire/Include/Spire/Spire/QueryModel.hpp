#ifndef SPIRE_QUERY_MODEL_HPP
#define SPIRE_QUERY_MODEL_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <QString>
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class BaseQueryModel {};

  /**
   * Used to retreive potential matches to a query.
   * @param <T> The type of value being queried.
   */
  template<typename T>
  class QueryModel : public BaseQueryModel {
    public:

      /** The type of value being queried. */
      using Type = T;

      virtual ~QueryModel() = default;

      /**
       * Parses a value from a query string.
       * @param query The query string to parse.
       * @return The value represented by the <i>query</i> or <i>none</i> if the
       *         <i>query</i> does not represent a valid value.
       */
      virtual boost::optional<Type> parse(const QString& query) = 0;

      /**
        * Submits a query to be asynchronously resolved.
        * @param query The query to submit.
        * @return An asynchronous list of matches to the given <i>query</i>.
        */
      virtual QtPromise<std::vector<Type>> submit(const QString& query) = 0;

    protected:

      /** Constructs a QueryModel. */
      QueryModel() = default;

    private:
      QueryModel(const QueryModel&) = delete;
      QueryModel& operator =(const QueryModel&) = delete;
  };
}

#endif
