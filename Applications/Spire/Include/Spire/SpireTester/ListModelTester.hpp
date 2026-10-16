#ifndef SPIRE_LIST_MODEL_TESTER_HPP
#define SPIRE_LIST_MODEL_TESTER_HPP
#include <deque>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /**
   * Tests that a series of operations performed on a ListModel matches.
   * @param operations The series of operations performed on a ListModel.
   * @param expected The expected series of operations.
   */
  template<typename T>
  void require_list_transaction(
      const std::deque<typename ListModel<T>::Operation>& operations,
      const std::vector<typename ListModel<T>::Operation>& expected) {
    auto offset = 0;
    if(expected.size() == 1) {
      if(operations.size() != 1) {
        REQUIRE(operations.size() == 3);
        REQUIRE(
          std::get_if<typename ListModel<T>::StartTransaction>(&operations[0]));
        offset = 1;
      }
    } else {
      if(!std::get_if<typename ListModel<T>::StartTransaction>(
          &operations[0])) {
        REQUIRE(operations.size() == 2);
        REQUIRE(std::get_if<typename ListModel<T>::PreRemoveOperation>(
          &operations[0]));
      } else {
        REQUIRE(operations.size() == expected.size() + 2);
        offset = 1;
      }
    }
    for(auto i = 0; i != std::ssize(expected); ++i) {
      visit(expected[i],
        [&] (const ListModel<T>::AddOperation& expected) {
          auto operation =
            std::get_if<ListModel<T>::AddOperation>(&operations[i + offset]);
          REQUIRE(operation != nullptr);
          REQUIRE(operation->m_index == expected.m_index);
        },
        [&] (const ListModel<T>::RemoveOperation& expected) {
          auto operation =
            std::get_if<ListModel<T>::RemoveOperation>(&operations[i + offset]);
          REQUIRE(operation != nullptr);
          REQUIRE(operation->m_index == expected.m_index);
        },
        [&] (const ListModel<T>::MoveOperation& expected) {
          auto operation =
            std::get_if<ListModel<T>::MoveOperation>(&operations[i + offset]);
          REQUIRE(operation != nullptr);
          REQUIRE(operation->m_source == expected.m_source);
          REQUIRE(operation->m_destination == expected.m_destination);
        },
        [&] (const ListModel<T>::UpdateOperation& expected) {
          auto operation =
            std::get_if<ListModel<T>::UpdateOperation>(&operations[i + offset]);
          REQUIRE(operation != nullptr);
          REQUIRE(operation->m_index == expected.m_index);
        });
    }
    if(offset != 0) {
      REQUIRE(
        std::get_if<typename ListModel<T>::EndTransaction>(&operations.back()));
    }
  }
}

#endif
