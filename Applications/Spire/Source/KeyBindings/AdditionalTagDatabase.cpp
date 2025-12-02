#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/KeyBindings/BasicAdditionalTagSchema.hpp"
#include "Spire/KeyBindings/EnumAdditionalTagSchema.hpp"
#include "Spire/KeyBindings/MaxFloorSchema.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto NONE = std::shared_ptr<AdditionalTagSchema>();

  void sort(std::vector<OrderFieldInfoTip::Model::Argument>& arguments) {
    std::sort(arguments.begin(), arguments.end(),
      [] (const auto& left, const auto& right) {
        return left.m_value < right.m_value;
      });
  }

  auto make_peg_difference_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "PegDifference";
    model.m_tag.m_description =
      "Amount (signed) added to the price of the peg for a pegged order.";
    auto schema  = std::make_shared<BasicAdditionalTagSchema>(
      std::move(model), 211, MoneyType::GetInstance());
    return schema;
  }

  auto make_asx_exec_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExecInst";
    model.m_tag.m_description =
      "Instructions for order handling on exchange trading floor.";
    model.m_tag.m_arguments.emplace_back("A", "No cross (cross is forbidden)");
    model.m_tag.m_arguments.emplace_back(
      "M", "Mid-price peg (midprice of inside quote)");
    model.m_tag.m_arguments.emplace_back(
      "R", "Primary peg (primary market - buy at bid/sell at offer)");
    model.m_tag.m_arguments.emplace_back("P", "Market peg");
    sort(model.m_tag.m_arguments);
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 18);
    return schema;
  }

  auto make_chix_ex_destination_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExDestination";
    model.m_tag.m_description = "Specifies the routing strategy to use.";
    model.m_tag.m_arguments.emplace_back("CXD", "Post to CHI-X Dark.");
    model.m_tag.m_arguments.emplace_back(
      "SMRTCHIX", "Spray all protected markets. Post on CHIC. No re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTCHIXD",
      "Spray all protected markets. Post on CHIC. Dynamic re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTDARKNR",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on CHIC. No re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTDARK",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on CHIC. Dynamic re-spray.");
    model.m_tag.m_arguments.emplace_back(
      "SMRTX", "Spray all protected markets. Post on TSX. No re-spray.");
    model.m_tag.m_arguments.emplace_back(
      "SMRTXD", "Spray all protected markets. Post on TSX. Dynamic re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTXDARKNR",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on TSX. No re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTXDARK",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on TSX. Dynamic re-spray.");
    model.m_tag.m_arguments.emplace_back("SWEEPANDCROSS",
      "Allows participants to sweep all protected quotes and print a cross "
      "order in one step by sending IOC bypass orders to all protected venues "
      "before printing the cross on CHIC.");
    model.m_tag.m_arguments.emplace_back("SWEEPANDPOST",
      "Aggresivley takes liquidity with a depth of book spray in a single pass "
      "and books any residual to CHIC.");
    model.m_tag.m_arguments.emplace_back("DEPTHFINDER",
      "Takes advantage of price improvement opportunities by taking liquidity "
      "with an iterative depth of book spray, capturing hidden portion of "
      "iceberg orders.  Any residual is posted to CHIC.");
    model.m_tag.m_arguments.emplace_back("SMRTFEE",
      "Spray all protected markets based on execution fees (CX2 & CHIC first). "
      "Post on CHIC.");
    model.m_tag.m_arguments.emplace_back("MULTI-CA",
      "CHIC Smart Router will attempt to split the shares evenly between "
      "CHIC & ALPHA");
    model.m_tag.m_arguments.emplace_back("MULTI-CXA",
      "CHIC Smart Router will attempt to split the shares evenly between CHIC, "
      "TSX & ALPHA.");
    model.m_tag.m_arguments.emplace_back("MULTI-CX",
      "Spray all protected markets. Split residual between CHIC and TSX. "
      "Dynamically rebalance.");
    model.m_tag.m_arguments.emplace_back("MULTI-CXY",
      "Spray all protected markets. Split residual between CHIC, CX2 and TSX. "
      "Dynamically rebalance.");
    model.m_tag.m_arguments.emplace_back("MULTIDARK-CM",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Split residual between CHIC "
      "& MATN mid-point.");
    model.m_tag.m_arguments.emplace_back("MULTIDARK-YM",
      "Ping CHI-X Dark. Ping CX2 & MATN mid-point. Split residual between CX2 "
      "& MatchNow Mid");
    model.m_tag.m_arguments.emplace_back("MULTIDARK-YCM",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Split residual between CHIC, "
      "CX2 & MATN mid-point.");
    model.m_tag.m_arguments.emplace_back("MULTIDARK-CYXM",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point & TSX mid-point. Split "
      "residual between CHIC, CX2, MATN & TSX mid-point.");
    model.m_tag.m_arguments.emplace_back("MULTIDARK-DM",
      "Ping CHI-X Dark/CX2/CHIC, MATN, TSX mid-point, split residual between "
      "CHI-X Dark and MATN.");
    sort(model.m_tag.m_arguments);
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 100);
    return schema;
  }

  auto make_chix_exec_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExecInst";
    model.m_tag.m_description =
      "Instructions for order handling on exchange trading floor.";
    model.m_tag.m_arguments.emplace_back(
      "M", "Mid-price peg (midprice of inside quote)");
    model.m_tag.m_arguments.emplace_back(
      "R", "Primary peg (primary market - buy at bid/sell at offer)");
    model.m_tag.m_arguments.emplace_back("P", "Market peg");
    model.m_tag.m_arguments.emplace_back(
      "x", "Minimum Price Improvement (CXD Only)");
    model.m_tag.m_arguments.emplace_back("f", "CSO (Not supported on CXD)");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 18);
    return schema;
  }

  auto make_cx2_ex_destination_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExDestination";
    model.m_tag.m_description = "Specifies the routing strategy to use.";
    model.m_tag.m_arguments.emplace_back("SMRTCX2",
      "Spray all protected markets. Post on CX2. No re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTCX2D",
      "Spray all protected markets. Post on CX2. Dynamic re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTCX2DARKNR",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on CX2. No re-spray.");
    model.m_tag.m_arguments.emplace_back("SMRTCX2DARK",
      "Ping CHI-X Dark/CX2/CHIC & MATN mid-point. Spray all protected markets. "
      "Post on TSX. Dynamic re-spray.");
    sort(model.m_tag.m_arguments);
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 100);
    return schema;
  }

  auto make_cx2_exec_inst_schema() {
    return make_chix_exec_inst_schema();
  }

  auto make_tsx_long_life_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "TSXLongLife";
    model.m_tag.m_description = "Commits an order to a minimum resting time of "
      "1 second in the order book, during which time the order cannot be "
      "modified or cancelled. In exchange the order will receive allocation "
      "priority of price/broker/long life/time.";
    model.m_tag.m_arguments.emplace_back("Y", "Submit as a long life order.");
    model.m_tag.m_arguments.emplace_back(
      "N", "Do not submit as a long life order (Default).");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 7735);
    return schema;
  }

  auto make_cse_exec_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExecInst";
    model.m_tag.m_description =
      "Instructions for order handling on exchange trading floor.";
    model.m_tag.m_arguments.emplace_back(
      "M", "Mid-price peg (midprice of inside quote)");
    model.m_tag.m_arguments.emplace_back(
      "R", "Primary peg (primary market - buy at bid/sell at offer)");
    model.m_tag.m_arguments.emplace_back("P", "Market peg");
    model.m_tag.m_arguments.emplace_back("9", "Post on bid");
    model.m_tag.m_arguments.emplace_back("0", "Post on offer");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 18);
    return schema;
  }

  auto make_matn_exec_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExecInst";
    model.m_tag.m_description =
      "Instructions for order handling on exchange trading floor.";
    model.m_tag.m_arguments.emplace_back(
      "M", "Trade at the PNBBO midpoint only.");
    model.m_tag.m_arguments.emplace_back(
      "N", "Trade at any eligible price (Default).");
    model.m_tag.m_arguments.emplace_back(
      "R", "Trade at the near-side PNBBO (Size and price restrictions apply).");
    model.m_tag.m_arguments.emplace_back("P", "Trade at the far-side PNBBO "
      "(Size and price restrictions apply)");
    model.m_tag.m_arguments.emplace_back("p", "Trade at the less aggressive of "
      "PNBBO midpoint or minimum improvement from the PNBBO.");
    model.m_tag.m_arguments.emplace_back(
      "B", "Trade at any eligible price within the PNBBO.");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 18);
    return schema;
  }

  auto make_matn_anonymous_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "Anonymous";
    model.m_tag.m_description = "Anonymize this order when trade reporting.";
    model.m_tag.m_arguments.emplace_back("Y", "Submit as anonymous.");
    model.m_tag.m_arguments.emplace_back(
      "N", "Do not submit as anonymous (Default).");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 6761);
    return schema;
  }

  auto make_neoe_ex_destination_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExDestination";
    model.m_tag.m_description = "Specifies the NEO book to route to.";
    model.m_tag.m_arguments.emplace_back("L", "Route to the lit book.");
    model.m_tag.m_arguments.emplace_back("N", "Route to the NEOE book.");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 100);
    return schema;
  }

  auto make_neoe_exec_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExecInst";
    model.m_tag.m_description =
      "Instructions for order handling on exchange trading floor.";
    model.m_tag.m_arguments.emplace_back("M", "Mid-point");
    model.m_tag.m_arguments.emplace_back("i", "Imbalance only");
    model.m_tag.m_arguments.emplace_back("6", "Resting only");
    model.m_tag.m_arguments.emplace_back(
      "100", "Re-price (resting orders only).");
    model.m_tag.m_arguments.emplace_back("x", "Minimum price improvement");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 18);
    return schema;
  }

  auto make_neoe_handl_inst_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "HandlInst";
    model.m_tag.m_description = "Handling instructions for the order.";
    model.m_tag.m_arguments.emplace_back("5", "Protect and cancel");
    model.m_tag.m_arguments.emplace_back("6", "Protect and reprice");
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 21);
    return schema;
  }

  auto make_tsx_ex_destination_schema() {
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name = "ExDestination";
    model.m_tag.m_description = "Specifies the routing strategy to use.";
    model.m_tag.m_arguments.emplace_back(
      "SMRTXOPG-X2", "Ping CX2 and CHIC before posting to TSX.");
    sort(model.m_tag.m_arguments);
    auto schema =
      std::make_shared<EnumAdditionalTagSchema>(std::move(model), 100);
    return schema;
  }

  const auto& ASX() {
    static const auto REGION = [&] {
      auto region = Region();
      region += DefaultVenues::ASX;
      region += DefaultVenues::CXA;
      return region;
    }();
    return REGION;
  }
}

AdditionalTagDatabase::AdditionalTagDatabase()
  : m_schemas(
      std::unordered_map<int, std::shared_ptr<AdditionalTagSchema>>()) {}

void AdditionalTagDatabase::add(const Destination& destination,
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  m_destination_schemas[destination][schema->get_key()] = schema;
}

void AdditionalTagDatabase::add(const Region& region,
    const std::shared_ptr<AdditionalTagSchema>& schema) {
  auto i = m_schemas.find(region);
  if(std::get<0>(*i) == region) {
    std::get<1>(*i)[schema->get_key()] = schema;
  } else {
    m_schemas.set(region, {});
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
  for(auto& venue : DEFAULT_DESTINATIONS.from(destination).m_venues) {
    region += venue;
  }
  return find(region, key);
}

const std::shared_ptr<AdditionalTagSchema>&
    AdditionalTagDatabase::find(const Region& region, int key) const {
  auto match = &NONE;
  for(auto i = m_schemas.begin(); i != m_schemas.end(); ++i) {
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
  for(auto& venue : DEFAULT_DESTINATIONS.from(destination).m_venues) {
    region += venue;
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
  for(auto i = m_schemas.begin(); i != m_schemas.end(); ++i) {
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
    auto database = AdditionalTagDatabase();
    database.add(Region::GLOBAL, MaxFloorSchema::get_instance());
    database.add(Region::GLOBAL, make_peg_difference_schema());
    database.add(DefaultVenues::ASX, make_asx_exec_inst_schema());
    database.add(DefaultDestinations::CHIX, make_chix_ex_destination_schema());
    database.add(DefaultDestinations::CHIX, make_chix_exec_inst_schema());
    database.add(DefaultDestinations::CHIX, make_tsx_long_life_schema());
    database.add(DefaultDestinations::CSE, make_cse_exec_inst_schema());
    database.add(DefaultDestinations::CSE2, make_cse_exec_inst_schema());
    database.add(DefaultDestinations::CX2, make_cx2_ex_destination_schema());
    database.add(DefaultDestinations::CX2, make_cx2_exec_inst_schema());
    database.add(DefaultDestinations::CX2, make_tsx_long_life_schema());
    database.add(DefaultDestinations::MATNLP, make_matn_anonymous_schema());
    database.add(DefaultDestinations::MATNLP, make_matn_exec_inst_schema());
    database.add(DefaultDestinations::MATNMF, make_matn_anonymous_schema());
    database.add(DefaultDestinations::MATNMF, make_matn_exec_inst_schema());
    database.add(DefaultDestinations::NEOE, make_neoe_ex_destination_schema());
    database.add(DefaultDestinations::NEOE, make_neoe_exec_inst_schema());
    database.add(DefaultDestinations::NEOE, make_neoe_handl_inst_schema());
    database.add(DefaultDestinations::TSX, make_tsx_ex_destination_schema());
    database.add(DefaultDestinations::TSX, make_tsx_long_life_schema());
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
