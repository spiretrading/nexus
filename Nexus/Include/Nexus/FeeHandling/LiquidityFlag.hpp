#ifndef NEXUS_LIQUIDITYFLAG_HPP
#define NEXUS_LIQUIDITYFLAG_HPP
#include <array>
#include <exception>
#include <string>
#include <unordered_map>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"

namespace Nexus {

  /*! \enum LiquidityFlag
      \brief Enumerates liquidity flags.
   */
  enum class LiquidityFlag : int {

    //! No liquidity flag specified.
    NONE = -1,

    //! The trade removed liquidity.
    ACTIVE = 0,

    //! The trade provided liquidity.
    PASSIVE
  };

  //! The number of liquidity flags enumerated.
  static const std::size_t LIQUIDITY_FLAG_COUNT = 2;

  //! Returns a string representing a LiquidityFlag.
  /*!
    \param flag The LiquidityFlag to represent.
    \return The string representation of the <i>flag</i>.
  */
  inline const std::string& ToString(LiquidityFlag flag) {
    if(flag == LiquidityFlag::NONE) {
      static auto flag = std::string{""};
      return flag;
    } else if(flag == LiquidityFlag::ACTIVE) {
      static auto flag = std::string{"A"};
      return flag;
    } else if(flag == LiquidityFlag::PASSIVE) {
      static auto flag = std::string{"P"};
      return flag;
    } else {
      static auto flag = std::string{"?"};
      return flag;
    }
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param table The table to store the fees in.
  */
  template<typename T>
  void ParseFeeTable(const YAML::Node& config,
      Beam::Out<std::unordered_map<std::string, T>> table) {
    for(auto i = config.begin(); i != config.end(); ++i) {
      std::string flag;
      i.first() >> flag;
      auto fee = Beam::Extract<T>(i.second());
      table->insert(std::make_pair(flag, fee));
    }
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param name The name of the fee table to parse.
    \param table The table to store the fees in.
  */
  template<typename T>
  void ParseFeeTable(const YAML::Node& config, const std::string& name,
      Beam::Out<std::unordered_map<std::string, T>> table) {
    auto node = config.FindValue(name);
    if(node == nullptr) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error{"Fee table \"" + name + "\" not found."});
    }
    ParseFeeTable(*node, Beam::Store(table));
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param table The table to store the fees in.
  */
  template<typename T, std::size_t COLUMNS>
  void ParseFeeTable(const YAML::Node& config,
      Beam::Out<std::array<T, COLUMNS>> table) {
    if(config.size() != COLUMNS) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Column size is invalid."});
    }
    for(auto columnIndex = 0; columnIndex != COLUMNS; ++columnIndex) {
      auto& column = config[columnIndex];
      auto fee = Beam::Extract<T>(column);
      (*table)[columnIndex] = fee;
    }
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param name The name of the fee table to parse.
    \param table The table to store the fees in.
  */
  template<typename T, std::size_t COLUMNS>
  void ParseFeeTable(const YAML::Node& config, const std::string& name,
      Beam::Out<std::array<T, COLUMNS>> table) {
    auto node = config.FindValue(name);
    if(node == nullptr) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error{"Fee table \"" + name + "\" not found."});
    }
    ParseFeeTable(*node, Beam::Store(table));
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param table The table to store the fees in.
  */
  template<typename T, std::size_t ROWS, std::size_t COLUMNS>
  void ParseFeeTable(const YAML::Node& config,
      Beam::Out<std::array<std::array<T, COLUMNS>, ROWS>> table) {
    if(config.size() != ROWS) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Row size is invalid."});
    }
    for(auto rowIndex = 0; rowIndex != ROWS; ++rowIndex) {
      auto& row = config[rowIndex];
      if(row.size() != COLUMNS) {
        BOOST_THROW_EXCEPTION(std::runtime_error{"Column size is invalid."});
      }
      for(auto columnIndex = 0; columnIndex != COLUMNS; ++columnIndex) {
        auto& column = row[columnIndex];
        auto fee = Beam::Extract<T>(column);
        (*table)[rowIndex][columnIndex] = fee;
      }
    }
  }

  //! Parses a table of fees from a YAML Node.
  /*!
    \param config The YAML Node to parse the fees from.
    \param name The name of the fee table to parse.
    \param table The table to store the fees in.
  */
  template<typename T, std::size_t ROWS, std::size_t COLUMNS>
  void ParseFeeTable(const YAML::Node& config, const std::string& name,
      Beam::Out<std::array<std::array<T, COLUMNS>, ROWS>> table) {
    auto node = config.FindValue(name);
    if(node == nullptr) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error{"Fee table \"" + name + "\" not found."});
    }
    ParseFeeTable(*node, Beam::Store(table));
  }
}

#endif
