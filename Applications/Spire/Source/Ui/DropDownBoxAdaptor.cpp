#include "Spire/Ui/DropDownBoxAdaptor.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;

DropDownBoxAdaptor::DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
  QWidget* parent)
  : DropDownBoxAdaptor(std::move(list_model),
      std::make_shared<LocalValueModel<optional<int>>>(), parent) {}

DropDownBoxAdaptor::DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
  std::shared_ptr<ValueModel<optional<int>>> current_model, QWidget* parent)
  : DropDownBox(*make_list_view(std::move(list_model),
      std::move(current_model)), parent) {}

const std::shared_ptr<ListModel>& DropDownBoxAdaptor::get_list_model() const {
  return m_list_view->get_list_model();
}

const std::shared_ptr<ValueModel<optional<int>>>&
    DropDownBoxAdaptor::get_current_model() const {
  return m_list_view->get_current_model();
}

ListView* DropDownBoxAdaptor::make_list_view(
    std::shared_ptr<ListModel> list_model,
    std::shared_ptr<ValueModel<optional<int>>> current_model) {
  m_list_view = new ListView(std::move(list_model),
    [] (const auto& model, auto index) {
      return make_label(displayTextAny(model->at(index)));
    }, std::move(current_model),
    std::make_shared<LocalValueModel<optional<int>>>());
  return m_list_view;
}
