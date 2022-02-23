#ifndef SPIRE_EXCLUDING_QUERY_MODEL_HPP
#define SPIRE_EXCLUDING_QUERY_MODEL_HPP
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /**
   * Implements a QueryModel excluding any matches that exist in the exclusion
   * list.
   */
  class ExcludingQueryModel : public ComboBox::QueryModel {
    public:

      /**
       * Constructs an ExcludingQueryModel
       * @param source The source model being queried.
       * @param exclusion The list of values being excluded from query matches.
       */
      ExcludingQueryModel(std::shared_ptr<ComboBox::QueryModel> source,
        std::shared_ptr<AnyListModel> exclusions);

      std::any parse(const QString& query) override;

      QtPromise<std::vector<std::any>> submit(const QString& query) override;

      /**
       * Converts a std::any to a QString.
       * @param value The value of std::any.
       * @return The string representation of the value.
       */
      virtual QString to_string(const std::any& value);

    private:
      std::shared_ptr<ComboBox::QueryModel> m_source;
      std::shared_ptr<AnyListModel> m_exclusions;
      std::unordered_set<QString> m_exclusion_set;
      std::vector<QString> m_exclusion_list;
      boost::signals2::scoped_connection m_connection;

      void add_exclusion(int index);
      void on_operation(const AnyListModel::Operation& operation);
  };
}

#endif
