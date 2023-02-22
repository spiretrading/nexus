#include "Spire/KeyBindings/MatchCache.hpp"

using namespace Spire;

MatchCache::MatchCache(Matcher matcher)
  : m_matcher(std::move(matcher)) {}

bool MatchCache::matches(const QString& query) {
  if(m_caches.contains(query)) {
    return true;
  }
  auto matched = m_matcher(query);
  if(matched) {
    m_caches.insert(query);
  }
  return matched;
}
