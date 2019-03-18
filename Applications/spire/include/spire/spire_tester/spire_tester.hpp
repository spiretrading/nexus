#ifndef SPIRE_TESTER_HPP
#define SPIRE_TESTER_HPP
#include <QApplication>
#include <QTimer>

namespace Spire {
  class test_chart_model;

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
    qDebug() << "\n**************************************************";
    qDebug() << name << " PASSED";
    qDebug() << "**************************************************\n";
  }
}

#endif
