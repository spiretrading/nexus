#include <memory>
#include "Nexus/Python/SoupBinTcp.hpp"
#include "Nexus/Python/ToPythonSoupBinTcpClient.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_soup_bin_tcp(module& module) {
  register_exception<SoupBinTcpParserException>(
    module, "SoupBinTcpParserException", PyExc_ValueError);
  class_<ToPythonSoupBinTcpPacket>(module, "SoupBinTcpPacket",
      "An owned SoupBinTCP packet with an independent bytes payload.").
    def_static("parse", [] (bytes source) {
      return ToPythonSoupBinTcpPacket(static_cast<std::string_view>(source));
    }, arg("source").noconvert()).
    def_property_readonly("length", [] (const ToPythonSoupBinTcpPacket& self) {
      return self.get().m_length;
    }).
    def_property_readonly("type", [] (const ToPythonSoupBinTcpPacket& self) {
      return self.get().m_type;
    }).
    def_property_readonly("payload", [] (const ToPythonSoupBinTcpPacket& self) {
      auto payload = self.get().get_payload();
      return bytes(payload.data(), payload.size());
    });
  class_<LoginAcceptedPacket>(module, "LoginAcceptedPacket").
    def_readonly("session", &LoginAcceptedPacket::m_session).
    def_readonly("sequence_number", &LoginAcceptedPacket::m_sequence_number);
  class_<LoginRejectedPacket>(module, "LoginRejectedPacket").
    def_readonly("reason", &LoginRejectedPacket::m_reason);
  module.def("parse_login_accepted_packet",
    [] (const ToPythonSoupBinTcpPacket& packet) {
      return parse_login_accepted_packet(packet.get());
    }, arg("packet"));
  module.def("parse_login_rejected_packet",
    [] (const ToPythonSoupBinTcpPacket& packet) {
      return parse_login_rejected_packet(packet.get());
    }, arg("packet"));
  module.def("make_login_request_packet", [] (std::string_view username,
      std::string_view password, std::string_view session,
      std::uint64_t sequence_number) {
    auto buffer = SharedBuffer();
    make_login_request_packet(
      username, password, session, sequence_number, out(buffer));
    return bytes(buffer.get_data(), buffer.get_size());
  }, arg("username"), arg("password"), arg("session"), arg("sequence_number"));
  module.def("make_client_heartbeat_packet", [] {
    auto buffer = SharedBuffer();
    make_client_heartbeat_packet(out(buffer));
    return bytes(buffer.get_data(), buffer.get_size());
  });
  using Client =
    ToPythonSoupBinTcpClient<SoupBinTcpClient<Channel, std::shared_ptr<Timer>>>;
  export_soup_bin_tcp_client<Client>(module, "SoupBinTcpClient").
    def(init<std::string_view, std::string_view, Channel,
      std::shared_ptr<Timer>>(),
      arg("username"), arg("password"), arg("channel"), arg("timer"),
      keep_alive<1, 4>(), keep_alive<1, 5>(),
      "Connects using a Beam channel and a one-second timer.").
    def(init<std::string_view, std::string_view, std::string_view,
      std::uint64_t, Channel, std::shared_ptr<Timer>>(),
      arg("username"), arg("password"),
      arg("session"), arg("sequence_number"), arg("channel"), arg("timer"),
      keep_alive<1, 6>(), keep_alive<1, 7>(),
      "Resumes a session using a Beam channel and a one-second timer.").
    def_readonly_static("TIMEOUT_TICKS", &Client::Client::TIMEOUT_TICKS);
}
