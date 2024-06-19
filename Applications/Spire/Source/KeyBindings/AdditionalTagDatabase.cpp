#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/KeyBindings/BasicAdditionalTagSchema.hpp"
#include "Spire/KeyBindings/EnumAdditionalTagSchema.hpp"
#include "Spire/KeyBindings/MaxFloorSchema.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto NONE = std::shared_ptr<AdditionalTagSchema>();

  auto make_asx_exec_inst_schema() {
    auto cases = std::vector<std::string>{"A", "M", "R", "P"};
    std::sort(cases.begin(), cases.end());
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_chix_ex_destination_schema() {
    auto cases = std::vector<std::string>{"CXD", "SMRTCHIX", "SMRTCHIXD",
      "SMRTDARKNR", "SMRTDARK", "SMRTX", "SMRTXD", "SMRTXDARKNR", "SMRTXDARK",
      "SWEEPANDCROSS", "SWEEPANDPOST", "DEPTHFINDER", "SMRTFEE", "MULTI-CA",
      "MULTI-CXA", "MULTI-CX", "MULTI-CXY", "MULTIDARK-CM", "MULTIDARK-YM",
      "MULTIDARK-YCM", "MULTIDARK-CYXM", "MULTIDARK-DM"};
    std::sort(cases.begin(), cases.end());
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExDestination", 100, std::move(cases));
    return schema;
  }

  auto make_chix_exec_inst_schema() {
    auto cases = std::vector<std::string>{"M", "R", "P", "x", "f"};
    std::sort(cases.begin(), cases.end());
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_cx2_ex_destination_schema() {
    auto cases = std::vector<std::string>{
      "SMRTCX2", "SMRTCX2D", "SMRTCX2DARKNR", "SMRTCX2DARK"};
    std::sort(cases.begin(), cases.end());
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExDestination", 100, std::move(cases));
    return schema;
  }

  auto make_cx2_exec_inst_schema() {
    auto cases = std::vector<std::string>{"M", "R", "P", "x", "f"};
    std::sort(cases.begin(), cases.end());
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_tsx_long_life_schema() {
    auto cases = std::vector<std::string>{"Y", "N"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "TSXLongLife", 7735, std::move(cases));
    return schema;
  }

  auto make_cse_exec_inst_schema() {
    auto cases = std::vector<std::string>{"M", "P", "R", "9", "0"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_matn_exec_inst_schema() {
    auto cases = std::vector<std::string>{"M", "N", "R", "P", "p", "b"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_matn_anonymous_schema() {
    auto cases = std::vector<std::string>{"Y", "N"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "Anonymous", 6761, std::move(cases));
    return schema;
  }

  auto make_neoe_ex_destination_schema() {
    auto cases = std::vector<std::string>{"L", "N"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExDestination", 100, std::move(cases));
    return schema;
  }

  auto make_neoe_exec_inst_schema() {
    auto cases = std::vector<std::string>{"M", "i", "6", "100", "x"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
    return schema;
  }

  auto make_neoe_handl_inst_schema() {
    auto cases = std::vector<std::string>{"5", "6"};
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "HandlInst", 21, std::move(cases));
    return schema;
  }

  const auto& ASX() {
    static const auto REGION = [&] {
      auto region = Region();
      region +=
        Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::ASX()));
      region +=
        Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::CXA()));
      return region;
    }();
    return REGION;
  }
}

AdditionalTagDatabase::AdditionalTagDatabase(
  MarketDatabase markets, DestinationDatabase destinations)
  : m_markets(std::move(markets)),
    m_destinations(std::move(destinations)),
    m_schemas(
      std::unordered_map<int, std::shared_ptr<AdditionalTagSchema>>()) {}

void AdditionalTagDatabase::add(const Destination& destination,
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  m_destination_schemas[destination][schema->get_key()] = schema;
}

void AdditionalTagDatabase::add(const Region& region,
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  auto i = m_schemas.Find(region);
  if(std::get<0>(*i) == region) {
    std::get<1>(*i)[schema->get_key()] = schema;
  } else {
    m_schemas.Set(region, {});
    add(region, schema);
  }
}

const std::shared_ptr<AdditionalTagSchema>&
    AdditionalTagDatabase::find(const Destination& destination, int key) const {
  auto i = m_destination_schemas.find(destination);
  if(i != m_destination_schemas.end()) {
    auto j = std::get<1>(*i).find(key);
    if(j != std::get<1>(*i).end()) {
      return j->second;
    }
  }
  auto region = Region();
  for(auto& market : m_destinations.FromId(destination).m_markets) {
    region += Region(m_markets.FromCode(market));
  }
  return find(region, key);
}

const std::shared_ptr<AdditionalTagSchema>&
    AdditionalTagDatabase::find(const Region& region, int key) const {
  auto match = &NONE;
  for(auto i = m_schemas.Begin(); i != m_schemas.End(); ++i) {
    if(region <= std::get<0>(*i)) {
      auto j = std::get<1>(*i).find(key);
      if(j != std::get<1>(*i).end()) {
        match = &j->second;
      }
    }
  }
  return *match;
}

std::vector<std::shared_ptr<AdditionalTagSchema>>
    AdditionalTagDatabase::find(const Destination& destination) const {
  auto matches = std::vector<std::shared_ptr<AdditionalTagSchema>>();
  auto i = m_destination_schemas.find(destination);
  if(i != m_destination_schemas.end()) {
    for(auto& j : i->second) {
      for(auto& schema : std::get<1>(*i)) {
        auto k = std::find_if(matches.begin(), matches.end(),
          [&] (const auto& match) {
            return match->get_key() == schema.second->get_key();
          });
        if(k == matches.end()) {
          matches.push_back(schema.second);
        }
      }
    }
  }
  auto region = Region();
  for(auto& market : m_destinations.FromId(destination).m_markets) {
    region += Region(m_markets.FromCode(market));
  }
  auto parent_matches = find(region);
  for(auto& match : parent_matches) {
    matches.push_back(match);
  }
  return matches;
}

std::vector<std::shared_ptr<AdditionalTagSchema>>
    AdditionalTagDatabase::find(const Region& region) const {
  auto matches = std::vector<std::shared_ptr<AdditionalTagSchema>>();
  for(auto i = m_schemas.Begin(); i != m_schemas.End(); ++i) {
    if(region <= std::get<0>(*i)) {
      for(auto& schema : std::get<1>(*i)) {
        auto j = std::find_if(matches.begin(), matches.end(),
          [&] (const auto& match) {
            return match->get_key() == schema.second->get_key();
          });
        if(j == matches.end()) {
          matches.push_back(schema.second);
        }
      }
    }
  }
  return matches;
}

const AdditionalTagDatabase& Spire::get_default_additional_tag_database() {
  static auto database = [] {
    auto database = AdditionalTagDatabase(
      GetDefaultMarketDatabase(), GetDefaultDestinationDatabase());
    database.add(Region::Global(), MaxFloorSchema::get_instance());
    database.add(Region::Global(), std::make_shared<BasicAdditionalTagSchema>(
      "PegDifference", 211, MoneyType::GetInstance()));
    database.add(ASX(), make_asx_exec_inst_schema());
    database.add(
      DefaultDestinations::CHIX(), make_chix_ex_destination_schema());
    database.add(DefaultDestinations::CHIX(), make_chix_exec_inst_schema());
    database.add(DefaultDestinations::CHIX(), make_tsx_long_life_schema());
    database.add(DefaultDestinations::CSE(), make_cse_exec_inst_schema());
    database.add(DefaultDestinations::CSE2(), make_cse_exec_inst_schema());
    database.add(DefaultDestinations::CX2(), make_cx2_ex_destination_schema());
    database.add(DefaultDestinations::CX2(), make_cx2_exec_inst_schema());
    database.add(DefaultDestinations::CX2(), make_tsx_long_life_schema());
    database.add(DefaultDestinations::MATNLP(), make_matn_anonymous_schema());
    database.add(DefaultDestinations::MATNLP(), make_matn_exec_inst_schema());
    database.add(DefaultDestinations::MATNMF(), make_matn_anonymous_schema());
    database.add(DefaultDestinations::MATNMF(), make_matn_exec_inst_schema());
    database.add(
      DefaultDestinations::NEOE(), make_neoe_ex_destination_schema());
    database.add(DefaultDestinations::NEOE(), make_neoe_exec_inst_schema());
    database.add(
      DefaultDestinations::NEOE(), make_neoe_handl_inst_schema());
    database.add(DefaultDestinations::TSX(), make_tsx_long_life_schema());
    return database;
  }();
  return database;
}

const std::shared_ptr<AdditionalTagSchema>& Spire::find(
    const AdditionalTagDatabase& database, const Destination& destination,
    const Region& region, int key) {
  if(!destination.empty()) {
    if(auto& schema = database.find(destination, key)) {
      return schema;
    }
  }
  return database.find(region, key);
}

std::vector<std::shared_ptr<AdditionalTagSchema>> Spire::find(
    const AdditionalTagDatabase& database, const Destination& destination,
    const Region& region) {
  if(destination.empty()) {
    return database.find(region);
  }
  return database.find(destination);
}
