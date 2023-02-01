#ifndef SPIRE_TABLE_MATCHER_HPP
#define SPIRE_TABLE_MATCHER_HPP
#include <Beam/Collections/Trie.hpp>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** Performs a text match on a table. */
  class TableMatcher {
    public:

      using Builder = std::function<std::vector<QString> (const std::any&)>;

      /**
       * Constructs a TableMatcher.
       * @param model The table model.
       */
      TableMatcher(std::shared_ptr<TableModel> model, Builder builder);

      /**
       * Matches the text on a row.
       * @param row The row index to be matched.
       * @param text The string to match.
       * @return <code>true</code> if the text matches the content of the row.
       */
      bool match(int row, const QString& text) const;

    private:
      std::shared_ptr<TableModel> m_model;
      Builder m_builder;
      std::vector<std::unique_ptr<rtv::Trie<QChar, char>>> m_rows;
      boost::signals2::scoped_connection m_operation_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
