#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include <quickfix/Session.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Beam;
using namespace Nexus;

namespace {
  const auto ANONYMOUS_TAG = 6761;
  const auto MATN_CONSTRAINTS_TAG = 6005;
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto LONG_LIFE_TAG = 7735;
  const auto UNIFORM_LIQUDITY_TAG = 9730;
  const auto ORIGINAL_LIQUIDITY_TAG = 9731;
  const auto NEO_VISIBILITY_TYPE_TAG = 20000;

  void populate_anonymous(
      const Tag& tag, FIX42::NewOrderSingle& new_order_single) {
    if(auto value = get<std::string>(&tag.get_value())) {
      if(*value == "Y" || *value == "N") {
        new_order_single.setField(ANONYMOUS_TAG, *value);
        return;
      }
    }
    throw_with_location(
      FixOrderRejectedException("Invalid value for tag 6761 (Anonymous)."));
  }

  void populate_long_life(
      const Tag& tag, FIX42::NewOrderSingle& new_order_single) {
    if(auto value = get<std::string>(&tag.get_value())) {
      if(*value == "Y" || *value == "N") {
        new_order_single.setField(LONG_LIFE_TAG, *value);
        return;
      }
    }
    throw_with_location(
      FixOrderRejectedException("Invalid value for tag 7735 (TsxLongLife)."));
  }

  void populate_exec_inst(
      const Tag& tag, FIX42::NewOrderSingle& new_order_single,
      std::initializer_list<const char*> cases) {
    if(auto value = get<std::string>(&tag.get_value())) {
      if(std::find(cases.begin(), cases.end(), *value) != cases.end()) {
        new_order_single.setField(FIX::ExecInst(*value));
        return;
      }
    }
    throw_with_location(
      FixOrderRejectedException("Invalid value for tag 18 (ExecInst)."));
  }
}

SerenityFixApplication::SerenityFixApplication(
  Ref<LiveNtpTimeClient> time_client,
  Ref<ApplicationMarketDataClient> market_data_client)
  : m_time_client(time_client.get()),
    m_market_data_client(market_data_client.get()) {}

std::shared_ptr<Order> SerenityFixApplication::recover(
    const SequencedAccountOrderRecord& record) {
  return m_order_log.recover(record);
}

std::shared_ptr<Order> SerenityFixApplication::submit(const OrderInfo& info) {
  auto modified_info = std::optional<OrderInfo>();
  auto submission_info = [&] () -> const OrderInfo* {
    if(info.m_fields.m_type != OrderType::MARKET &&
        info.m_fields.m_type != OrderType::PEGGED) {
      return &info;
    }
    if(info.m_fields.m_type == OrderType::MARKET &&
        (info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::OPG ||
        info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::MOC)) {
      return &info;
    }
    modified_info.emplace(info);
    if(info.m_fields.m_type == OrderType::MARKET) {
      modified_info->m_fields.m_type = OrderType::LIMIT;
      modified_info->m_fields.m_price = Money::ZERO;
    }
    auto bboQuote = load_bbo_quote(modified_info->m_fields.m_security);
    if(modified_info->m_fields.m_price == Money::ZERO) {
      if(info.m_fields.m_side == Side::BID) {
        modified_info->m_fields.m_price =
          std::max(bboQuote.m_ask.m_price + 2 * Money::CENT,
            floor_to(1.02 * bboQuote.m_ask.m_price, Money::CENT));
      } else {
        modified_info->m_fields.m_price =
          std::max(std::min(bboQuote.m_bid.m_price - 2 * Money::CENT,
            floor_to(0.98 * bboQuote.m_bid.m_price, Money::CENT)),
            Money::CENT / 2);
      }
    }
    return &*modified_info;
  }();
  if(modified_info->m_fields.m_security.get_venue() == DefaultVenues::OTCM) {
    return submit_to_us(*submission_info);
  } else {
    return submit_to_ca(*submission_info);
  }
}

void SerenityFixApplication::cancel(
    const OrderExecutionSession& session, OrderId id) {
  if(!m_cancellations.insert(id)) {
    return;
  }
  m_order_log.cancel(session, id, m_time_client->get_time(),
    get_session_id().getSenderCompID(), get_session_id().getTargetCompID(),
    [&] (const std::shared_ptr<Order>& order,
        Out<FIX42::OrderCancelRequest> request) {
      if(order->get_info().m_fields.m_security.get_venue() !=
          DefaultVenues::OTCM) {
        request->setField(UMIR_USER_ID_TAG, get_umir_user_id());
      }
    });
}

void SerenityFixApplication::update(const OrderExecutionSession& session,
    OrderId id, const ExecutionReport& report) {
  m_order_log.update(session, id, report, m_time_client->get_time());
}

void SerenityFixApplication::onCreate(const FIX::SessionID&) {}

void SerenityFixApplication::onLogon(const FIX::SessionID&) {}

void SerenityFixApplication::onLogout(const FIX::SessionID&) {}

void SerenityFixApplication::toAdmin(FIX::Message&, const FIX::SessionID&) {}

void SerenityFixApplication::toApp(FIX::Message&, const FIX::SessionID&) {}

void SerenityFixApplication::fromAdmin(
  const FIX::Message&, const FIX::SessionID&) {}

void SerenityFixApplication::fromApp(
    const FIX::Message& message, const FIX::SessionID& session_id) {
  crack(message, session_id);
}

void SerenityFixApplication::onMessage(
    const FIX42::ExecutionReport& message, const FIX::SessionID& session_id) {
  m_order_log.update(message, session_id, m_time_client->get_time(),
    [=] (const std::shared_ptr<Order>& order, Out<ExecutionReport> update) {
      auto liquidity_flag = std::string();
      if(message.isSetField(UNIFORM_LIQUDITY_TAG)) {
        liquidity_flag = message.getField(UNIFORM_LIQUDITY_TAG);
      }
      if(liquidity_flag == "A") {
        update->m_liquidity_flag = "P";
      } else if(liquidity_flag == "R") {
        update->m_liquidity_flag = "A";
      }
      auto original_liquidity_flag = std::string();
      if(message.isSetField(ORIGINAL_LIQUIDITY_TAG)) {
        original_liquidity_flag = message.getField(ORIGINAL_LIQUIDITY_TAG);
      }
      auto last_mkt = FIX::LastMkt();
      if(message.isSet(last_mkt)) {
        message.get(last_mkt);
        if(last_mkt == "XTSX") {
          update->m_last_market = DefaultVenues::TSX.get_code().get_data();
        } else if(last_mkt == "CHIX") {
          update->m_last_market = DefaultVenues::CHIC.get_code().get_data();
        } else if(last_mkt == "XCXD") {
          if(!original_liquidity_flag.empty()) {
            update->m_liquidity_flag = original_liquidity_flag;
          }
          update->m_last_market = DefaultVenues::CHIC.get_code().get_data();
        } else if(last_mkt == "XCX2") {
          update->m_last_market = DefaultVenues::XCX2.get_code().get_data();
        } else if(last_mkt == "MATN") {
          update->m_last_market = DefaultVenues::MATN.get_code().get_data();
        } else if(last_mkt == "XCNQ") {
          if(order->get_info().m_fields.m_security.get_venue() ==
              DefaultVenues::CSE) {
            update->m_last_market = DefaultVenues::CSE.get_code().get_data();
          } else {
            update->m_last_market = DefaultVenues::PURE.get_code().get_data();
          }
        } else if(last_mkt == "CSE2") {
          update->m_last_market = DefaultVenues::CSE2.get_code().get_data();
        } else if(last_mkt == "XATS") {
          update->m_last_market = DefaultVenues::XATS.get_code().get_data();
        } else if(last_mkt == "OMGA") {
          update->m_last_market = DefaultVenues::OMGA.get_code().get_data();
        } else if(last_mkt == "LYNX") {
          update->m_last_market = DefaultVenues::LYNX.get_code().get_data();
        } else if(
            last_mkt == "NEON" || last_mkt == "NEOL" || last_mkt == "NEOD") {
          update->m_last_market = DefaultVenues::NEOE.get_code().get_data();
        } else if(last_mkt == "TSXV") {
          update->m_last_market = DefaultVenues::TSXV.get_code().get_data();
        }
      }
      if(update->m_last_market == DefaultVenues::TSXV.get_code() ||
          update->m_last_market == DefaultVenues::TSX.get_code()) {
        if(original_liquidity_flag == "O") {
          update->m_liquidity_flag = "O";
        } else if(original_liquidity_flag.size() >= 3) {
          auto subflag = original_liquidity_flag.substr(1, 2);
          if(subflag == "AO" || subflag == "AE") {
            update->m_liquidity_flag = subflag;
          }
        }
      } else if(update->m_last_market == DefaultVenues::PURE.get_code() ||
          update->m_last_market == DefaultVenues::CSE.get_code() ||
          update->m_last_market == DefaultVenues::CSE2.get_code()) {
        if(original_liquidity_flag == "TC") {
          update->m_liquidity_flag = "TC";
        }
      }
  });
}

void SerenityFixApplication::onMessage(
  const FIX42::TradingSessionStatus&, const FIX::SessionID&) {}

void SerenityFixApplication::onMessage(
  const FIX42::OrderCancelReject&, const FIX::SessionID&) {}

BboQuote SerenityFixApplication::load_bbo_quote(const Security& security) {
  auto bbo = m_bbo_quotes.get_or_insert(security, [&] {
    auto bbo = std::make_shared<StateQueue<BboQuote>>();
    query_real_time_with_snapshot(*m_market_data_client, security, bbo);
    return bbo;
  });
  try {
    return bbo->peek();
  } catch(const Beam::PipeBrokenException&) {
    m_bbo_quotes.erase(security);
    throw_with_location(FixOrderRejectedException("No BBO quote available."));
  }
}

std::shared_ptr<Order> SerenityFixApplication::submit_to_ca(
    const OrderInfo& info) {
  return m_order_log.submit(info,
    get_session_id().getSenderCompID(), get_session_id().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> new_order_single) {
      if(info.m_fields.m_currency != DefaultCurrencies::CAD) {
        throw_with_location(FixOrderRejectedException("Invalid currency."));
      }
      if(info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTC ||
          info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTD) {
        throw_with_location(
          FixOrderRejectedException("Invalid time in force."));
      }
      if(info.m_fields.m_type == OrderType::STOP) {
        throw_with_location(FixOrderRejectedException("Invalid order type."));
      }
      new_order_single->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      new_order_single->setField(UMIR_USER_ID_TAG, get_umir_user_id());
      auto no_trade_feat = get_no_trade_feat();
      auto no_trade_key = get_no_trade_key();
      if(!no_trade_feat.empty() && !no_trade_key.empty()) {
        new_order_single->setField(NO_TRADE_FEAT_TAG, no_trade_feat);
        new_order_single->setField(NO_TRADE_KEY_TAG, no_trade_key);
      }
      new_order_single->set(FIX::Account(info.m_submission_account.m_name));
      if(auto& anonymousTag = get_anonymous_tag()) {
        new_order_single->setField(ANONYMOUS_TAG, *anonymousTag);
      }
      for(auto& tag : info.m_fields.m_additional_fields) {
        if(tag.get_key() == LONG_LIFE_TAG) {
          if(auto value = get<std::string>(&tag.get_value())) {
            if(*value == "Y" || *value == "N") {
              new_order_single->setField(LONG_LIFE_TAG, *value);
            }
          }
        }
      }
      if(info.m_fields.m_destination == DefaultDestinations::CHIX ||
          info.m_fields.m_destination == DefaultDestinations::CX2) {
        route_to_chix(info, out(new_order_single));
      } else if(info.m_fields.m_destination == DefaultDestinations::CSE ||
          info.m_fields.m_destination == DefaultDestinations::PURE) {
        route_to_cse(info, out(new_order_single));
      } else if(info.m_fields.m_destination == DefaultDestinations::MATNLP ||
          info.m_fields.m_destination == DefaultDestinations::MATNMF) {
        route_to_matn(info, out(new_order_single));
      } else if(info.m_fields.m_destination == DefaultDestinations::NEOE) {
        route_to_neo(info, out(new_order_single));
      } else if(info.m_fields.m_destination == DefaultDestinations::TSX) {
        route_to_tsx(info, out(new_order_single));
      } else {
        auto ex_destination = [&] {
          if(info.m_fields.m_destination == DefaultDestinations::CSE2) {
            return FIX::ExDestination("CSE2");
          } else if(info.m_fields.m_destination == DefaultDestinations::ALPHA) {
            return FIX::ExDestination("XATS");
          } else if(info.m_fields.m_destination == DefaultDestinations::OMEGA) {
            return FIX::ExDestination("OMGA");
          } else if(info.m_fields.m_destination == DefaultDestinations::LYNX) {
            return FIX::ExDestination("LYNX");
          }
          BOOST_THROW_EXCEPTION(
            FixOrderRejectedException("Invalid destination."));
        }();
        new_order_single->set(FIX::HandlInst('6'));
        new_order_single->set(ex_destination);
      }
    });
}

std::shared_ptr<Order> SerenityFixApplication::submit_to_us(
    const OrderInfo& info) {
  return m_order_log.submit(info,
    get_session_id().getSenderCompID(), get_session_id().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> new_order_single) {
      if(info.m_shorting_flag) {
        throw_with_location(
          FixOrderRejectedException("Short sale not allowed."));
      }
      if(info.m_fields.m_currency != DefaultCurrencies::USD) {
        throw_with_location(FixOrderRejectedException("Invalid currency."));
      }
      if(info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTC ||
          info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTD) {
        throw_with_location(
          FixOrderRejectedException("Invalid time in force."));
      }
      if(info.m_fields.m_type == OrderType::STOP) {
        throw_with_location(FixOrderRejectedException("Invalid order type."));
      }
      new_order_single->set(FIX::Account(info.m_submission_account.m_name));
      auto ex_destination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::OTCM) {
          return FIX::ExDestination("US01");
        }
        BOOST_THROW_EXCEPTION(
          FixOrderRejectedException("Invalid destination."));
      }();
      new_order_single->set(ex_destination);
    });
}

void SerenityFixApplication::route_to_chix(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> new_order_single) {
  auto has_destination = false;
  for(auto& tag : info.m_fields.m_additional_fields) {
    if(tag.get_key() == LONG_LIFE_TAG) {
      populate_long_life(tag, *new_order_single);
    } else if(tag.get_key() == FIX::FIELD::ExDestination) {
      if(auto value = boost::get<std::string>(&tag.get_value())) {
        auto destination = [&] {
          if(*value == "SMRTCHIX") {
            return FIX::ExDestination("CX01");
          } else if(*value == "SMRTCHIXD") {
            return FIX::ExDestination("CX02");
          } else if(*value == "SMRTDARKNR") {
            return FIX::ExDestination("CX03");
          } else if(*value == "SMRTDARK") {
            return FIX::ExDestination("CX04");
          } else if(*value == "SMRTCX2") {
            return FIX::ExDestination("CX05");
          } else if(*value == "SMRTCX2D") {
            return FIX::ExDestination("CX06");
          } else if(*value == "SMRTCX2DARKNR") {
            return FIX::ExDestination("CX07");
          } else if(*value == "SMRTCX2DARK") {
            return FIX::ExDestination("CX08");
          } else if(*value == "SMRTX") {
            return FIX::ExDestination("CX09");
          } else if(*value == "SMRTXD") {
            return FIX::ExDestination("CX10");
          } else if(*value == "SMRTXDARKNR") {
            return FIX::ExDestination("CX11");
          } else if(*value == "SMRTXDARK") {
            return FIX::ExDestination("CX12");
          } else if(*value == "SWEEPANDCROSS") {
            return FIX::ExDestination("CX13");
          } else if(*value == "DEPTHFINDER") {
            return FIX::ExDestination("CX14");
          } else if(*value == "SMRTFEE") {
            return FIX::ExDestination("CX15");
          } else if(*value == "MULTI-CA") {
            return FIX::ExDestination("CX16");
          } else if(*value == "MULTI-CXA") {
            return FIX::ExDestination("CX17");
          } else if(*value == "MULTI-CX") {
            return FIX::ExDestination("CX18");
          } else if(*value == "MULTI-CXY") {
            return FIX::ExDestination("CX19");
          } else if(*value == "MULTIDARK-CM") {
            return FIX::ExDestination("CX20");
          } else if(*value == "MULTIDARK-YM") {
            return FIX::ExDestination("CX21");
          } else if(*value == "MULTIDARK-YCM") {
            return FIX::ExDestination("CX22");
          } else if(*value == "MULTIDARK-CYXM") {
            return FIX::ExDestination("CX23");
          } else if(*value == "MULTIDARK-DM") {
            return FIX::ExDestination("CX24");
          } else if(*value == "SMRTXOPG-X2") {
            return FIX::ExDestination("CX25");
          } else if(*value == "CXD") {
            return FIX::ExDestination("XCXD");
          } else if(*value == "SMRTCXD") {
            return FIX::ExDestination("XCXD");
          }
          throw_with_location(FixOrderRejectedException(
            "Invalid value for tag 100 (ExDestination)."));
        }();
        new_order_single->getHeader().setField(destination);
        if(*value == "SMRTXDARKNR") {
          new_order_single->setField(ANONYMOUS_TAG, "Y");
        }
        has_destination = true;
      } else {
        throw_with_location(FixOrderRejectedException(
          "Invalid value for tag 100 (ExDestination)."));
      }
    } else if(tag.get_key() == FIX::FIELD::ExecInst) {
      populate_exec_inst(tag, *new_order_single, {"M", "R", "P", "x", "f"});
    }
  }
  if(!has_destination) {
    if(info.m_fields.m_type == OrderType::PEGGED) {
      auto destination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::CHIX) {
          return FIX::ExDestination("CHIX");
        } else if(info.m_fields.m_destination == DefaultDestinations::CX2) {
          return FIX::ExDestination("XCX2");
        } else {
          throw_with_location(
            FixOrderRejectedException("Destination not supported."));
        }
      }();
      new_order_single->getHeader().setField(destination);
    } else {
      auto destination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::CHIX) {
          return FIX::ExDestination("CX01");
        } else if(info.m_fields.m_destination == DefaultDestinations::CX2) {
          return FIX::ExDestination("CX05");
        } else {
          throw_with_location(
            FixOrderRejectedException("Destination not supported."));
        }
      }();
      new_order_single->getHeader().setField(destination);
    }
  }
}

void SerenityFixApplication::route_to_cse(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> new_order_single) {
  new_order_single->setField(FIX::ExDestination("XCNQ"));
  new_order_single->set(FIX::HandlInst('6'));
  for(auto& tag : info.m_fields.m_additional_fields) {
    if(tag.get_key() == FIX::FIELD::ExecInst) {
      populate_exec_inst(tag, *new_order_single, {"M", "P", "R", "9", "0"});
    }
  }
}

void SerenityFixApplication::route_to_matn(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> new_order_single) {
  new_order_single->setField(FIX::ExDestination("MATN"));
  for(auto& tag : info.m_fields.m_additional_fields) {
    if(tag.get_key() == FIX::FIELD::ExecInst) {
      if(tag.get_value() == Tag::Type(std::string("p"))) {
        populate_exec_inst(
          Tag(FIX::FIELD::ExecInst, "x"), *new_order_single, {"x"});
      } else {
        populate_exec_inst(
          tag, *new_order_single, {"M", "N", "R", "P", "p", "b"});
      }
    } else if(tag.get_key() == ANONYMOUS_TAG) {
      populate_anonymous(tag, *new_order_single);
    } else if(tag.get_key() == MATN_CONSTRAINTS_TAG) {
      if(auto value = get<std::string>(&tag.get_value())) {
        if(*value == "PAG") {
          new_order_single->setField(FIX::ExecInst("R"));
          new_order_single->setField(FIX::OrdType(FIX::OrdType_PEGGED));
        } else if(*value == "PMI") {
          new_order_single->setField(FIX::ExecInst("x"));
          new_order_single->setField(FIX::OrdType(FIX::OrdType_PEGGED));
        }
      }
    }
  }
}

void SerenityFixApplication::route_to_neo(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> new_order_single) {
  auto is_protected = true;
  auto is_mid_point = false;
  auto is_neo_book = false;
  for(auto& tag : info.m_fields.m_additional_fields) {
    if(tag.get_key() == FIX::FIELD::ExecInst) {
      if(auto value = get<std::string>(&tag.get_value())) {
        if(*value == "M") {
          is_protected = false;
          is_mid_point = true;
        }
      } else {
        throw_with_location(FixOrderRejectedException(
          "Invalid value for tag 18 (ExecInst)."));
      }
    } else if(tag.get_key() == FIX::FIELD::ExDestination) {
      if(auto value = get<std::string>(&tag.get_value())) {
        auto destination = [&] {
          if(*value == "N") {
            return FIX::ExDestination("NEON");
          }
          throw_with_location(FixOrderRejectedException(
            "Invalid value for tag 100 (ExDestination)."));
        }();
        new_order_single->getHeader().setField(destination);
        if(*value == "N") {
          is_neo_book = true;
        }
        break;
      } else {
        throw_with_location(FixOrderRejectedException(
          "Invalid value for tag 100 (ExDestination)."));
      }
    }
  }
  if(!is_neo_book) {
    new_order_single->getHeader().setField(FIX::ExDestination("NEOL"));
  }
  if(is_protected) {
    new_order_single->set(FIX::HandlInst('5'));
  }
  if(is_mid_point) {
    if(!is_neo_book) {
      new_order_single->setField(NEO_VISIBILITY_TYPE_TAG, "2");
    }
  }
}

void SerenityFixApplication::route_to_tsx(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> new_order_single) {
  auto has_destination = false;
  for(auto& tag : info.m_fields.m_additional_fields) {
    if(tag.get_key() == LONG_LIFE_TAG) {
      populate_long_life(tag, *new_order_single);
    } else if(tag.get_key() == FIX::FIELD::ExDestination) {
      if(auto value = boost::get<std::string>(&tag.get_value())) {
        auto destination = [&] {
          if(*value == "SMRTXOPG-X2") {
            return FIX::ExDestination("CX25");
          }
          throw_with_location(FixOrderRejectedException(
            "Invalid value for tag 100 (ExDestination)."));
        }();
        has_destination = true;
        new_order_single->getHeader().setField(destination);
      }
    }
  }
  if(!has_destination) {
    auto destination = [&] {
      if(info.m_fields.m_security.get_venue() == DefaultVenues::TSXV) {
        return FIX::ExDestination("TSXV");
      }
      return FIX::ExDestination("XTSX");
    }();
    new_order_single->getHeader().setField(destination);
  }
  new_order_single->set(FIX::HandlInst('6'));
  if(info.m_fields.m_type == OrderType::PEGGED) {
    new_order_single->set(FIX::ExecInst("M"));
  }
}

const optional<std::string>& SerenityFixApplication::get_anonymous_tag() const {
  if(m_anonymous_tag) {
    return *m_anonymous_tag;
  }
  if(get_session_settings().get(get_session_id()).has("Anonymous")) {
    m_anonymous_tag.emplace(
      get_session_settings().get(get_session_id()).getString("Anonymous"));
  } else {
    m_anonymous_tag.emplace(none);
  }
  return *m_anonymous_tag;
}

std::string SerenityFixApplication::get_umir_user_id() const {
  if(get_session_settings().get(get_session_id()).has("UMIRUserID")) {
    return get_session_settings().get(get_session_id()).getString("UMIRUserID");
  }
  return {};
}

std::string SerenityFixApplication::get_no_trade_feat() const {
  if(get_session_settings().get(get_session_id()).has("NoTradeFeat")) {
    return get_session_settings().get(get_session_id()).getString("NoTradeFeat");
  }
  return {};
}

std::string SerenityFixApplication::get_no_trade_key() const {
  if(get_session_settings().get(get_session_id()).has("NoTradeKey")) {
    return get_session_settings().get(get_session_id()).getString("NoTradeKey");
  }
  return {};
}
