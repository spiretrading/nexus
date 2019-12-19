#include "Spire/BookView/BookViewHighlightPropertiesWidget.hpp"
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
  connect(m_ui->m_marketsList, &QListWidget::currentItemChanged, this,
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
  m_userProfile = userProfile.Get();
  m_properties = properties.Get();
  auto marketEntries = m_userProfile->GetMarketDatabase().GetEntries();
  sort(marketEntries.begin(), marketEntries.end(),
    [] (const MarketDatabase::Entry& lhs, const MarketDatabase::Entry& rhs) {
      return lhs.m_displayName < rhs.m_displayName;
    });
  for(auto i = marketEntries.begin(); i != marketEntries.end(); ++i) {
    QListWidgetItem* item = new QListWidgetItem(
      QString::fromStdString(i->m_displayName));
    m_ui->m_marketsList->addItem(item);
  }
  m_ui->m_marketsList->setCurrentRow(0);
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
  for(int i = 0; i < m_ui->m_marketsList->count(); ++i) {
    auto item = m_ui->m_marketsList->item(i);
    auto& marketEntry = m_userProfile->GetMarketDatabase().FromDisplayName(
      item->text().toStdString());
    QColor backgroundColor;
    auto highlight = m_properties->GetMarketHighlight(marketEntry.m_code);
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
  OnCurrentItemChanged(m_ui->m_marketsList->currentItem(), nullptr);
}

const MarketDatabase::Entry& BookViewHighlightPropertiesWidget::
    GetCurrentMarketHighlightEntry() const {
  return m_userProfile->GetMarketDatabase().FromDisplayName(
    m_ui->m_marketsList->currentItem()->text().toStdString());
}

void BookViewHighlightPropertiesWidget::UpdateMarketHighlightColor(
    const QColor& color) {
  if(color == m_marketHighlightColor) {
    return;
  }
  m_marketHighlightColor = color;
  m_ui->m_highlightColorButton->setStyleSheet(GetButtonStyle(
    m_marketHighlightColor));
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
  auto& currentMarketEntry = GetCurrentMarketHighlightEntry();
  auto currentMarketHighlight = m_properties->GetMarketHighlight(
    currentMarketEntry.m_code);
  QRadioButton* highlightButton;
  if(!currentMarketHighlight.is_initialized()) {
    highlightButton = m_ui->m_highlightNoneButton;
  } else if(!currentMarketHighlight->m_highlightAllLevels) {
    highlightButton = m_ui->m_highlightTopLevelButton;
  } else {
    highlightButton = m_ui->m_highlightAllLevelsButton;
  }
  if(currentMarketHighlight.is_initialized()) {
    UpdateMarketHighlightColor(currentMarketHighlight->m_color);
  } else {
    UpdateMarketHighlightColor(
      m_properties->GetBookQuoteBackgroundColors().front());
  }
  highlightButton->setChecked(true);
}

void BookViewHighlightPropertiesWidget::OnNoneClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentMarketEntry = GetCurrentMarketHighlightEntry();
  m_properties->RemoveMarketHighlight(currentMarketEntry.m_code);
}

void BookViewHighlightPropertiesWidget::OnTopLevelClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentMarketEntry = GetCurrentMarketHighlightEntry();
  auto currentHighlight = m_properties->GetMarketHighlight(
    currentMarketEntry.m_code);
  BookViewProperties::MarketHighlight newHighlight;
  if(currentHighlight.is_initialized()) {
    newHighlight = *currentHighlight;
  } else {
    newHighlight.m_color = m_marketHighlightColor;
  }
  newHighlight.m_highlightAllLevels = false;
  m_properties->SetMarketHighlight(currentMarketEntry.m_code, newHighlight);
}

void BookViewHighlightPropertiesWidget::OnAllLevelsClicked(bool checked) {
  if(!checked) {
    return;
  }
  auto& currentMarketEntry = GetCurrentMarketHighlightEntry();
  auto currentHighlight = m_properties->GetMarketHighlight(
    currentMarketEntry.m_code);
  BookViewProperties::MarketHighlight newHighlight;
  if(currentHighlight.is_initialized()) {
    newHighlight = *currentHighlight;
  } else {
    newHighlight.m_color = m_marketHighlightColor;
  }
  newHighlight.m_highlightAllLevels = true;
  m_properties->SetMarketHighlight(currentMarketEntry.m_code, newHighlight);
}

void BookViewHighlightPropertiesWidget::OnColorClicked() {
  QColor color = QColorDialog::getColor(m_marketHighlightColor, this);
  if(!color.isValid()) {
    return;
  }
  auto& currentMarketEntry = GetCurrentMarketHighlightEntry();
  auto currentMarketHighlight = m_properties->GetMarketHighlight(
    currentMarketEntry.m_code);
  BookViewProperties::MarketHighlight newMarketHighlight;
  if(currentMarketHighlight.is_initialized()) {
    newMarketHighlight = *currentMarketHighlight;
  } else {
    newMarketHighlight.m_highlightAllLevels =
      m_ui->m_highlightAllLevelsButton->isChecked();
  }
  newMarketHighlight.m_color = color;
  m_properties->SetMarketHighlight(currentMarketEntry.m_code,
    newMarketHighlight);
  UpdateMarketHighlightColor(color);
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
