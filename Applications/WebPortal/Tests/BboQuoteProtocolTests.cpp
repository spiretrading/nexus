#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BboQuoteProtocol") {
  TEST_CASE("typescript_current_query") {
    auto buffer = from<SharedBuffer>(R"({
      "index":{"symbol":"TST","venue":"XTSE"},
      "range":{
        "start":{"which":0,"value":"0"},
        "end":{"which":0,"value":"18446744073709551615"}},
      "snapshot_limit":{"type":1,"size":1},
      "interruption_policy":2,
      "filter":{"expression":{
        "__type":"Beam.Queries.ConstantExpression",
        "value":{"value":{
          "__type":"Beam.Queries.BoolValue","value":true}}}}
    })");
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    Nexus::register_query_types(out(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.set(Ref(buffer));
    auto query = TickerQuery();
    receiver.shuttle(query);
    REQUIRE(query.get_index() == parse_ticker("TST.TSX"));
    REQUIRE(query.get_range() == Range::TOTAL);
    REQUIRE(query.get_snapshot_limit() == SnapshotLimit::from_tail(1));
    REQUIRE(query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
    REQUIRE(query.get_filter().as<ConstantExpression>().get_value().as<bool>());
  }

  TEST_CASE("sequenced_snapshot_and_update") {
    auto quote = BboQuote(make_bid(Money(10), 100), make_ask(Money(11), 200),
      time_from_string("2026-11-08 10:00:00"));
    auto sequence = Sequence(9007199254740993ULL);
    auto ticker = parse_ticker("TST.TSX");
    auto snapshot = BboQuoteQueryResult(7,
      std::vector{SequencedBboQuote(quote, sequence)});
    auto json = std::get<JsonObject>(parse<JsonValue>(to_json(snapshot)));
    REQUIRE(json.at("id") == 7);
    auto& entries = std::get<std::vector<JsonValue>>(json.at("snapshot"));
    REQUIRE(entries.size() == 1);
    auto& entry = std::get<JsonObject>(entries.front());
    REQUIRE(entry.at("sequence") == "9007199254740993");
    auto& value = std::get<JsonObject>(entry.at("value"));
    REQUIRE(value.at("timestamp") == "20261108T100000");
    REQUIRE(std::get<JsonObject>(value.at("bid")).at("price") == 10000000);
    auto update = BboQuoteMessage(
      SequencedTickerBboQuote(TickerBboQuote(quote, ticker), sequence));
    json = std::get<JsonObject>(parse<JsonValue>(to_json(update)));
    auto& update_entry = std::get<JsonObject>(json.at("bbo_quote"));
    REQUIRE(update_entry.at("sequence") == "9007199254740993");
    auto& indexed = std::get<JsonObject>(update_entry.at("value"));
    auto& index = std::get<JsonObject>(indexed.at("index"));
    REQUIRE(index.at("symbol") == "TST");
    REQUIRE(index.at("venue") == "XTSE");
    REQUIRE(std::get<JsonObject>(indexed.at("value")).at("timestamp") ==
      "20261108T100000");
  }
}
