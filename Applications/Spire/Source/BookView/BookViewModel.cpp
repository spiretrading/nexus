#include "Spire/BookView/BookViewModel.hpp"
#ifdef slots
  #undef slots
#endif
#include <tuple>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/QuoteConversions.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace boost;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace Spire;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

BookViewModel::BookViewModel(RefType<UserProfile> userProfile,
    const BookViewProperties& properties, const Security& security, Side side)
    : m_userProfile(userProfile.Get()),
      m_properties(properties),
      m_security(security),
      m_side(side) {
  connect(&m_updateTimer, &QTimer::timeout, this,
    &BookViewModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  if(m_security == Security()) {
    return;
  }
  m_boardLot = m_userProfile->GetMarketDatabase().FromCode(
    m_security.GetMarket()).m_boardLot;
  QueryRealTimeBookQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(),
    m_security, m_slotHandler.GetSlot<BookQuote>(
    std::bind(&BookViewModel::OnBookQuote, this, std::placeholders::_1),
    std::bind(&BookViewModel::OnBookQuoteInterruption, this,
    std::placeholders::_1)), InterruptionPolicy::BREAK_QUERY);
  QueryRealTimeMarketQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(),
    m_security, m_slotHandler.GetSlot<MarketQuote>(
    std::bind(&BookViewModel::OnMarketQuote, this, std::placeholders::_1),
    std::bind(&BookViewModel::OnMarketQuoteInterruption, this,
    std::placeholders::_1)), InterruptionPolicy::BREAK_QUERY);
  m_userProfile->GetBlotterSettings().GetConsolidatedBlotter(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount()).
    GetTasksModel().GetOrderExecutionPublisher().Monitor(
    m_slotHandler.GetSlot<const Order*>(
    std::bind(&BookViewModel::OnOrderExecuted, this, std::placeholders::_1)));
}

BookViewModel::~BookViewModel() {}

void BookViewModel::SetProperties(const BookViewProperties& properties) {
  m_properties = properties;
  for(auto& orderQuantity : m_orderQuantities) {
    Quote quote(orderQuantity.first.m_price, 0, m_side);
    BookQuote bookQuote("@" + orderQuantity.first.m_destination, false,
      MarketCode(), quote,
      m_userProfile->GetServiceClients().GetTimeClient().GetTime());
    OnBookQuote(bookQuote);
  }
  if(m_properties.GetOrderHighlight() != BookViewProperties::HIDE_ORDERS) {
    for(auto& orderQuantity : m_orderQuantities) {
      Quote quote(orderQuantity.first.m_price, orderQuantity.second, m_side);
      BookQuote bookQuote("@" + orderQuantity.first.m_destination, false,
        MarketCode(), quote,
        m_userProfile->GetServiceClients().GetTimeClient().GetTime());
      OnBookQuote(bookQuote);
    }
  }
  if(!m_bookQuotes.empty()) {
    dataChanged(index(0, 0), index(static_cast<int>(m_bookQuotes.size()) - 1,
      COLUMN_COUNT - 1));
  }
}

int BookViewModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_bookQuotes.size());
}

int BookViewModel::columnCount(const QModelIndex& parent) const {
  return COLUMN_COUNT;
}

QVariant BookViewModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& quote = m_bookQuotes[index.row()];
  if(role == Qt::TextAlignmentRole) {
    if(index.column() == MPID_COLUMN) {
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    } else if(index.column() == SIZE_COLUMN || index.column() == PRICE_COLUMN) {
      return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::FontRole) {
    if(index.column() == MPID_COLUMN) {
      auto font = m_properties.GetBookQuoteFont();
      font.setBold(true);
      return font;
    }
  } else if(role == Qt::BackgroundRole) {
    if(!quote.m_mpid.empty() && quote.m_mpid[0] == '@' &&
        m_properties.GetOrderHighlight() ==
        BookViewProperties::HIGHLIGHT_ORDERS) {
      return m_properties.GetOrderHighlightColor();
    }
    auto highlight = m_properties.GetMarketHighlight(quote.m_market);
    if(highlight.is_initialized() && TestHighlight(*highlight, quote)) {
      return highlight->m_color;
    }
    auto level = m_quoteLevels[index.row()];
    if(level < static_cast<int>(
        m_properties.GetBookQuoteBackgroundColors().size())) {
      return m_properties.GetBookQuoteBackgroundColors()[level];
    } else if(!m_properties.GetBookQuoteBackgroundColors().empty()) {
      return m_properties.GetBookQuoteBackgroundColors().back();
    } else {
      return QVariant();
    }
  } else if(role == Qt::ForegroundRole) {
    return m_properties.GetBookQuoteForegroundColor();
  } else if(role == Qt::DisplayRole) {
    if(index.column() == PRICE_COLUMN) {
      return QVariant::fromValue(quote.m_quote.m_price);
    } else if(index.column() == SIZE_COLUMN) {
      return QVariant::fromValue(
        std::max<Quantity>(1, quote.m_quote.m_size / m_boardLot));
    } else if(index.column() == MPID_COLUMN) {
      return QString::fromStdString(quote.m_mpid);
    }
  }
  return QVariant();
}

QVariant BookViewModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == MPID_COLUMN) {
      return tr("MPID");
    } else if(section == PRICE_COLUMN) {
      return tr("Price");
    } else if(section == SIZE_COLUMN) {
      return tr("Size");
    }
  }
  return QVariant();
}

bool BookViewModel::TestHighlight(
    const BookViewProperties::MarketHighlight& highlight,
    const BookQuote& quote) const {
  if(highlight.m_highlightAllLevels) {
    return true;
  }
  auto topLevelIterator = m_topLevels.find(quote.m_market);
  if(topLevelIterator == m_topLevels.end()) {
    return false;
  }
  auto& topQuote = topLevelIterator->second;
  return topQuote.m_mpid == quote.m_mpid && topQuote.m_quote.m_price ==
    quote.m_quote.m_price;
}

void BookViewModel::HighlightQuote(const BookQuote& quote) {
  auto topQuoteIterator = m_topLevels.find(quote.m_market);
  if(topQuoteIterator == m_topLevels.end()) {
    if(quote.m_quote.m_size != 0) {
      m_topLevels.insert(make_pair(quote.m_market, quote));
    }
  } else {
    auto& topQuote = topQuoteIterator->second;
    if(quote.m_quote.m_size == 0 && quote.m_mpid == topQuote.m_mpid &&
        quote.m_quote.m_price == topQuote.m_quote.m_price) {
      auto newTopQuoteIndex = -1;
      auto previousTopQuoteIndex = -1;
      auto i = m_bookQuotes.begin();
      while(i != m_bookQuotes.end() && (newTopQuoteIndex == -1 ||
          previousTopQuoteIndex == -1)) {
        if(newTopQuoteIndex == -1 && i->m_market == quote.m_market &&
            *i != topQuote) {
          newTopQuoteIndex = static_cast<int>(std::distance(
            m_bookQuotes.begin(), i));
        }
        if(previousTopQuoteIndex == -1 && *i == topQuote) {
          previousTopQuoteIndex = static_cast<int>(std::distance(
            m_bookQuotes.begin(), i));
        }
        ++i;
      }
      if(newTopQuoteIndex != -1) {
        topQuote = m_bookQuotes[newTopQuoteIndex];
      } else {
        m_topLevels.erase(quote.m_market);
      }
    } else if(quote.m_quote.m_size != 0 &&
        (-GetDirection(m_side) * quote.m_quote.m_price <
        -GetDirection(m_side) * topQuote.m_quote.m_price ||
        quote.m_quote.m_price == topQuote.m_quote.m_price &&
        quote.m_isPrimaryMpid && !topQuote.m_isPrimaryMpid)) {
      auto bookQuoteIterator = std::find(m_bookQuotes.begin(),
        m_bookQuotes.end(), topQuote);
      auto topQuoteIndex = static_cast<int>(std::distance(m_bookQuotes.begin(),
        bookQuoteIterator));
      topQuote = quote;
    }
  }
}

void BookViewModel::AddQuote(const BookQuote& quote, int quoteIndex) {
  m_minRow = std::min(m_minRow, quoteIndex);
  m_maxRow = std::max(m_maxRow, quoteIndex);
  if(m_quoteLevels.empty()) {
    m_quoteLevels.push_back(0);
  } else if(quoteIndex > 0 &&
      quote.m_quote.m_price == m_bookQuotes[quoteIndex - 1].m_quote.m_price) {
    auto nextLevel = m_quoteLevels[quoteIndex - 1];
    m_quoteLevels.insert(m_quoteLevels.begin() + quoteIndex, nextLevel);
  } else if(quoteIndex < static_cast<int>(m_bookQuotes.size()) - 1 &&
      quote.m_quote.m_price == m_bookQuotes[quoteIndex + 1].m_quote.m_price) {
    auto nextLevel = m_quoteLevels[quoteIndex];
    m_quoteLevels.insert(m_quoteLevels.begin() + quoteIndex, nextLevel);
  } else {
    int level;
    if(quoteIndex == 0) {
      level = 0;
    } else {
      level = m_quoteLevels[quoteIndex - 1] + 1;
    }
    m_quoteLevels.insert(m_quoteLevels.begin() + quoteIndex, level);
    for(auto i = m_quoteLevels.begin() + quoteIndex + 1;
        i != m_quoteLevels.end(); ++i) {
      ++(*i);
    }
    auto lastRow = static_cast<int>(m_quoteLevels.size() - 1);
  }
}

void BookViewModel::RemoveQuote(const BookQuote& quote, int quoteIndex) {
  m_minRow = std::min(m_minRow, quoteIndex);
  m_maxRow = std::max(m_maxRow, quoteIndex);
  if(m_quoteLevels.size() == 1) {
    m_quoteLevels.clear();
  } else if(quoteIndex == m_bookQuotes.size()) {
    m_quoteLevels.pop_back();
  } else if(quoteIndex > 0 &&
      quote.m_quote.m_price == m_bookQuotes[quoteIndex - 1].m_quote.m_price ||
      quote.m_quote.m_price == m_bookQuotes[quoteIndex].m_quote.m_price) {
    m_quoteLevels.erase(m_quoteLevels.begin() + quoteIndex);
  } else {
    for(auto i = m_quoteLevels.begin() + quoteIndex;
        i != m_quoteLevels.end(); ++i) {
      --(*i);
    }
    m_quoteLevels.erase(m_quoteLevels.begin() + quoteIndex);
    auto lastRow = static_cast<int>(m_quoteLevels.size() - 1);
  }
}

void BookViewModel::OnMarketQuote(const MarketQuote& quote) {
  auto& previousMarketQuote = m_marketQuotes[quote.m_market];
  auto mpid = m_userProfile->GetMarketDatabase().FromCode(
    quote.m_market).m_displayName;
  if(!previousMarketQuote.m_market.IsEmpty()) {
    auto previousBookQuotes = ToBookQuotePair(previousMarketQuote);
    auto& previousBookQuote = Pick(m_side, previousBookQuotes.m_ask,
      previousBookQuotes.m_bid);
    previousBookQuote.m_quote.m_size = 0;
    previousBookQuote.m_mpid = mpid;
    OnBookQuote(previousBookQuote);
  }
  auto bookQuotes = ToBookQuotePair(quote);
  auto& bookQuote = Pick(m_side, bookQuotes.m_ask, bookQuotes.m_bid);
  previousMarketQuote = quote;
  bookQuote.m_mpid = mpid;
  OnBookQuote(bookQuote);
}

void BookViewModel::OnBookQuote(const BookQuote& quote) {
  if(quote.m_quote.m_side != m_side) {
    return;
  }
  HighlightQuote(quote);
  auto Comparator = [] (const BookQuote& lhs, const BookQuote& rhs) {
    return std::make_tuple(
      -GetDirection(lhs.m_quote.m_side) * lhs.m_quote.m_price,
      -lhs.m_quote.m_size, lhs.m_timestamp, lhs.m_mpid) <
      std::make_tuple(-GetDirection(rhs.m_quote.m_side) * rhs.m_quote.m_price,
      -rhs.m_quote.m_size, rhs.m_timestamp, rhs.m_mpid);
  };
  auto quoteLocation = find_if(m_bookQuotes.begin(), m_bookQuotes.end(),
    [&] (const auto& entry) {
      return entry.m_quote.m_price == quote.m_quote.m_price &&
        entry.m_mpid == quote.m_mpid;
    });
  if(quoteLocation == m_bookQuotes.end()) {
    if(quote.m_quote.m_size != 0) {
      quoteLocation = lower_bound(m_bookQuotes.begin(), m_bookQuotes.end(),
        quote, Comparator);
      quoteLocation = m_bookQuotes.insert(quoteLocation, quote);
      AddQuote(quote, distance(m_bookQuotes.begin(), quoteLocation));
    }
    return;
  }
  auto quoteIndex = distance(m_bookQuotes.begin(), quoteLocation);
  m_bookQuotes.erase(quoteLocation);
  RemoveQuote(quote, quoteIndex);
  if(quote.m_quote.m_size != 0) {
    quoteLocation = lower_bound(m_bookQuotes.begin(), m_bookQuotes.end(), quote,
      Comparator);
    quoteIndex = distance(m_bookQuotes.begin(), quoteLocation);
    m_bookQuotes.insert(quoteLocation, quote);
    AddQuote(quote, quoteIndex);
  }
}

void BookViewModel::OnOrderExecuted(const Order* order) {
  if(order->GetInfo().m_fields.m_security != m_security ||
      order->GetInfo().m_fields.m_side != m_side ||
      order->GetInfo().m_fields.m_type != OrderType::LIMIT) {
    return;
  }
  order->GetPublisher().Monitor(m_slotHandler.GetSlot<ExecutionReport>(
    std::bind(&BookViewModel::OnExecutionReport, this, order,
    std::placeholders::_1)));
}

void BookViewModel::OnExecutionReport(const Order* order,
    const ExecutionReport& executionReport) {
  OrderKey key;
  key.m_price = order->GetInfo().m_fields.m_price;
  key.m_destination = order->GetInfo().m_fields.m_destination;
  if(executionReport.m_status == OrderStatus::PENDING_NEW) {
    m_remainingOrderQuantities.insert(
      make_pair(order, order->GetInfo().m_fields.m_quantity));
    auto& quantity = m_orderQuantities[key];
    quantity += order->GetInfo().m_fields.m_quantity;
    if(m_properties.GetOrderHighlight() != BookViewProperties::HIDE_ORDERS) {
      Quote quote(key.m_price, quantity, m_side);
      BookQuote bookQuote("@" + key.m_destination, false, MarketCode(), quote,
        executionReport.m_timestamp);
      OnBookQuote(bookQuote);
    }
  } else if(executionReport.m_lastQuantity != 0 ||
      IsTerminal(executionReport.m_status)) {
    auto quantityIterator = m_orderQuantities.find(key);
    if(quantityIterator == m_orderQuantities.end()) {
      return;
    }
    auto& quantity = quantityIterator->second;
    if(IsTerminal(executionReport.m_status)) {
      quantity -= m_remainingOrderQuantities[order];
    } else {
      quantity -= executionReport.m_lastQuantity;
    }
    if(m_properties.GetOrderHighlight() != BookViewProperties::HIDE_ORDERS) {
      Quote quote(key.m_price, quantity, m_side);
      BookQuote bookQuote("@" + key.m_destination, false, MarketCode(), quote,
        executionReport.m_timestamp);
      OnBookQuote(bookQuote);
    }
  }
  m_remainingOrderQuantities[order] -= executionReport.m_lastQuantity;
}

void BookViewModel::OnBookQuoteInterruption(const std::exception_ptr& e) {
  auto bookQuotes = m_bookQuotes;
  unordered_set<string> marketQuoteMpids;
  for(auto& marketCode : m_marketQuotes | adaptors::map_keys) {
    auto mpid = m_userProfile->GetMarketDatabase().FromCode(
      marketCode).m_displayName;
    marketQuoteMpids.insert(mpid);
  }
  for(auto& bookQuote : bookQuotes) {
    if(!bookQuote.m_mpid.empty() && bookQuote.m_mpid[0] != '@' &&
        marketQuoteMpids.find(bookQuote.m_mpid) == marketQuoteMpids.end()) {
      auto clearQuote = bookQuote;
      clearQuote.m_quote.m_size = 0;
      OnBookQuote(clearQuote);
    }
  }
  QueryRealTimeBookQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(),
    m_security, m_slotHandler.GetSlot<BookQuote>(
    std::bind(&BookViewModel::OnBookQuote, this, std::placeholders::_1),
    std::bind(&BookViewModel::OnBookQuoteInterruption, this,
    std::placeholders::_1)), InterruptionPolicy::BREAK_QUERY);
}

void BookViewModel::OnMarketQuoteInterruption(const std::exception_ptr& e) {
  auto marketQuotes = m_marketQuotes;
  for(auto& marketQuote : marketQuotes | adaptors::map_values) {
    auto clearQuote = marketQuote;
    clearQuote.m_ask.m_size = 0;
    clearQuote.m_bid.m_size = 0;
    OnMarketQuote(clearQuote);
  }
  QueryRealTimeMarketQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(),
    m_security, m_slotHandler.GetSlot<MarketQuote>(
    std::bind(&BookViewModel::OnMarketQuote, this, std::placeholders::_1),
    std::bind(&BookViewModel::OnMarketQuoteInterruption, this,
    std::placeholders::_1)), InterruptionPolicy::BREAK_QUERY);
}

void BookViewModel::OnUpdateTimer() {
  m_minRow = m_bookQuotes.size();
  m_maxRow = -1;
  auto startCount = m_bookQuotes.size();
  HandleTasks(m_slotHandler);
  auto endCount = m_bookQuotes.size();
  m_maxRow = std::min(m_maxRow, static_cast<int>(startCount) - 1);
  m_minRow = std::min(m_minRow, m_maxRow);
  if(m_maxRow != -1) {
    dataChanged(index(m_minRow, 0), index(m_maxRow, COLUMN_COUNT - 1));
  }
  if(startCount < endCount) {
    beginInsertRows(QModelIndex{}, startCount, endCount - 1);
    endInsertRows();
  } else if(startCount > endCount) {
    beginRemoveRows(QModelIndex{}, endCount - 1, startCount - 1);
    endRemoveRows();
  }
}

bool BookViewModel::OrderKey::operator <(const OrderKey& value) const {
  return std::tie(m_price, m_destination) <
    std::tie(value.m_price, value.m_destination);
}
