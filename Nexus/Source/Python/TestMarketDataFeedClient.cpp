#include "Nexus/Python/MarketDataService.hpp"
#include <Beam/Python/Collections.hpp>
#include <Beam/Python/DateTime.hpp>
#include <Beam/Python/GilLock.hpp>
#include <Beam/Python/Queries.hpp>
#include <Beam/Python/QueueWriter.hpp>
#include <Beam/Python/ServiceResult.hpp>
#include <Beam/Python/SharedObject.hpp>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"
#include "Nexus/Python/ToPythonMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  using Client = ToPythonMarketDataFeedClient<TestMarketDataFeedClient>;
  using Operation = TestMarketDataFeedClient::Operation;

  template<typename O>
  auto export_operation(handle scope, const char* name) {
    return class_<O, std::shared_ptr<O>>(scope, name).
      def_readonly("result", &O::m_result);
  }

  std::unique_ptr<Client> make_client(
      std::shared_ptr<QueueWriter<object>> queue) {
    auto operations = SharedObject(cast(queue));
    return std::make_unique<Client>(callback<std::shared_ptr<Operation>>(
      [=] (const auto& operation) {
        auto lock = GilLock();
        std::visit([&] (auto& value) {
          using Value = std::remove_cvref_t<decltype(value)>;
          operations->cast<QueueWriter<object>&>().push(
            cast(std::shared_ptr<Value>(operation, &value)));
        }, *operation);
      }, [=] (const std::exception_ptr& error) {
        auto lock = GilLock();
        operations->cast<QueueWriter<object>&>().close(error);
      }));
  }
}

void Nexus::Python::export_test_market_data_feed_client(module& module) {
  export_service_result<void>(module, "VoidServiceResult");
  auto client = export_market_data_feed_client<Client>(
    module, "TestMarketDataFeedClient");
  client.def(init(&make_client), arg("operations"));
  using Add = TestMarketDataFeedClient::AddOperation;
  export_operation<Add>(client, "AddOperation").
    def_readonly("info", &Add::m_info);
  using Imbalance = TestMarketDataFeedClient::PublishOrderImbalanceOperation;
  export_operation<Imbalance>(client, "PublishOrderImbalanceOperation").
    def_readonly("imbalance", &Imbalance::m_imbalance);
  using Bbo = TestMarketDataFeedClient::PublishBboQuoteOperation;
  export_operation<Bbo>(client, "PublishBboQuoteOperation").
    def_readonly("quote", &Bbo::m_quote);
  using Book = TestMarketDataFeedClient::PublishBookQuoteOperation;
  export_operation<Book>(client, "PublishBookQuoteOperation").
    def_readonly("quote", &Book::m_quote);
  using Sale = TestMarketDataFeedClient::PublishTimeAndSaleOperation;
  export_operation<Sale>(client, "PublishTimeAndSaleOperation").
    def_readonly("time_and_sale", &Sale::m_time_and_sale);
  using Status = TestMarketDataFeedClient::PublishTickerStatusOperation;
  export_operation<Status>(client, "PublishTickerStatusOperation").
    def_readonly("status", &Status::m_status);
  using AddOrder = TestMarketDataFeedClient::AddOrderOperation;
  export_operation<AddOrder>(client, "AddOrderOperation").
    def_readonly("ticker", &AddOrder::m_ticker).
    def_readonly("venue", &AddOrder::m_venue).
    def_readonly("mpid", &AddOrder::m_mpid).
    def_readonly("is_primary_mpid", &AddOrder::m_is_primary_mpid).
    def_readonly("id", &AddOrder::m_id).
    def_readonly("side", &AddOrder::m_side).
    def_readonly("price", &AddOrder::m_price).
    def_readonly("size", &AddOrder::m_size).
    def_readonly("timestamp", &AddOrder::m_timestamp);
  using Size = TestMarketDataFeedClient::ModifyOrderSizeOperation;
  export_operation<Size>(client, "ModifyOrderSizeOperation").
    def_readonly("id", &Size::m_id).
    def_readonly("size", &Size::m_size).
    def_readonly("timestamp", &Size::m_timestamp);
  using Offset = TestMarketDataFeedClient::OffsetOrderSizeOperation;
  export_operation<Offset>(client, "OffsetOrderSizeOperation").
    def_readonly("id", &Offset::m_id).
    def_readonly("delta", &Offset::m_delta).
    def_readonly("timestamp", &Offset::m_timestamp);
  using Price = TestMarketDataFeedClient::ModifyOrderPriceOperation;
  export_operation<Price>(client, "ModifyOrderPriceOperation").
    def_readonly("id", &Price::m_id).
    def_readonly("price", &Price::m_price).
    def_readonly("timestamp", &Price::m_timestamp);
  using Remove = TestMarketDataFeedClient::RemoveOrderOperation;
  export_operation<Remove>(client, "RemoveOrderOperation").
    def_readonly("id", &Remove::m_id).
    def_readonly("timestamp", &Remove::m_timestamp);
}
