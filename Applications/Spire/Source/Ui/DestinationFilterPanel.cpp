#include "Spire/Ui/DestinationFilterPanel.hpp"
#include "Spire/Spire/ExcludingQueryModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

class DestinationExcludingQueryModel : public ExcludingQueryModel {
  public:
    using ExcludingQueryModel::ExcludingQueryModel;

  private:
    QString to_string(const std::any& value) override {
      return displayTextAny(
        std::any_cast<const DestinationDatabase::Entry&>(value).m_id);
    }
};

AnyInputBox* destination_box_builder(std::shared_ptr<ComboBox::QueryModel> model,
    std::shared_ptr<AnyListModel> matches) {
  auto box = new DestinationBox(
    std::make_shared<DestinationExcludingQueryModel>(model, matches));
  box->set_placeholder(QObject::tr("Search destinations"));
  auto input_box = new AnyInputBox(*box);
  input_box->connect_submit_signal([=] (const auto& submission) {
    input_box->get_current()->set(Destination());
    matches->push(any_cast<Destination>(submission));
  });
  return input_box;
}

DestinationFilterPanel* Spire::make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent) {
  return make_destination_filter_panel(std::move(query_model),
    std::make_shared<ArrayListModel<Destination>>(),
    std::make_shared<LocalValueModel<DestinationFilterPanel::Mode>>(
      DestinationFilterPanel::Mode::INCLUDE), parent);
}

DestinationFilterPanel* Spire::make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Destination>> matches,
    std::shared_ptr<ValueModel<DestinationFilterPanel::Mode>> mode,
    QWidget& parent) {
  return new DestinationFilterPanel(
    std::bind_front(destination_box_builder, std::move(query_model)),
    std::move(matches), std::move(mode), QObject::tr("Filter by Destination"),
    parent);
}
