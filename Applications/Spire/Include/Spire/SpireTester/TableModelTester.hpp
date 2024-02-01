#ifndef SPIRE_TABLE_MODEL_TESTER_HPP
#define SPIRE_TABLE_MODEL_TESTER_HPP
#include <deque>
#include <vector>
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Tests that a series of operations performed on a TableModel matches.
   * @param operations The series of operations performed on a TableModel.
   * @param expected The expected series of operations.
   */
  void require_transaction(const std::deque<TableModel::Operation>& operations,
    const std::vector<TableModel::Operation>& expected);
}

#endif
