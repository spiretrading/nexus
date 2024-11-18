#ifndef SPIRE_LOCAL_QUERY_MODEL_HPP
#define SPIRE_LOCAL_QUERY_MODEL_HPP
#include <Beam/Collections/Trie.hpp>
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
      LocalQueryModel();

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
      rtv::Trie<QChar, Type> m_values;
  };

  template<typename T>
  LocalQueryModel<T>::LocalQueryModel()
    : m_values(QChar()) {}

  template<typename T>
  void LocalQueryModel<T>::add(const Type& value) {
    add(to_text(value).toLower(), value);
  }

  template<typename T>
  void LocalQueryModel<T>::add(const QString& id, const Type& value) {
    m_values[id.toLower().data()] = value;
  }

  template<typename T>
  boost::optional<typename LocalQueryModel<T>::Type>
      LocalQueryModel<T>::parse(const QString& query) {
    auto i = m_values.find(query.toLower().data());
    if(i == m_values.end()) {
      return boost::none;
    }
    return *i->second;
  }

  template<typename T>
  QtPromise<std::vector<typename LocalQueryModel<T>::Type>>
      LocalQueryModel<T>::submit(const QString& query) {
    auto matches = std::vector<Type>();
    for(auto i = m_values.startsWith(query.toLower().data());
        i != m_values.end(); ++i) {
      matches.push_back(*i->second);
    }
    return QtPromise(std::move(matches));
  }
}

#endif
