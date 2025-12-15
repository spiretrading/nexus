#include "Spire/BookView/BookViewHighlightPropertiesWidget.hpp"
#include <ranges>
#include <QColorDialog>
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_BookViewHighlightPropertiesWidget.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  QString GetButtonStyle(const QColor& color) {
    QString style = "QPushButton {background-color: " + color.name() + ";}";
    return style;
  }
}

BookViewHighlightPropertiesWidget::BookViewHighlightPropertiesWidget(
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_BookViewHighlightPropertiesWidget>()) {
  m_ui->setupUi(this);
  connect(m_ui->m_venuesList, &QListWidget::currentItemChanged, this,
    &BookViewHighlightPropertiesWidget::OnCurrentItemChanged);
  connect(m_ui->m_highlightNoneButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnNoneClicked);
  connect(m_ui->m_highlightTopLevelButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnTopLevelClicked);
  connect(m_ui->m_highlightAllLevelsButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnAllLevelsClicked);
  connect(m_ui->m_highlightColorButton, &QPushButton::clicked, this,
    &BookViewHighlightPropertiesWidget::OnColorClicked);
  connect(m_ui->m_hideOrdersButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnHideOrdersClicked);
  connect(m_ui->m_displayOrdersButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnDisplayOrdersClicked);
  connect(m_ui->m_highlightOrdersButton, &QRadioButton::toggled, this,
    &BookViewHighlightPropertiesWidget::OnHighlightOrdersClicked);
  connect(m_ui->m_orderColorButton, &QPushButton::clicked, this,
    &BookViewHighlightPropertiesWidget::OnOrderHighlightColorClicked);
}

BookViewHighlightPropertiesWidget::~BookViewHighlightPropertiesWidget() {}

void BookViewHighlightPropertiesWidget::Initialize(
    Ref<UserProfile> userProfile, Ref<BookViewProperties> properties) {
  m_userProfile = userProfile.get();
  m_properties = properties.get();
  auto venueView = m_userProfile->GetVenueDatabase().get_entries();
  auto venueEntries = std::vector(venueView.begin(), venueView.end());
  sort(venueEntries.begin(), venueEntries.end(),
    [] (const VenueDatabase::Entry& lhs, const VenueDatabase::Entry& rhs) {
      return lhs.m_display_name < rhs.m_display_name;
    });
  for(auto i = venueEntries.begin(); i != venueEntries.end(); ++i) {
    QListWidgetItem* item = new QListWidgetItem(
      QString::fromStdString(i->m_display_name));
    m_ui->m_venuesList->addItem(item);
  }
  m_ui->m_venuesList->setCurrentRow(0);
  UpdateOrderHighlightColor(m_properties->GetOrderHighlightColor());
  if(properties->GetOrderHighlight() == BookViewProperties::HIDE_ORDERS) {
    m_ui->m_hideOrdersButton->setChecked(true);
  } else if(properties->GetOrderHighlight() ==
      BookViewProperties::DISPLAY_ORDERS) {
    m_ui->m_displayOrdersButton->setChecked(true);
  } else if(properties->GetOrderHighlight() ==
      BookViewProperties::HIGHLIGHT_ORDERS) {
    m_ui->m_highlightOrdersButton->setChecked(true);
  }
}

void BookViewHighlightPropertiesWidget::Redisplay() {
  for(int i = 0; i < m_ui->m_venuesList->count(); ++i) {
    auto item = m_ui->m_venuesList->item(i);
    auto& venueEntry = m_userProfile->GetVenueDatabase().from_display_name(
      item->text().toStdString());
    QColor backgroundColor;
    auto highlight = m_properties->GetVenueHighlight(venueEntry.m_venue);
    if(highlight.is_initialized()) {
      item->setBackground(highlight->m_color);
    } else {
      item->setBackground(m_properties->GetBookQuoteBackgroundColors().front());
    }
    item->setForeground(m_properties->GetBookQuoteForegroundColor());
    item->setTextAlignment(Qt::AlignCenter);
    QFont font = m_properties->GetBookQuoteFont();
    font.setBold(true);
    item->setFont(font);
  }
  OnCurrentItemChanged(m_ui->m_venuesList->currentItem(), nullptr);
}

const VenueDatabase::Entry& BookViewHighlightPropertiesWidget::
    GetCurrentVenueHighlightEntry() const {
  return m_userProfile->GetVenueDatabase().from_display_name(
    m_ui->m_venuesList->currentItem()->text().toStdString());
}

void BookViewHighlightPropertiesWidget::UpdateVenueHighlightColor(
    const QColor& color) {
  if(color == m_venueHighlightColor) {
    return;
  }
  m_venueHighlightColor = color;
  m_ui->m_highlightColorButton->setStyleSheet(GetButtonStyle(
    m_venueHighlightColor));
  Redisplay();
}

void BookViewHighlightPropertiesWidget::UpdateOrderHighlightColor(
    const QColor& color) {
  if(color.isValid() && color == m_orderHighlightColor) {
    return;
  }
  m_orderHighlightColor = color;
  m_ui->m_orderColorButton->setStyleSheet(GetButtonStyle(
    m_orderHighlightColor));
  Redisplay();
}

void BookViewHighlightPropertiesWidget::OnCurrentItemChanged(
    QListWidgetItem* current, QListWidgetItem* previous) {
  auto& currentVenueEntry = GetCurrentVenueHighlightEntry();
  auto currentVenueHighlight = m_properties->GetVenueHighlight(
    currentVenueEntry.m_venue);
  QRadioButton* highlightButton;
  if(!currentVenueHighlight.is_initialized()) {
    highlightButton = m_ui->m_highlightNoneButton;
  } else if(!currentVenueHighlight->m_highlightAllLevels) {
    highlightButton = m_ui->m_highlightTopLevelButton;
  } else {
    highlightButton = m_ui->m_highlightAllLevelsButton;
  }
  if(currentVenueHighlight.is_initialized()) {
    UpdateVenueHighlightColor(currentVenueHighlight->m_color);
  } else {
    UpdateVenueHighlightColor(
      m_properties->GetBookQuoteBackgroundColors().front());
  }
  highlightButton->setChecked(true);
}

void BookViewHighlightPropertiesWidget::OnNoneClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentVenueEntry = GetCurrentVenueHighlightEntry();
  m_properties->RemoveVenueHighlight(currentVenueEntry.m_venue);
}

void BookViewHighlightPropertiesWidget::OnTopLevelClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentVenueEntry = GetCurrentVenueHighlightEntry();
  auto currentHighlight = m_properties->GetVenueHighlight(
    currentVenueEntry.m_venue);
  BookViewProperties::VenueHighlight newHighlight;
  if(currentHighlight.is_initialized()) {
    newHighlight = *currentHighlight;
  } else {
    newHighlight.m_color = m_venueHighlightColor;
  }
  newHighlight.m_highlightAllLevels = false;
  m_properties->SetVenueHighlight(currentVenueEntry.m_venue, newHighlight);
}

void BookViewHighlightPropertiesWidget::OnAllLevelsClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentVenueEntry = GetCurrentVenueHighlightEntry();
  auto currentHighlight = m_properties->GetVenueHighlight(
    currentVenueEntry.m_venue);
  BookViewProperties::VenueHighlight newHighlight;
  if(currentHighlight.is_initialized()) {
    newHighlight = *currentHighlight;
  } else {
    newHighlight.m_color = m_venueHighlightColor;
  }
  newHighlight.m_highlightAllLevels = true;
  m_properties->SetVenueHighlight(currentVenueEntry.m_venue, newHighlight);
}

void BookViewHighlightPropertiesWidget::OnColorClicked() {
  QColor color = QColorDialog::getColor(m_venueHighlightColor, this);
  if(!color.isValid()) {
    return;
  }
  auto& currentVenueEntry = GetCurrentVenueHighlightEntry();
  auto currentVenueHighlight = m_properties->GetVenueHighlight(
    currentVenueEntry.m_venue);
  BookViewProperties::VenueHighlight newVenueHighlight;
  if(currentVenueHighlight.is_initialized()) {
    newVenueHighlight = *currentVenueHighlight;
  } else {
    newVenueHighlight.m_highlightAllLevels =
      m_ui->m_highlightAllLevelsButton->isChecked();
  }
  newVenueHighlight.m_color = color;
  m_properties->SetVenueHighlight(currentVenueEntry.m_venue,
    newVenueHighlight);
  UpdateVenueHighlightColor(color);
}

void BookViewHighlightPropertiesWidget::OnHideOrdersClicked(
    bool checked) {
  if(!checked) {
    return;
  }
  m_properties->SetOrderHighlight(BookViewProperties::HIDE_ORDERS);
  m_ui->m_orderColorButton->setEnabled(false);
}

void BookViewHighlightPropertiesWidget::OnDisplayOrdersClicked(
    bool checked) {
  if(!checked) {
    return;
  }
  m_properties->SetOrderHighlight(BookViewProperties::DISPLAY_ORDERS);
  m_ui->m_orderColorButton->setEnabled(false);
}

void BookViewHighlightPropertiesWidget::OnHighlightOrdersClicked(
    bool checked) {
  if(!checked) {
    return;
  }
  m_properties->SetOrderHighlight(BookViewProperties::HIGHLIGHT_ORDERS);
  m_properties->SetOrderHighlightColor(m_orderHighlightColor);
  m_ui->m_orderColorButton->setEnabled(true);
}

void BookViewHighlightPropertiesWidget::OnOrderHighlightColorClicked() {
  QColor color = QColorDialog::getColor(m_orderHighlightColor, this);
  if(!color.isValid()) {
    return;
  }
  m_properties->SetOrderHighlightColor(color);
  UpdateOrderHighlightColor(color);
}
