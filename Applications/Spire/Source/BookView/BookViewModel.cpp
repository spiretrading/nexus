#include "Spire/BookView/BookViewModel.hpp"
#include <tuple>
#include <Beam/Utilities/HashTuple.hpp>
#include <boost/range/adaptor/map.hpp>
#include <QCoreApplication>
#include "Nexus/Definitions/QuoteConversions.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;
using namespace Spire;

BookViewModel::BookViewModel(Ref<UserProfile> userProfile,
    const BookViewProperties& properties, const Security& security, Side side)
    : m_userProfile(userProfile.Get()),
      m_properties(properties),
      m_security(security),
      m_side(side) {
  if(m_security == Security()) {
    return;
  }
  QueryRealTimeBookQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(), m_security,
    m_eventHandler.get_slot<BookQuote>(
      std::bind_front(&BookViewModel::OnBookQuote, this),
      std::bind_front(&BookViewModel::OnBookQuoteInterruption, this)),
    InterruptionPolicy::BREAK_QUERY);
  QueryRealTimeMarketQuotesWithSnapshot(
    m_userProfile->GetServiceClients().GetMarketDataClient(), m_security,
    m_eventHandler.get_slot<MarketQuote>(
      std::bind_front(&BookViewModel::OnMarketQuote, this),
      std::bind_front(&BookViewModel::OnMarketQuoteInterruption, this)),
    InterruptionPolicy::BREAK_QUERY);
  m_userProfile->GetBlotterSettings().GetConsolidatedBlotter(
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount()).
      GetTasksModel().GetOrderExecutionPublisher().Monitor(
        m_eventHandler.get_slot<const Order*>(
          std::bind_front(&BookViewModel::OnOrderExecuted, this)));
}

void BookViewModel::SetProperties(const BookViewProperties& properties) {
  m_properties = properties;
  for(auto& orderQuantity : m_orderQuantities) {
    auto bookQuote = BookQuote("@" + orderQuantity.first.m_destination, false,
      MarketCode(), Quote(orderQuantity.first.m_price, 0, m_side),
      m_userProfile->GetServiceClients().GetTimeClient().GetTime());
    OnBookQuote(bookQuote);
  }
  if(m_properties.GetOrderHighlight() != BookViewProperties::HIDE_ORDERS) {
    for(auto& orderQuantity : m_orderQuantities) {
      auto bookQuote = BookQuote("@" + orderQuantity.first.m_destination,
        false, MarketCode(),
        Quote(orderQuantity.first.m_price, orderQuantity.second, m_side),
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
  auto& entry = *m_bookQuotes[m_bookQuotes.size() - 1 - index.row()];
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
    if(!entry.m_quote.m_mpid.empty() && entry.m_quote.m_mpid[0] == '@' &&
        m_properties.GetOrderHighlight() ==
        BookViewProperties::HIGHLIGHT_ORDERS) {
      return m_properties.GetOrderHighlightColor();
    }
    auto highlight = m_properties.GetMarketHighlight(entry.m_quote.m_market);
    if(highlight && TestHighlight(*highlight, entry.m_quote)) {
      return highlight->m_color;
    }
    if(entry.m_level <
        static_cast<int>(m_properties.GetBookQuoteBackgroundColors().size())) {
      return m_properties.GetBookQuoteBackgroundColors()[entry.m_level];
    } else if(!m_properties.GetBookQuoteBackgroundColors().empty()) {
      return m_properties.GetBookQuoteBackgroundColors().back();
    } else {
      return QVariant();
    }
  } else if(role == Qt::ForegroundRole) {
    return m_properties.GetBookQuoteForegroundColor();
  } else if(role == Qt::DisplayRole) {
    if(index.column() == PRICE_COLUMN) {
      return QVariant::fromValue(entry.m_quote.m_quote.m_price);
    } else if(index.column() == SIZE_COLUMN) {
      return QVariant::fromValue(Floor(entry.m_quote.m_quote.m_size / 100, 0));
    } else if(index.column() == MPID_COLUMN) {
      return QString::fromStdString(entry.m_quote.m_mpid);
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
  return topQuote.m_mpid == quote.m_mpid &&
    topQuote.m_quote.m_price == quote.m_quote.m_price;
}

void BookViewModel::HighlightQuote(const BookQuote& quote) {
  auto topQuoteIterator = m_topLevels.find(quote.m_market);
  if(topQuoteIterator == m_topLevels.end()) {
    if(quote.m_quote.m_size != 0) {
      m_topLevels.insert(std::pair(quote.m_market, quote));
    }
  } else {
    auto& topQuote = topQuoteIterator->second;
    if(quote.m_quote.m_size == 0 && quote.m_mpid == topQuote.m_mpid &&
        quote.m_quote.m_price == topQuote.m_quote.m_price) {
      auto newTopQuoteIndex = -1;
      auto previousTopQuoteIndex = -1;
      auto i = m_bookQuotes.rbegin();
      while(i != m_bookQuotes.rend() &&
          (newTopQuoteIndex == -1 || previousTopQuoteIndex == -1)) {
        auto& entry = **i;
        if(newTopQuoteIndex == -1 && entry.m_quote.m_market == quote.m_market &&
            entry.m_quote != topQuote) {
          newTopQuoteIndex =
            static_cast<int>(std::distance(m_bookQuotes.rbegin(), i));
        }
        if(previousTopQuoteIndex == -1 && entry.m_quote == topQuote) {
          previousTopQuoteIndex =
            static_cast<int>(std::distance(m_bookQuotes.rbegin(), i));
        }
        ++i;
      }
      if(newTopQuoteIndex != -1) {
        topQuote =
          m_bookQuotes[m_bookQuotes.size() - 1 - newTopQuoteIndex]->m_quote;
        dataChanged(index(newTopQuoteIndex, 0),
          index(newTopQuoteIndex, COLUMN_COUNT - 1));
      } else {
        m_topLevels.erase(quote.m_market);
      }
      if(previousTopQuoteIndex != -1) {
        dataChanged(index(previousTopQuoteIndex, 0),
          index(previousTopQuoteIndex, COLUMN_COUNT - 1));
      }
    } else if(quote.m_quote.m_size != 0 &&
        (-GetDirection(m_side) * quote.m_quote.m_price <
        -GetDirection(m_side) * topQuote.m_quote.m_price ||
        quote.m_quote.m_price == topQuote.m_quote.m_price &&
        quote.m_isPrimaryMpid && !topQuote.m_isPrimaryMpid)) {
      auto bookQuoteIterator = std::find_if(m_bookQuotes.rbegin(),
        m_bookQuotes.rend(), [&] (const auto& quote) {
          return quote->m_quote == topQuote;
        });
      auto topQuoteIndex = static_cast<int>(
        std::distance(m_bookQuotes.rbegin(), bookQuoteIterator));
      topQuote = quote;
      dataChanged(
        index(topQuoteIndex, 0), index(topQuoteIndex, COLUMN_COUNT - 1));
    }
  }
}

void BookViewModel::AddQuote(const BookQuote& quote, int quoteIndex) {
  auto entry = std::make_unique<BookQuoteEntry>(BookQuoteEntry(quote, 0));
  auto i = m_bookQuotes.rbegin() + quoteIndex;
  beginInsertRows(QModelIndex(), quoteIndex, quoteIndex);
  if(m_bookQuotes.empty()) {
    m_bookQuotes.push_back(std::move(entry));
    endInsertRows();
  } else if(i != m_bookQuotes.rend() &&
      quote.m_quote.m_price == (*i)->m_quote.m_quote.m_price) {
    entry->m_level = (*i)->m_level;
    m_bookQuotes.insert(i.base(), std::move(entry));
    endInsertRows();
  } else if(i != m_bookQuotes.rbegin() &&
      quote.m_quote.m_price == (*(i - 1))->m_quote.m_quote.m_price) {
    entry->m_level = (*(i - 1))->m_level;
    m_bookQuotes.insert(i.base(), std::move(entry));
    endInsertRows();
  } else {
    if(i == m_bookQuotes.rend()) {
      entry->m_level = (*(i - 1))->m_level + 1;
    } else {
      entry->m_level = (*i)->m_level;
    }
    for(auto j = i; j != m_bookQuotes.rend(); ++j) {
      ++(*j)->m_level;
    }
    m_bookQuotes.insert(i.base(), std::move(entry));
    endInsertRows();
    auto lastRow = static_cast<int>(m_bookQuotes.size() - 1);
    dataChanged(index(quoteIndex, 0), index(lastRow, COLUMN_COUNT - 1));
  }
}

void BookViewModel::RemoveQuote(int quoteIndex) {
  auto i = m_bookQuotes.rbegin() + quoteIndex;
  beginRemoveRows(QModelIndex(), quoteIndex, quoteIndex);
  if(i == (m_bookQuotes.rend() - 1) || (*i)->m_level == (*(i + 1))->m_level ||
      i != m_bookQuotes.rbegin() && (*i)->m_level == (*(i - 1))->m_level) {
    m_bookQuotes.erase((i + 1).base());
    endRemoveRows();
  } else {
    for(auto j = i + 1; j != m_bookQuotes.rend(); ++j) {
      --(*j)->m_level;
    }
    m_bookQuotes.erase((i + 1).base());
    endRemoveRows();
    if(!m_bookQuotes.empty()) {
      auto lastRow = static_cast<int>(m_bookQuotes.size() - 1);
      dataChanged(index(quoteIndex, 0), index(lastRow, COLUMN_COUNT - 1));
    }
  }
}

void BookViewModel::OnMarketQuote(const MarketQuote& quote) {
  auto& previousMarketQuote = m_marketQuotes[quote.m_market];
  auto mpid =
    m_userProfile->GetMarketDatabase().FromCode(quote.m_market).m_displayName;
  if(!previousMarketQuote.m_market.IsEmpty()) {
    auto previousBookQuotes = ToBookQuotePair(previousMarketQuote);
    auto& previousBookQuote =
      Pick(m_side, previousBookQuotes.m_ask, previousBookQuotes.m_bid);
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
  auto direction = GetDirection(m_side);
  auto lowerBound = [&] {
    for(auto i = m_bookQuotes.rbegin(); i != m_bookQuotes.rend(); ++i) {
      auto& bookQuote = (*i)->m_quote;
      if(direction * bookQuote.m_quote.m_price <=
          direction * quote.m_quote.m_price) {
        return i;
      }
    }
    return m_bookQuotes.rend();
  }();
  auto existingIterator = lowerBound;
  while(existingIterator != m_bookQuotes.rend() &&
      (*existingIterator)->m_quote.m_quote.m_price == quote.m_quote.m_price &&
      (*existingIterator)->m_quote.m_mpid != quote.m_mpid) {
    ++existingIterator;
  }
  if(existingIterator == m_bookQuotes.rend() ||
      (*existingIterator)->m_quote.m_quote.m_price != quote.m_quote.m_price) {
    if(quote.m_quote.m_size != 0) {
      auto insertIterator = lowerBound;
      while(insertIterator != m_bookQuotes.rend() &&
          (*insertIterator)->m_quote.m_quote.m_price == quote.m_quote.m_price &&
          std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
          std::tie((*insertIterator)->m_quote.m_quote.m_size,
            (*insertIterator)->m_quote.m_timestamp,
            (*insertIterator)->m_quote.m_mpid)) {
        ++insertIterator;
      }
      AddQuote(quote, std::distance(m_bookQuotes.rbegin(), insertIterator));
    }
    return;
  }
  if(quote.m_quote.m_size == 0) {
    RemoveQuote(std::distance(m_bookQuotes.rbegin(), existingIterator));
  } else {
    auto insertIterator = lowerBound;
    while(insertIterator != m_bookQuotes.rend() &&
        (*insertIterator)->m_quote.m_quote.m_price == quote.m_quote.m_price &&
        std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
        std::tie((*insertIterator)->m_quote.m_quote.m_size,
          (*insertIterator)->m_quote.m_timestamp,
          (*insertIterator)->m_quote.m_mpid)) {
      ++insertIterator;
    }
    if(insertIterator == existingIterator) {
      (*insertIterator)->m_quote.m_quote.m_size = quote.m_quote.m_size;
      (*insertIterator)->m_quote.m_timestamp = quote.m_timestamp;
      auto quoteIndex = std::distance(m_bookQuotes.rbegin(), insertIterator);
      dataChanged(index(quoteIndex, 0), index(quoteIndex, COLUMN_COUNT - 1));
    } else {
      auto existingIndex = std::distance(m_bookQuotes.rbegin(),
        existingIterator);
      auto quoteIndex = std::distance(m_bookQuotes.rbegin(), insertIterator);
      if(quoteIndex > existingIndex) {
        --quoteIndex;
      }
      RemoveQuote(existingIndex);
      AddQuote(quote, quoteIndex);
    }
  }
}

void BookViewModel::OnOrderExecuted(const Order* order) {
  if(order->GetInfo().m_fields.m_security != m_security ||
      order->GetInfo().m_fields.m_side != m_side ||
      order->GetInfo().m_fields.m_type != OrderType::LIMIT) {
    return;
  }
  order->GetPublisher().Monitor(m_eventHandler.get_slot<ExecutionReport>(
    std::bind_front(&BookViewModel::OnExecutionReport, this, order)));
}

void BookViewModel::OnExecutionReport(const Order* order,
    const ExecutionReport& executionReport) {
  auto key = OrderKey();
  key.m_price = order->GetInfo().m_fields.m_price;
  key.m_destination = order->GetInfo().m_fields.m_destination;
  if(executionReport.m_status == OrderStatus::PENDING_NEW) {
    m_remainingOrderQuantities.insert(
      std::pair(order, order->GetInfo().m_fields.m_quantity));
    auto& quantity = m_orderQuantities[key];
    quantity += order->GetInfo().m_fields.m_quantity;
    if(m_properties.GetOrderHighlight() != BookViewProperties::HIDE_ORDERS) {
      auto bookQuote = BookQuote("@" + key.m_destination, false, MarketCode(),
        Quote(key.m_price, quantity, m_side), executionReport.m_timestamp);
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
      auto bookQuote = BookQuote("@" + key.m_destination, false, MarketCode(),
        Quote(key.m_price, quantity, m_side), executionReport.m_timestamp);
      OnBookQuote(bookQuote);
    }
  }
  m_remainingOrderQuantities[order] -= executionReport.m_lastQuantity;
}

void BookViewModel::OnBookQuoteInterruption(const std::exception_ptr& e) {
  auto bookQuotes = std::vector<BookQuote>();
  for(auto& bookQuote : m_bookQuotes) {
    bookQuotes.push_back(bookQuote->m_quote);
  }
  auto marketQuoteMpids = std::unordered_set<std::string>();
  for(auto& marketCode : m_marketQuotes | adaptors::map_keys) {
    auto mpid =
      m_userProfile->GetMarketDatabase().FromCode(marketCode).m_displayName;
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
    m_userProfile->GetServiceClients().GetMarketDataClient(), m_security,
    m_eventHandler.get_slot<BookQuote>(
      std::bind_front(&BookViewModel::OnBookQuote, this),
      std::bind_front(&BookViewModel::OnBookQuoteInterruption, this)),
    InterruptionPolicy::BREAK_QUERY);
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
    m_userProfile->GetServiceClients().GetMarketDataClient(), m_security,
    m_eventHandler.get_slot<MarketQuote>(
      std::bind_front(&BookViewModel::OnMarketQuote, this),
      std::bind_front(&BookViewModel::OnMarketQuoteInterruption, this)),
    InterruptionPolicy::BREAK_QUERY);
}
