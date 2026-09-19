#include "Nexus/Python/MoldUdp64.hpp"
#include "Nexus/Python/ToPythonMoldUdp64Client.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_mold_udp64(module& module) {
  register_exception<MoldUdp64ParserException>(
    module, "MoldUdp64ParserException", PyExc_ValueError);
  class_<ToPythonMoldUdp64Packet>(module, "MoldUdp64Packet",
      "An owned MoldUDP64 packet with independent bytes payloads.").
    def_static("parse", [] (bytes source) {
      return ToPythonMoldUdp64Packet(static_cast<std::string_view>(source));
    }, arg("source").noconvert(), "Parses one complete downstream datagram.").
    def_property_readonly("session", [] (const ToPythonMoldUdp64Packet& self) {
      return bytes(self.get().m_session.get_data(),
        MoldUdp64Packet::SESSION_FIELD_LENGTH);
    }, "The exact 10-byte session identifier.").
    def_property_readonly("sequence_number",
      [] (const ToPythonMoldUdp64Packet& self) {
        return self.get().m_sequence_number;
      }).
    def_property_readonly("count", [] (const ToPythonMoldUdp64Packet& self) {
      return self.get().m_count;
    }).
    def_property_readonly("is_heartbeat",
      [] (const ToPythonMoldUdp64Packet& self) {
        return self.get().is_heartbeat();
      }).
    def_property_readonly("is_end_of_session",
      [] (const ToPythonMoldUdp64Packet& self) {
        return self.get().is_end_of_session();
      }).
    def("__iter__", [] (const ToPythonMoldUdp64Packet& self) {
      auto payloads = list();
      for(auto& message : self.get()) {
        payloads.append(bytes(message.m_data, message.m_length));
      }
      return payloads.attr("__iter__")();
    });
  using Client = ToPythonMoldUdp64Client<MoldUdp64Client<Channel>>;
  export_mold_udp64_client<Client>(module, "MoldUdp64Client").
    def(init<Channel>(), arg("channel"), keep_alive<1, 2>(),
      "Uses a Beam channel delivering one complete datagram per read.");
}
