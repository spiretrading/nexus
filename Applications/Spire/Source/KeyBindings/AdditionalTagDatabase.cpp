#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
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
    auto schema = std::make_shared<EnumAdditionalTagSchema>(
      "ExecInst", 18, std::move(cases));
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

AdditionalTagDatabase::AdditionalTagDatabase()
  : m_schemas(
      std::unordered_map<int, std::shared_ptr<AdditionalTagSchema>>()) {}

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
  return NONE;
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

std::vector<std::shared_ptr<AdditionalTagSchema>>
    AdditionalTagDatabase::find(const Destination& destination) const {
  return {};
}

const AdditionalTagDatabase& Spire::get_default_additional_tag_database() {
  static auto database = [] {
    auto database = AdditionalTagDatabase();
    database.add(Region::Global(), MaxFloorSchema::get_instance());
    database.add(Region::Global(), std::make_shared<BasicAdditionalTagSchema>(
      "PegDifference", 211, MoneyType::GetInstance()));
    database.add(ASX(), make_asx_exec_inst_schema());
    return database;
  }();
  return database;
}

const std::shared_ptr<AdditionalTagSchema>& Spire::find(
    const AdditionalTagDatabase& database, const Destination& destination,
    const Region& region, int key) {
  if(auto& schema = database.find(destination, key)) {
    return schema;
  }
  return database.find(region, key);
}

std::vector<std::shared_ptr<AdditionalTagSchema>> Spire::find(
    const AdditionalTagDatabase& database, const Destination& destination,
    const Region& region) {
  return database.find(region);
}
