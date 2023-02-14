#ifndef SPIRE_MATCH_CACHE_HPP
#define SPIRE_MATCH_CACHE_HPP
#include <any>
#include <functional>
#include <qhash>
#include <unordered_set>
#include <QString>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Represents a cache for matching. */
  class MatchCache {
    public:

      /**
       * The type of function used to match a value and a string.
       * @param value The value to be matched.
       * @param string The search string.
       * @return <code>true</code> if the string matches the value.
       */
      using Matcher =
        std::function<bool (const std::any& value, const QString& string)>;

      /**
       * Constructs a MatchCache.
       * @param matcher The macher used to match.
       */
      explicit MatchCache(Matcher matcher);

      /**
       * Matches a value against a string.
       * @param value The value to be matched.
       * @param string The search string.
       * @return <code>true</code> if the string matches the value.
       */
      bool matches(const std::any& value, const QString& string);

    private:
      Matcher m_matcher;
      std::unordered_set<QString> m_caches;
  };
}

#endif
