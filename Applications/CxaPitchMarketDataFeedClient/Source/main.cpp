#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchConfiguration.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSequencer.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel =
    WrapperChannel<std::unique_ptr<MulticastSocketChannel>,
      QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient =
    CxaPitchProtocolClient<std::unique_ptr<ApplicationFeedChannel>>;
  static const auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(134217728);

  std::unique_ptr<ApplicationProtocolClient> make_protocol_client(
      const CxaPitchFeed& feed, const MulticastSocketOptions& options) {
    auto channel = try_or_nest([&] {
      return std::make_unique<MulticastSocketChannel>(
        feed.m_address, feed.m_interface, options);
    }, std::runtime_error("Unable to join the CXA PITCH multicast group."));
    auto reader = &channel->get_reader();
    return std::make_unique<ApplicationProtocolClient>(
      std::make_unique<ApplicationFeedChannel>(std::move(channel), reader));
  }

  void log(const CxaPitchMessage& message) {
    if(message.m_type == CxaPitchAddOrder::TYPE) {
      auto add_order = CxaPitchAddOrder::parse(message);
      std::cout << add_order.m_timestamp << ",add_order," <<
        add_order.m_order_id << ',' << add_order.m_side << ',' <<
        add_order.m_quantity << ',' << add_order.m_symbol << ',' <<
        add_order.m_price << ',' << add_order.m_pid << std::endl;
    } else if(message.m_type == CxaPitchOrderExecuted::TYPE) {
      auto executed = CxaPitchOrderExecuted::parse(message);
      std::cout << executed.m_timestamp << ",order_executed," <<
        executed.m_order_id << ',' << executed.m_executed_quantity << ',' <<
        executed.m_execution_id << ',' << executed.m_contra_order_id << ',' <<
        executed.m_contra_pid << std::endl;
    } else if(message.m_type == CxaPitchOrderExecutedAtPrice::TYPE) {
      auto executed = CxaPitchOrderExecutedAtPrice::parse(message);
      std::cout << executed.m_timestamp << ",order_executed_at_price," <<
        executed.m_order_id << ',' << executed.m_executed_quantity << ',' <<
        executed.m_execution_id << ',' << executed.m_contra_order_id << ',' <<
        executed.m_contra_pid << ',' << executed.m_execution_type << ',' <<
        executed.m_price << std::endl;
    } else if(message.m_type == CxaPitchReduceSize::TYPE) {
      auto reduce_size = CxaPitchReduceSize::parse(message);
      std::cout << reduce_size.m_timestamp << ",reduce_size," <<
        reduce_size.m_order_id << ',' << reduce_size.m_cancelled_quantity <<
        std::endl;
    } else if(message.m_type == CxaPitchModifyOrder::TYPE) {
      auto modify_order = CxaPitchModifyOrder::parse(message);
      std::cout << modify_order.m_timestamp << ",modify_order," <<
        modify_order.m_order_id << ',' << modify_order.m_quantity << ',' <<
        modify_order.m_price << std::endl;
    } else if(message.m_type == CxaPitchDeleteOrder::TYPE) {
      auto delete_order = CxaPitchDeleteOrder::parse(message);
      std::cout << delete_order.m_timestamp << ",delete_order," <<
        delete_order.m_order_id << std::endl;
    } else if(message.m_type == CxaPitchTrade::TYPE) {
      auto trade = CxaPitchTrade::parse(message);
      std::cout << trade.m_timestamp << ",trade," << trade.m_symbol << ',' <<
        trade.m_quantity << ',' << trade.m_price << ',' <<
        trade.m_execution_id << ',' << trade.m_order_id << ',' <<
        trade.m_contra_order_id << ',' << trade.m_pid << ',' <<
        trade.m_contra_pid << ',' << trade.m_trade_type << ',' <<
        trade.m_trade_designation << ',' << trade.m_trade_report_type <<
        std::endl;
    } else if(message.m_type == CxaPitchTradeBreak::TYPE) {
      auto trade_break = CxaPitchTradeBreak::parse(message);
      std::cout << trade_break.m_timestamp << ",trade_break," <<
        trade_break.m_execution_id << std::endl;
    } else if(message.m_type == CxaPitchTradingStatus::TYPE) {
      auto status = CxaPitchTradingStatus::parse(message);
      std::cout << status.m_timestamp << ",trading_status," <<
        status.m_symbol << ',' << status.m_status << ',' <<
        status.m_market_id_code << std::endl;
    } else if(message.m_type == CxaPitchCalculatedValue::TYPE) {
      auto value = CxaPitchCalculatedValue::parse(message);
      std::cout << value.m_timestamp << ",calculated_value," <<
        value.m_symbol << ',' << value.m_category << ',' << value.m_value <<
        ',' << value.m_value_timestamp << std::endl;
    } else if(message.m_type == CxaPitchAuctionUpdate::TYPE) {
      auto update = CxaPitchAuctionUpdate::parse(message);
      std::cout << update.m_timestamp << ",auction_update," <<
        update.m_symbol << ',' << update.m_auction_type << ',' <<
        update.m_buy_shares << ',' << update.m_sell_shares << ',' <<
        update.m_indicative_price << std::endl;
    } else if(message.m_type == CxaPitchAuctionSummary::TYPE) {
      auto summary = CxaPitchAuctionSummary::parse(message);
      std::cout << summary.m_timestamp << ",auction_summary," <<
        summary.m_symbol << ',' << summary.m_auction_type << ',' <<
        summary.m_price << ',' << summary.m_shares << std::endl;
    } else if(message.m_type == CxaPitchUnitClear::TYPE) {
      std::cout << ",unit_clear" << std::endl;
    } else if(message.m_type == CxaPitchEndOfSession::TYPE) {
      std::cout << ",end_of_session" << std::endl;
    }
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" CXA_PITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto feed_configuration = CxaPitchConfiguration::parse(config);
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    auto clients = std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : feed_configuration.m_feeds) {
      clients.push_back(make_protocol_client(feed, options));
    }
    auto sequencer = Sync<CxaPitchSequencer>(
      static_cast<int>(clients.size()), feed_configuration.m_liveness);
    auto routines = RoutineHandlerGroup();
    auto reported_gap = std::uint32_t(0);
    auto gap_timestamp = microsec_clock::universal_time();
    auto is_running = std::atomic<bool>(true);
    for(auto i = std::size_t(0); i != clients.size(); ++i) {
      routines.spawn([&, i] {
        auto units = std::set<std::uint8_t>();
        while(true) {
          try {
            auto block = clients[i]->read();
            auto& header = block.get_header();
            if(feed_configuration.m_is_logging_messages &&
                units.insert(header.m_unit).second) {
              std::cout << feed_configuration.m_feeds[i].m_name << ",feed," <<
                static_cast<int>(header.m_unit) << ',' << header.m_sequence <<
                std::endl;
            }
            if(header.m_unit != feed_configuration.m_unit) {
              continue;
            }
            with(sequencer, [&] (auto& sequencer) {
              auto timestamp = microsec_clock::universal_time();
              sequencer.add(static_cast<int>(i), block, timestamp);
              while(auto message = sequencer.read()) {
                if(feed_configuration.m_is_logging_messages) {
                  log(*message);
                }
              }
              auto gap = sequencer.get_gap();
              if(!gap) {
                return;
              }
              if(gap->m_sequence != reported_gap) {
                reported_gap = gap->m_sequence;
                gap_timestamp = timestamp;
                if(feed_configuration.m_is_logging_messages) {
                  std::cout << ",gap," << gap->m_sequence << ',' <<
                    gap->m_count << std::endl;
                }
              } else if(timestamp - gap_timestamp >
                  feed_configuration.m_gap_timeout) {
                if(feed_configuration.m_is_logging_messages) {
                  std::cout << ",skip," << gap->m_sequence << ',' <<
                    gap->m_count << std::endl;
                }
                sequencer.reset(gap->m_sequence + gap->m_count);
                reported_gap = 0;
                while(auto message = sequencer.read()) {
                  if(feed_configuration.m_is_logging_messages) {
                    log(*message);
                  }
                }
              }
            });
          } catch(const std::exception& e) {
            if(feed_configuration.m_is_logging_messages) {
              std::cout << feed_configuration.m_feeds[i].m_name <<
                ",error," << e.what() << std::endl;
            }
            if(!is_running) {
              break;
            }
          }
        }
      });
    }
    wait_for_kill_event();
    is_running = false;
    for(auto& client : clients) {
      client->close();
    }
    routines.wait();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
