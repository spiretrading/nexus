#include "Spire/KeyBindings/NoneAdditionalTagSchema.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;

const std::shared_ptr<NoneAdditionalTagSchema>&
    NoneAdditionalTagSchema::get_instance() {
  static const auto schema =
    std::shared_ptr<NoneAdditionalTagSchema>(new NoneAdditionalTagSchema());
  return schema;
}

bool NoneAdditionalTagSchema::test(const AdditionalTag& tag) const {
  return false;
}

AnyInputBox* NoneAdditionalTagSchema::make_input_box(
    std::shared_ptr<AdditionalTagValueModel> current,
    const SubmitSignal::slot_type& submission) const {
  current->set(none);
  return new AnyInputBox(*make_label(""));
}

std::unique_ptr<CanvasNode> NoneAdditionalTagSchema::make_canvas_node(
    const boost::optional<Nexus::Tag::Type>& value) const {
  return std::make_unique<NoneNode>();
}

NoneAdditionalTagSchema::NoneAdditionalTagSchema()
  : AdditionalTagSchema(OrderFieldInfoTip::Model(), -1) {}
