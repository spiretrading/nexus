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
      std::vector<std::any> m_data;

      boost::optional<size_t> index_of(const std::any& value);
  };
}

#endif
