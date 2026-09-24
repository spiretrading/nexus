#ifndef NEXUS_PYTHON_SOUP_BIN_TCP_HPP
#define NEXUS_PYTHON_SOUP_BIN_TCP_HPP
#include <string_view>
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports SoupBinTCP packets and the session client.
   * @param module The module to export to.
   */
  void export_soup_bin_tcp(pybind11::module& module);

  /**
   * Exports SoupBinTCP packets and heartbeat construction.
   * @param module The module to export to.
   */
  void export_soup_bin_tcp_packet(pybind11::module& module);

  /**
   * Exports login acceptance packets and parsing.
   * @param module The module to export to.
   */
  void export_login_accepted_packet(pybind11::module& module);

  /**
   * Exports login rejection packets and parsing.
   * @param module The module to export to.
   */
  void export_login_rejected_packet(pybind11::module& module);

  /**
   * Exports login request construction.
   * @param module The module to export to.
   */
  void export_login_request_packet(pybind11::module& module);

  /**
   * Exports the SoupBinTCP client using Beam channels and timers.
   * @param module The module to export to.
   */
  void export_soup_bin_tcp_client(pybind11::module& module);

  /**
   * Exports a SoupBinTcpClient class.
   * @tparam T The type of client to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported client.
   */
  template<typename T>
  auto export_soup_bin_tcp_client(
      pybind11::module& module, std::string_view name) {
    return pybind11::class_<T>(module, name.data(),
        "Supports one reader; close may be called from another thread.").
      def_property_readonly("session", &T::get_session).
      def_property_readonly("sequence_number", &T::get_sequence_number).
      def("read", &T::read, "Returns the next owned packet.").
      def("close", &T::close);
  }
}

#endif
