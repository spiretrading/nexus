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
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

BookViewProperties BookViewProperties::GetDefault() {
  auto properties = BookViewProperties();
  properties.SetBboQuoteFont(QFont("Arial", 14, QFont::Bold));
  properties.SetBookQuoteFont(QFont("Arial", 8, QFont::Bold));
  auto baseColor = QColor(0, 84, 168);
  for(auto i = 0; i < 7; ++i) {
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
  auto bookFilePath = userProfile->GetProfilePath() / "book_view.dat";
  if(!std::filesystem::exists(bookFilePath)) {
    userProfile->SetDefaultBookViewProperties(GetDefault());
    return;
  }
  auto properties = BookViewProperties();
  try {
    auto reader = BasicIStreamReader<std::ifstream>(
      init(bookFilePath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(properties);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load book view properties, using defaults."));
    properties = GetDefault();
  }
  userProfile->SetDefaultBookViewProperties(properties);
}

void BookViewProperties::Save(const UserProfile& userProfile) {
  auto bookFilePath = userProfile.GetProfilePath() / "book_view.dat";
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.shuttle(userProfile.GetDefaultBookViewProperties());
    auto writer = BasicOStreamWriter<std::ofstream>(
      init(bookFilePath, std::ios::binary));
    writer.write(buffer);
  } catch(const std::exception&) {
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

const std::vector<QColor>&
    BookViewProperties::GetBookQuoteBackgroundColors() const {
  return m_bookQuoteBackgroundColors;
}

std::vector<QColor>& BookViewProperties::GetBookQuoteBackgroundColors() {
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

optional<const BookViewProperties::VenueHighlight&>
    BookViewProperties::GetVenueHighlight(Venue venue) const {
  auto venueIterator = m_venueHighlights.find(venue);
  if(venueIterator == m_venueHighlights.end()) {
    return none;
  }
  return venueIterator->second;
}

void BookViewProperties::SetVenueHighlight(
    Venue venue, const VenueHighlight& highlight) {
  m_venueHighlights[venue] = highlight;
}

void BookViewProperties::RemoveVenueHighlight(Venue venue) {
  m_venueHighlights.erase(venue);
}

BookViewProperties::OrderHighlight
    BookViewProperties::GetOrderHighlight() const {
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
