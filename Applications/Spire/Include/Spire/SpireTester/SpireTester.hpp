#ifndef SPIRE_TESTER_HPP
#define SPIRE_TESTER_HPP
#include <utility>
#include <QApplication>
#include <QTimer>
#include <QtPlugin>

#ifdef _MSC_VER
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __linux__
  Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

namespace Spire {
  class TestChartModel;

  /**
   * Runs a unit test that use QtPromises.
   * @param test The unit test to run.
   */
  template<typename T>
  void run_test(T&& test) {
    auto argc = 0;
    auto app = QApplication(argc, nullptr);
    QTimer::singleShot(0,
      [&] {
        std::forward<T>(test)();
        app.exit();
      });
    app.exec();
  }

  /**
   * Processes events in the Qt event loop until the given predicate returns
   * true.
   */
  void wait_until(const std::function<bool ()>& predicate);
}

#endif
