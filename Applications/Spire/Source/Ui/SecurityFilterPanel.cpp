#include "Spire/Ui/SecurityFilterPanel.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Spire/ExcludingQueryModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

class SecurityExcludingQueryModel : public ExcludingQueryModel {
  public:
    using ExcludingQueryModel::ExcludingQueryModel;

  private:
    QString to_string(const std::any& value) override {
      return displayText(
        std::any_cast<const SecurityInfo&>(value).m_security);
    }
};

AnyInputBox* security_box_builder(std::shared_ptr<ComboBox::QueryModel> model,
    std::shared_ptr<AnyListModel> matches) {
  auto box = new SecurityBox(
    std::make_shared<SecurityExcludingQueryModel>(model, matches));
  box->set_placeholder(QObject::tr("Search securities"));
  auto input_box = new AnyInputBox(*box);
  input_box->connect_submit_signal([=] (const auto& submission) {
    input_box->get_current()->set(Security());
    matches->push(any_cast<Security>(submission));
  });
  return input_box;
}

SecurityFilterPanel* Spire::make_security_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent) {
  return make_security_filter_panel(std::move(query_model),
    std::make_shared<ArrayListModel<Security>>(),
    std::make_shared<LocalValueModel<SecurityFilterPanel::Mode>>(
      SecurityFilterPanel::Mode::INCLUDE), parent);
}

SecurityFilterPanel* Spire::make_security_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Security>> matches,
    std::shared_ptr<ValueModel<SecurityFilterPanel::Mode>> mode,
    QWidget& parent) {
  return new SecurityFilterPanel(
    std::bind_front(security_box_builder, std::move(query_model)),
    std::move(matches), std::move(mode), QObject::tr("Filter by Security"),
    parent);
}
