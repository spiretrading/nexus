#ifndef SPIRE_ORDER_TASK_MATCH_HPP
#define SPIRE_ORDER_TASK_MATCH_HPP
#include <QStringList>
#include "Spire/KeyBindings/OrderTaskArguments.hpp"

namespace Spire {

  /**
   * Checks if all words in the query can be matched to distinct row words
   * using prefix matching.
   * @param query A space-separated string containing query words.
   * @param words The list of candidate words.
   */
  bool matches(const QString& query, const std::vector<QString>& words);

  /**
   * Checks if all query words can be matched to distinct row words using
   * prefix matching.
   * @param query_words The lowercase words to match.
   * @param words The list of candidate words.
   */
  bool matches(
    const QStringList& query_words, const std::vector<QString>& words);
}

#endif
