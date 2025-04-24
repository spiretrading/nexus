#ifndef SPIRE_ORDER_TASK_MATCH_HPP
#define SPIRE_ORDER_TASK_MATCH_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"

namespace Spire {

  /**
   * Checks if all words in the query can be matched to distinct row words
   * using prefix matching.
   * @param query A space-separated string containing query words.
   * @param words The list of candidate words.
   */
  bool matches(const QString& query, const std::vector<QString>& words);
}

#endif
