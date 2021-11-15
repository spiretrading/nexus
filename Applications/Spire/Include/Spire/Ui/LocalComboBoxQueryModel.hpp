#ifndef SPIRE_LOCAL_COMBO_BOX_QUERY_MODEL_HPP
#define SPIRE_LOCAL_COMBO_BOX_QUERY_MODEL_HPP
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  class LocalComboBoxQueryModel : public ComboBox::QueryModel {
    public:

      void add(const std::any& value);
  
      void remove(const std::any& value);

      QtPromise<std::vector<std::any>>
        query(const ComboBox::QueryModel::Query& query) const override;

    private:
      struct AnyHash {
         std::size_t operator ()(const std::any& value) const;
      };

      struct Predicate {
			  bool operator ()(const std::any& first, const std::any& second) const;
			};

      std::unordered_set<std::any, AnyHash, Predicate> m_data;
  };
}

#endif
