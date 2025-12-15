#ifndef NEXUS_PARSE_FEE_TABLE_HPP
#define NEXUS_PARSE_FEE_TABLE_HPP
#include <array>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param table The table to store the fees in.
   */
  template<typename T>
  void parse_fee_table(const YAML::Node& config,
      Beam::Out<std::unordered_map<std::string, T>> table) {
    for(auto& row : config) {
      auto flag = row.first.as<std::string>();
      auto fee = Beam::extract<T>(row.second);
      table->insert(std::pair(flag, fee));
    }
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param name The name of the fee table to parse.
   * @param table The table to store the fees in.
   */
  template<typename T>
  void parse_fee_table(const YAML::Node& config, const std::string& name,
      Beam::Out<std::unordered_map<std::string, T>> table) {
    if(auto node = config[name]) {
      parse_fee_table(node, Beam::out(table));
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param table The table to store the fees in.
   */
  template<typename T, std::size_t COLUMNS>
  void parse_fee_table(
      const YAML::Node& config, Beam::Out<std::array<T, COLUMNS>> table) {
    if(config.size() != COLUMNS) {
      boost::throw_with_location(std::runtime_error("Column size is invalid."));
    }
    for(auto i = std::size_t(0); i != COLUMNS; ++i) {
      auto& column = config[i];
      auto fee = Beam::extract<T>(column);
      (*table)[i] = fee;
    }
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param name The name of the fee table to parse.
   * @param table The table to store the fees in.
   */
  template<typename T, std::size_t COLUMNS>
  void parse_fee_table(const YAML::Node& config, const std::string& name,
      Beam::Out<std::array<T, COLUMNS>> table) {
    if(auto node = config[name]) {
      parse_fee_table(node, Beam::out(table));
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param table The table to store the fees in.
   */
  template<typename T, std::size_t ROWS, std::size_t COLUMNS>
  void parse_fee_table(const YAML::Node& config,
      Beam::Out<std::array<std::array<T, COLUMNS>, ROWS>> table) {
    if(config.size() != ROWS) {
      boost::throw_with_location(std::runtime_error("Row size is invalid."));
    }
    for(auto i = std::size_t(0); i != ROWS; ++i) {
      auto& row = config[i];
      if(row.size() != COLUMNS) {
        boost::throw_with_location(
          std::runtime_error("Column size is invalid."));
      }
      for(auto j = std::size_t(0); j != COLUMNS; ++j) {
        auto& column = row[j];
        auto fee = Beam::extract<T>(column);
        (*table)[i][j] = fee;
      }
    }
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param name The name of the fee table to parse.
   * @param table The table to store the fees in.
   */
  template<typename T, std::size_t ROWS, std::size_t COLUMNS>
  void parse_fee_table(const YAML::Node& config, const std::string& name,
      Beam::Out<std::array<std::array<T, COLUMNS>, ROWS>> table) {
    if(auto node = config[name]) {
      parse_fee_table(node, Beam::out(table));
    } else {
      boost::throw_with_location(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
  }
}

#endif
