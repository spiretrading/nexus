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

    private:
      std::shared_ptr<ComboBox::QueryModel> m_source;
      std::shared_ptr<AnyListModel> m_exclusions;

      bool is_in_exclusion(const std::any& value);
  };
}

#endif
