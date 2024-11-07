#ifndef SPIRE_QUERY_MODEL_HPP
#define SPIRE_QUERY_MODEL_HPP
#include <any>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QString>
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Base class for the QueryModel. */
  class AnyQueryModel {
    public:
      virtual ~AnyQueryModel() = default;

      /**
       * Parses a value from a query string.
       * @param query The query string to parse.
       * @return The value represented by the <i>query</i> or an empty value iff
       *         the <i>query</i> does not represent a valid value.
       */
      std::any parse(const QString& query);

      /**
       * Submits a query to be asynchronously resolved.
       * @param query The query to submit.
       * @return An asynchronous list of matches to the given <i>query</i>.
       */
      QtPromise<std::vector<std::any>> submit(const QString& query);

    protected:

      /** Constructs an empty AnyQueryModel. */
      AnyQueryModel() = default;
      virtual std::any parse_any(const QString& query) = 0;
      virtual QtPromise<std::vector<std::any>>
        submit_any(const QString& query) = 0;

    private:
      AnyQueryModel(const AnyQueryModel&) = delete;
      AnyQueryModel& operator =(const AnyQueryModel&) = delete;
  };

  /**
   * Used to retreive potential matches to a query.
   * @param <T> The type of value being queried.
   */
  template<typename T>
  class QueryModel : public AnyQueryModel {
    public:

      /** The type of value being queried. */
      using Type = T;

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

      std::any parse_any(const QString& query) override;
      QtPromise<std::vector<std::any>>
        submit_any(const QString& query) override;
  };

  template<typename T>
  std::any QueryModel<T>::parse_any(const QString& query) {
    if(auto result = parse(query)) {
      return *result;
    }
    return {};
  }

  template<typename T>
  QtPromise<std::vector<std::any>>
      QueryModel<T>::submit_any(const QString& query) {
    return submit(query).then([] (std::vector<Type> matches) {
      return std::vector<std::any>(std::make_move_iterator(matches.begin()),
        std::make_move_iterator(matches.end()));
    });
  }
}

#endif
