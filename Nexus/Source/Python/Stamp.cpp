#include "Nexus/Python/Stamp.hpp"
#include <algorithm>
#include "Nexus/Python/ToPythonStampField.hpp"
#include "Nexus/Python/ToPythonStampMessage.hpp"
#include "Nexus/Stamp/StampFieldReader.hpp"

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_stamp(module& module) {
  register_exception<StampParserException>(
    module, "StampParserException", PyExc_ValueError);
  export_stamp_field(module);
  export_stamp_message(module);
  export_stamp_field_reader(module);
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

void Nexus::Python::export_stamp_field_reader(module& module) {
  class_<StampFieldReader>(module, "StampFieldReader").
    def(
      init<const StampMessage::Section&>(), arg("section"), keep_alive<1, 2>()).
    def("read", [] (const StampFieldReader& self,
        std::uint16_t identifier, function parser) {
      return self.read(identifier, [&] (auto value) {
        return parser(value);
      });
    }, arg("identifier"), arg("parser")).
    def("read", [] (const StampFieldReader& self,
        std::uint16_t identifier, std::uint16_t index, function parser) {
      return self.read(identifier, index, [&] (auto value) {
        return parser(value);
      });
    }, arg("identifier"), arg("index"), arg("parser")).
    def("read_optional", [] (const StampFieldReader& self,
        std::uint16_t identifier, function parser) -> object {
      if(auto value = self.read_optional(identifier, [&] (auto value) {
          return parser(value);
        })) {
        return *value;
      }
      return none();
    }, arg("identifier"), arg("parser")).
    def("read_optional", [] (const StampFieldReader& self,
        std::uint16_t identifier, std::uint16_t index, function parser) ->
        object {
      if(auto value = self.read_optional(identifier, index,
          [&] (auto value) { return parser(value); })) {
        return *value;
      }
      return none();
    }, arg("identifier"), arg("index"), arg("parser")).
    def("get_count", [] (const StampFieldReader& self, iterable identifiers) {
      auto count = std::uint16_t(0);
      for(auto identifier : identifiers) {
        count = std::max(count,
          self.get_count({cast<std::uint16_t>(identifier)}));
      }
      return count;
    }, arg("identifiers"));
}
