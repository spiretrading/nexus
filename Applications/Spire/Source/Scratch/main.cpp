#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QVBoxLayout>
#include <QWidget>
#include "Spire/Ui/RangeInputWidget.hpp"
#include "Spire/Ui/ScalarWidget.hpp"
#include "Spire/Ui/QuantityInputWidget.hpp"
#include "Spire/Spire/LocalRangeInputModel.hpp"
#include "Spire/Ui/RangeInputWidget.hpp"
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto layout = new QVBoxLayout(window);
  auto min1 = new QuantityInputWidget(window);
  auto max1 = new QuantityInputWidget(window);
  std::srand((unsigned) std::time(0));
  auto data = [] (size_t min, size_t max) {
    auto data = std::vector<Scalar>();
    for(auto i = std::size_t(0); i < max; ++i) {
      data.push_back(Scalar(rand() % max + min));
    }
    return data;
  };
  auto model1 = std::make_shared<LocalRangeInputModel>(data(1, 3));
  auto scalar_min1 = new ScalarWidget(min1,
    &QuantityInputWidget::connect_modified_signal,
    &QuantityInputWidget::set_value);
  auto scalar_max1 = new ScalarWidget(max1,
    &QuantityInputWidget::connect_modified_signal,
    &QuantityInputWidget::set_value);
  auto range1 = new RangeInputWidget(model1, scalar_min1, scalar_max1,
    Scalar(1), window);
  scalar_min1->setFixedWidth(120);
  scalar_max1->setFixedWidth(120);
  scalar_min1->setFixedHeight(39);
  range1->setFixedHeight(49);
  layout->addWidget(range1);
  window->resize(600, 200);
  window->show();
  application->exec();
}
