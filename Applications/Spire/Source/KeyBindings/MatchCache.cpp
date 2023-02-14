#include "Spire/KeyBindings/MatchCache.hpp"

using namespace Spire;

MatchCache::MatchCache(Matcher matcher)
  : m_matcher(std::move(matcher)) {}

bool MatchCache::matches(const std::any& value, const QString& source) {
  if(m_caches.contains(source)) {
    return true;
  }
  auto matched = m_matcher(value, source);
  if(matched) {
    m_caches.insert(source);
  }
  return matched;
}
