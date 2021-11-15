#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/LocalComboBoxQueryModel.hpp"

using namespace Spire;

const auto VALUES = std::unordered_set<QString>({"A", "AB", "B", "C"});

auto make_model() {
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  for(auto& value : VALUES) {
    model->add(value);
  }
  return model;
}

using Query = ComboBox::QueryModel::Query;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto model = make_model();
  auto promise = model->query(Query::make_empty_query());
  auto result =
    wait(promise.then([] (auto&& result) { return result.Get(); }));
  auto a = 0;
  application->exec();
}
