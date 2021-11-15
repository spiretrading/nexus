#ifndef SPIRE_COMBO_BOX_HPP
#define SPIRE_COMBO_BOX_HPP
#include <any>
#include <QWidget>
#include <Beam/Queries/SnapshotLimit.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

  /**
   * Represents a widget for selecting a value from a drop down list with a
   * secondary input component for filtering the selectable values.
   */
  class ComboBox: public QWidget {
    public:

      /** A model over a set of values that can be queried. */
      class QueryModel {
        public:

          /** Represents the parameters of a query. */
          struct Query {

            /** The query text. */
            std::string m_text;

            /** The limit for returned values. */
            Beam::Queries::SnapshotLimit m_limit;

            /** Returns an unlimited Query with empty text. */
            static Query make_empty_query();
          };

          virtual ~QueryModel() = default;

          /**
           * Queries the model for matching values.
           * @param query The query applied to the model.
           */
          virtual QtPromise<std::vector<std::any>>
            query(const Query& query) const = 0;
      };

      /** Constructs a ComboBox using a TextBox as the filter input. */
      explicit ComboBox(
        std::shared_ptr<QueryModel> model, QWidget* parent = nullptr);

    private:
      std::shared_ptr<QueryModel> m_model;
  };
}

#endif
