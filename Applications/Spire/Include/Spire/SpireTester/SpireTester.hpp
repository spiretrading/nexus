#ifndef SPIRE_TESTER_HPP
#define SPIRE_TESTER_HPP
#include <QApplication>
#include <QTimer>

namespace Spire {
  class test_chart_model;

  //! Runs a unit test that use QtPromises.
  /*!
    \param test The unit test to run.
  */
  template<typename T>
  void run_test(T&& test) {
    auto argc = 0;
    auto app = QCoreApplication(argc, nullptr);
    QTimer::singleShot(0,
      [&] {
        test();
        app.exit();
      });
    app.exec();
  }
}

#endif
