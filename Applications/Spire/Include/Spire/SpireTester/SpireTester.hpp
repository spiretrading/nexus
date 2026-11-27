#ifndef SPIRE_TESTER_HPP
#define SPIRE_TESTER_HPP
#include <doctest/doctest.h>
#include <QApplication>

/**
 * Defines a test case that runs within a UiTestFixture.
 * @param name The name of the test case.
 */
#define UI_TEST_CASE(name) TEST_CASE_FIXTURE(Spire::Tests::UiTestFixture, name)

namespace Spire::Tests {

  /**
   * Provides an offscreen QApplication with Spire resources at a fixed DPI.
   */
  class UiTestFixture {
    public:

      /** Constructs a UiTestFixture. */
      UiTestFixture();

      ~UiTestFixture();

    private:
      int m_count;
      QApplication m_application;
  };
}

#endif
