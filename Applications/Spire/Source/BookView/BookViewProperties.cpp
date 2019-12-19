#include "Spire/BookView/BookViewProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QMessageBox>
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

BookViewProperties BookViewProperties::GetDefault() {
  BookViewProperties properties;
  properties.SetBboQuoteFont(QFont("Arial", 14, QFont::Bold));
  properties.SetBookQuoteFont(QFont("Arial", 8, QFont::Bold));
  QColor baseColor(0, 84, 168);
  for(int i = 0; i < 7; ++i) {
    properties.GetBookQuoteBackgroundColors().push_back(
      baseColor.lighter(300 - i * (200 / 7)));
  }
  properties.SetBookQuoteForegroundColor(QColor(0, 0, 0));
  properties.SetOrderHighlightColor(QColor(0, 255, 0));
  properties.SetShowGrid(false);
  properties.SetShowBbo(false);
  return properties;
}

void BookViewProperties::Load(Out<UserProfile> userProfile) {
  path bookFilePath = userProfile->GetProfilePath() / "book_view.dat";
  if(!exists(bookFilePath)) {
    userProfile->SetDefaultBookViewProperties(GetDefault());
    return;
  }
  BookViewProperties properties;
  try {
    BasicIStreamReader<ifstream> reader(Initialize(bookFilePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(properties);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load book view properties, using defaults."));
    properties = GetDefault();
  }
  userProfile->SetDefaultBookViewProperties(properties);
}

void BookViewProperties::Save(const UserProfile& userProfile) {
  path bookFilePath = userProfile.GetProfilePath() / "book_view.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(userProfile.GetDefaultBookViewProperties());
    BasicOStreamWriter<ofstream> writer(Initialize(bookFilePath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save book view properties."));
  }
}

BookViewProperties::BookViewProperties()
    : m_showGrid(false),
      m_showBbo(false) {}

const QColor& BookViewProperties::GetBookQuoteForegroundColor() const {
  return m_bookQuoteForegroundColor;
}

void BookViewProperties::SetBookQuoteForegroundColor(const QColor& color) {
  m_bookQuoteForegroundColor = color;
}

const vector<QColor>& BookViewProperties::GetBookQuoteBackgroundColors() const {
  return m_bookQuoteBackgroundColors;
}

vector<QColor>& BookViewProperties::GetBookQuoteBackgroundColors() {
  return m_bookQuoteBackgroundColors;
}

const QFont& BookViewProperties::GetBboQuoteFont() const {
  return m_bboQuoteFont;
}

void BookViewProperties::SetBboQuoteFont(const QFont& font) {
  m_bboQuoteFont = font;
}

const QFont& BookViewProperties::GetBookQuoteFont() const {
  return m_bookQuoteFont;
}

void BookViewProperties::SetBookQuoteFont(const QFont& font) {
  m_bookQuoteFont = font;
}

boost::optional<const BookViewProperties::MarketHighlight&> BookViewProperties::
    GetMarketHighlight(MarketCode market) const {
  auto marketIterator = m_marketHighlights.find(market);
  if(marketIterator == m_marketHighlights.end()) {
    return none;
  }
  return marketIterator->second;
}

void BookViewProperties::SetMarketHighlight(MarketCode market,
    const MarketHighlight& highlight) {
  m_marketHighlights[market] = highlight;
}

void BookViewProperties::RemoveMarketHighlight(MarketCode market) {
  m_marketHighlights.erase(market);
}

BookViewProperties::OrderHighlight BookViewProperties::
    GetOrderHighlight() const {
  return m_orderHighlight;
}

void BookViewProperties::SetOrderHighlight(OrderHighlight orderHighlight) {
  m_orderHighlight = orderHighlight;
}

const QColor& BookViewProperties::GetOrderHighlightColor() const {
  return m_orderHighlightColor;
}

void BookViewProperties::SetOrderHighlightColor(const QColor& color) {
  m_orderHighlightColor = color;
  m_orderHighlight = HIGHLIGHT_ORDERS;
}

bool BookViewProperties::GetShowGrid() const {
  return m_showGrid;
}

void BookViewProperties::SetShowGrid(bool value) {
  m_showGrid = value;
}

bool BookViewProperties::GetShowBbo() const {
  return m_showBbo;
}

void BookViewProperties::SetShowBbo(bool value) {
  m_showBbo = value;
}
