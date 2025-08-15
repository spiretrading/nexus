#ifndef NEXUS_PARSE_FEE_TABLE_HPP
#define NEXUS_PARSE_FEE_TABLE_HPP
#include <array>
#include <exception>
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
      auto fee = Beam::Extract<T>(row.second);
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
    auto node = config[name];
    if(!node) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
    parse_fee_table(node, Beam::Store(table));
  }

  /**
   * Parses a table of fees from a YAML Node.
   * @param config The YAML Node to parse the fees from.
   * @param table The table to store the fees in.
   */
  template<typename T, std::size_t COLUMNS>
  void parse_fee_table(const YAML::Node& config,
      Beam::Out<std::array<T, COLUMNS>> table) {
    if(config.size() != COLUMNS) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Column size is invalid."));
    }
    for(auto column_index = std::size_t(0); column_index != COLUMNS;
        ++column_index) {
      auto& column = config[column_index];
      auto fee = Beam::Extract<T>(column);
      (*table)[column_index] = fee;
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
    auto node = config[name];
    if(!node) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
    parse_fee_table(node, Beam::Store(table));
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
      BOOST_THROW_EXCEPTION(std::runtime_error("Row size is invalid."));
    }
    for(auto row_index = std::size_t(0); row_index != ROWS; ++row_index) {
      auto& row = config[row_index];
      if(row.size() != COLUMNS) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Column size is invalid."));
      }
      for(auto column_index = std::size_t(0); column_index != COLUMNS;
          ++column_index) {
        auto& column = row[column_index];
        auto fee = Beam::Extract<T>(column);
        (*table)[row_index][column_index] = fee;
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
    auto node = config[name];
    if(!node) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error("Fee table \"" + name + "\" not found."));
    }
    parse_fee_table(node, Beam::Store(table));
  }
}

#endif
