#ifndef SPIRE_TESTER_HPP
#define SPIRE_TESTER_HPP
#include <QApplication>
#include <QTimer>

namespace Spire {
  class test_chart_model;

  //! Prints a unit test's name using QDebug.
  /*!
    \param name The unit test's name.
  */
  void print_test_name(const QString& name);

  //! Runs a unit test that use QtPromises.
  /*!
    \param test The unit test to run.
    \param name The unit test's name.
  */
  template <typename T>
  void run_test(T&& test, const QString& name) {
    auto argc = 0;
    auto app = QCoreApplication(argc, nullptr);
    QTimer::singleShot(0,
      [&] {
        test();
        app.exit();
    });
    app.exec();
    print_test_name(name);
  }
}

#endif
