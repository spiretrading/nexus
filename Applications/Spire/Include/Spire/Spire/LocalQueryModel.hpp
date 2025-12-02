#ifndef SPIRE_LOCAL_QUERY_MODEL_HPP
#define SPIRE_LOCAL_QUERY_MODEL_HPP
#include <tsl/htrie_map.h>
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  /**
   * Implements an in-memory QueryModel associating values with a string
   * representation.
   */
  template<typename T>
  class LocalQueryModel : public QueryModel<T> {
    public:
      using Type = typename QueryModel<T>::Type;

      /** Constructs an empty model. */
      LocalQueryModel() = default;

      /**
       * Adds a value to the model that can be queried through its string
       * representation.
       * @param value The value to add.
       */
      void add(const Type& value);

      /**
       * Adds a value to the model that can be queried by a given string.
       * @param id The string used to query the value.
       * @param value The value to add.
       */
      void add(const QString& id, const Type& value);

      boost::optional<Type> parse(const QString& query) override;
      QtPromise<std::vector<Type>> submit(const QString& query) override;

    private:
      tsl::htrie_map<char, Type> m_values;
  };

  template<typename T>
  void LocalQueryModel<T>::add(const Type& value) {
    add(to_text(value).toLower(), value);
  }

  template<typename T>
  void LocalQueryModel<T>::add(const QString& id, const Type& value) {
    m_values[id.toLower().toStdString()] = value;
  }

  template<typename T>
  boost::optional<typename LocalQueryModel<T>::Type>
      LocalQueryModel<T>::parse(const QString& query) {
    auto i = m_values.find(query.toLower().toStdString());
    if(i == m_values.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T>
  QtPromise<std::vector<typename LocalQueryModel<T>::Type>>
      LocalQueryModel<T>::submit(const QString& query) {
    auto matches = std::vector<Type>();
    auto range = m_values.equal_prefix_range(query.toLower().toStdString());
    for(auto i = range.first; i != range.second; ++i) {
      if constexpr(requires(const Type& a, const Type& b) { a == b; }) {
        auto j = std::find(matches.begin(), matches.end(), *i);
        if(j == matches.end()) {
          matches.push_back(*i);
        }
      } else {
        matches.push_back(*i);
      }
    }
    return QtPromise(std::move(matches));
  }
}

#endif
