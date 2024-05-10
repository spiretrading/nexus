#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/RecycledListViewBuilder.hpp"

#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Ui/IntegerBox.hpp"

using namespace boost;
using namespace Spire;

struct Builder {
  QWidget* mount(const std::shared_ptr<ListModel<int>>& model, int index) {
    auto box = new IntegerBox(make_proxy_scalar_value_model(
      make_optional_scalar_value_model_decorator(
        make_scalar_value_model_decorator(
          make_list_value_model(model, index)))), {});
    apply_label_style(*box);
    return box;
  }

  void reset(QWidget& widget, const std::shared_ptr<ListModel<int>>& model,
      int index) {
    static_cast<ProxyScalarValueModel<optional<int>>&>(
      *static_cast<IntegerBox&>(widget).get_current()).set_source(
        make_optional_scalar_value_model_decorator(
          make_scalar_value_model_decorator(
            make_list_value_model(model, index))));
  }

  void unmount(QWidget* widget) {
    delete widget;
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto list = std::make_shared<ArrayListModel<int>>();
  for(auto i = 0; i != 1000; ++i) {
    list->push(i);
  }
  auto list_view =
    new ListView(list, RecycledListViewBuilder(Builder()));
  auto scroll_box = new ScrollableListBox(*list_view);
  scroll_box->resize(300, 100);
  scroll_box->show();
  application.exec();
}
