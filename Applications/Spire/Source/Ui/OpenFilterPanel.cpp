#include "Spire/Ui/OpenFilterPanel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

OpenFilterPanel::OpenFilterPanel(
  std::shared_ptr<ComboBox::QueryModel> query_model, QString title,
  QWidget& parent) {}

OpenFilterPanel::OpenFilterPanel(
  InputBoxBuilder input_box_builder, QString title, QWidget& parent) {}

OpenFilterPanel::OpenFilterPanel(InputBoxBuilder input_box_builder,
  std::shared_ptr<AnyListModel> matches, std::shared_ptr<ValueModel<Mode>> mode,
  QString title, QWidget& parent) {}

const std::shared_ptr<AnyListModel>& OpenFilterPanel::get_matches() const {
  throw std::runtime_error("Not implemented.");
}

const std::shared_ptr<ValueModel<OpenFilterPanel::Mode>>&
    OpenFilterPanel::get_mode() const {
  throw std::runtime_error("Not implemented.");
}

connection OpenFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return {};
}
