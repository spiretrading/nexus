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
       * The type of function used to build a matcher.
       * @param table The table of values being matched.
       * @param row The row of the specific value to be matched.
       * @param column The column of the specific value to be matched.
       * @return The Matcher used to match the value in the
       *         <i>table</i> at the given <i>row</i> and <i>column</i>.
       */
      using MatcherBuilder =
        std::function<Matcher (const std::shared_ptr<TableModel>& table,
          int row, int column)>;

      /**
       * Constructs a TableMatchCache.
       * @param table The table model.
       * @param builder The builder used to build a matcher.
       */
      TableMatchCache(std::shared_ptr<TableModel> table, MatcherBuilder builder);

      /**
       * Matches a specified row against the query.
       * @param row The row to be matched.
       * @param query The query string.
       * @return <code>true</code> if the specified row matches the query.
       */
      bool matches(int row, const QString& query);

    private:
      std::shared_ptr<TableModel> m_table;
      MatcherBuilder m_builder;
      std::vector<std::vector<MatchCache>> m_caches;
      boost::signals2::scoped_connection m_operation_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
