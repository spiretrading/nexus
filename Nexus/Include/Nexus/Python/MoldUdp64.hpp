#ifndef NEXUS_PYTHON_MOLD_UDP_64_HPP
#define NEXUS_PYTHON_MOLD_UDP_64_HPP
#include <string_view>
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports MoldUDP64 packet parsing and the feed client.
   * @param module The module to export to.
   */
  void export_mold_udp64(pybind11::module& module);

  /**
   * Exports MoldUDP64 retransmission requests.
   * @param module The module to export to.
   */
  void export_mold_udp64_request(pybind11::module& module);

  /**
   * Exports MoldUDP64 packets.
   * @param module The module to export to.
   */
  void export_mold_udp64_packet(pybind11::module& module);

  /**
   * Exports the MoldUDP64 client using Beam channels.
   * @param module The module to export to.
   */
  void export_mold_udp64_client(pybind11::module& module);

  /**
   * Exports a MoldUdp64Client class.
   * @tparam T The type of client to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported client.
   */
  template<typename T>
  auto export_mold_udp64_client(
      pybind11::module& module, std::string_view name) {
    return pybind11::class_<T>(module, name.data(),
        "Supports one reader; close may be called from another thread.").
      def("read", &T::read, "Returns the next owned packet.").
      def("request", &T::request, pybind11::arg("request")).
      def("close", &T::close);
  }
}

#endif
