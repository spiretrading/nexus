#ifndef SPIRE_TABLE_MATCH_CACHE_HPP
#define SPIRE_TABLE_MATCH_CACHE_HPP
#include "Spire/KeyBindings/MatchCache.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /** Represents a cache for table matching. */
  class TableMatchCache {
    public:

      using Matcher = MatchCache::Matcher;

      /**
       * Constructs a TableMatchCache.
       * @param model The table model.
       * @param matcher The matcher function used to match.
       */
      TableMatchCache(std::shared_ptr<TableModel> model, Matcher matcher);

      /**
       * Matches the value in a specified row and column against a string.
       * @param row The row of the specific value to be matched.
       * @param column The column of the specific value to be matched.
       * @param string The search string.
       * @return <code>true</code> if the value in the sprecified row and column
       *         matches the string.
       */
      bool matches(int row, int column, const QString& string);

    private:
      std::shared_ptr<TableModel> m_model;
      Matcher m_matcher;
      std::vector<std::vector<MatchCache>> m_caches;
      boost::signals2::scoped_connection m_operation_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
