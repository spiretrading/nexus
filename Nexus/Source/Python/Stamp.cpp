#include "Nexus/Python/Stamp.hpp"
#include "Nexus/Python/ToPythonStampField.hpp"
#include "Nexus/Python/ToPythonStampMessage.hpp"

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_stamp(module& module) {
  register_exception<StampParserException>(
    module, "StampParserException", PyExc_ValueError);
  export_stamp_field(module);
  export_stamp_message(module);
}

void Nexus::Python::export_stamp_field(module& module) {
  class_<ToPythonStampField>(module, "StampField").
    def_static("parse", [] (bytes source) {
      return ToPythonStampField(
        StampField::parse(static_cast<std::string_view>(source)));
    }, arg("source").noconvert()).
    def_property_readonly("identifier", [] (const ToPythonStampField& self) {
      return self.get().m_identifier;
    }).
    def_property_readonly("index", [] (const ToPythonStampField& self) {
      return self.get().m_index;
    }).
    def_property_readonly("value", [] (const ToPythonStampField& self) {
      return self.get().m_value;
    });
}

void Nexus::Python::export_stamp_message(module& module) {
  auto message = class_<ToPythonStampMessage>(module, "StampMessage").
    def_static("parse", [] (bytes source) {
      return ToPythonStampMessage(static_cast<std::string_view>(source));
    }, arg("source").noconvert()).
    def_property_readonly("control_header",
      [] (const ToPythonStampMessage& self) -> const StampMessage::Section& {
        return self.get().m_control_header;
      }, return_value_policy::reference_internal).
    def_property_readonly("business_content",
      [] (const ToPythonStampMessage& self) -> const StampMessage::Section& {
        return self.get().m_business_content;
      }, return_value_policy::reference_internal);
  class_<StampMessage::Section>(message, "Section").
    def("find", [] (const StampMessage::Section& self,
        std::uint16_t identifier) -> object {
      if(auto field = self.find(identifier)) {
        return cast(ToPythonStampField(*field));
      }
      return none();
    }, arg("identifier")).
    def("find", [] (const StampMessage::Section& self,
        std::uint16_t identifier, std::uint16_t index) -> object {
      if(auto field = self.find(identifier, index)) {
        return cast(ToPythonStampField(*field));
      }
      return none();
    }, arg("identifier"), arg("index")).
    def("__iter__", [] (const StampMessage::Section& self) {
      auto fields = list();
      for(auto& field : self) {
        fields.append(ToPythonStampField(field));
      }
      return fields.attr("__iter__")();
    });
}
