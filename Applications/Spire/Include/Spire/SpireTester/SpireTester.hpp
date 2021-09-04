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
  class test_chart_model;

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
}

#endif
