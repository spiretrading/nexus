#ifndef SPIRE_MATCH_CACHE_HPP
#define SPIRE_MATCH_CACHE_HPP
#include <functional>
#include <unordered_set>
#include <QHash>
#include <QString>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Represents a cache for matching. */
  class MatchCache {
    public:

      /**
       * The type of function used to match a query string.
       * @param query The query string.
       * @return <code>true</code> if the query string is matched.
       */
      using Matcher = std::function<bool (const QString& query)>;

      /**
       * Constructs a MatchCache.
       * @param matcher The macher used to match.
       */
      explicit MatchCache(Matcher matcher);

      /**
       * Matches the query string.
       * @param query The query string.
       * @return <code>true</code> if the query string is matched.
       */
      bool matches(const QString& query);

    private:
      Matcher m_matcher;
      std::unordered_set<QString> m_caches;
  };
}

#endif
