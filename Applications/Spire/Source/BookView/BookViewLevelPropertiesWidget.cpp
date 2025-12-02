#include "Spire/BookView/BookViewLevelPropertiesWidget.hpp"
#include <QColorDialog>
#include "Spire/BookView/BookViewProperties.hpp"
#include "ui_BookViewLevelPropertiesWidget.h"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  QString GetButtonStyle(const QColor& color) {
    QString style = "QPushButton {background-color: " + color.name() + ";}";
    return style;
  }
}

BookViewLevelPropertiesWidget::BookViewLevelPropertiesWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_BookViewLevelPropertiesWidget>()) {
  m_ui->setupUi(this);
  connect(m_ui->m_bandList, &QListWidget::currentItemChanged, this,
    &BookViewLevelPropertiesWidget::OnPriceRangeItemChanged);
  connect(m_ui->m_topGradientButton, &QPushButton::clicked, this,
    &BookViewLevelPropertiesWidget::OnTopGradientButtonClicked);
  connect(m_ui->m_bottomGradientButton, &QPushButton::clicked, this,
    &BookViewLevelPropertiesWidget::OnBottomGradientButtonClicked);
  connect(m_ui->m_applyGradientButton, &QPushButton::clicked, this,
    &BookViewLevelPropertiesWidget::OnApplyGradientButtonClicked);
  connect(m_ui->m_individualColorButton, &QPushButton::clicked, this,
    &BookViewLevelPropertiesWidget::OnBandColorButtonClicked);
  connect(m_ui->m_bandCountSpinBox,
    static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
    &BookViewLevelPropertiesWidget::OnBandCountValueChanged);
  m_fontChangedConnection = m_ui->m_fontSelector->ConnectFontChangedSignal(
    std::bind(&BookViewLevelPropertiesWidget::OnFontChanged, this,
    std::placeholders::_1, std::placeholders::_2));
  connect(m_ui->m_showGridLinesCheckBox, &QCheckBox::stateChanged, this,
    &BookViewLevelPropertiesWidget::OnShowGridClicked);
  connect(m_ui->m_showBboCheckBox, &QCheckBox::stateChanged, this,
    &BookViewLevelPropertiesWidget::OnShowBboClicked);
}

BookViewLevelPropertiesWidget::~BookViewLevelPropertiesWidget() {}

void BookViewLevelPropertiesWidget::Initialize(
    Ref<BookViewProperties> properties) {
  m_properties = properties.get();
}

void BookViewLevelPropertiesWidget::Redisplay() {
  m_colorBandItems.clear();
  m_ui->m_bandList->clear();
  for(auto i = m_properties->GetBookQuoteBackgroundColors().begin();
      i != m_properties->GetBookQuoteBackgroundColors().end(); ++i) {
    QListWidgetItem* item = new QListWidgetItem(tr("Level ") +
      QString::number(m_ui->m_bandList->count() + 1));
    item->setBackground(*i);
    item->setForeground(m_properties->GetBookQuoteForegroundColor());
    item->setTextAlignment(Qt::AlignCenter);
    m_colorBandItems.push_back(item);
    m_ui->m_bandList->addItem(item);
  }
  m_ui->m_bandCountSpinBox->setValue(
    static_cast<int>(m_properties->GetBookQuoteBackgroundColors().size()));
  m_ui->m_bandList->setFont(m_properties->GetBookQuoteFont());
  m_ui->m_bandList->setCurrentRow(0);
  m_ui->m_individualColorButton->setStyleSheet(GetButtonStyle(
    m_properties->GetBookQuoteBackgroundColors()[
    m_ui->m_bandList->currentRow()]));
  m_topGradient = m_properties->GetBookQuoteBackgroundColors().front();
  m_ui->m_topGradientButton->setStyleSheet(GetButtonStyle(m_topGradient));
  m_bottomGradient = m_properties->GetBookQuoteBackgroundColors().back();
  m_ui->m_bottomGradientButton->setStyleSheet(GetButtonStyle(m_bottomGradient));
  m_ui->m_fontSelector->SetFont(m_properties->GetBookQuoteFont());
  m_ui->m_showGridLinesCheckBox->setChecked(m_properties->GetShowGrid());
  m_ui->m_showBboCheckBox->setChecked(m_properties->GetShowBbo());
}

void BookViewLevelPropertiesWidget::OnFontChanged(const QFont& oldFont,
    const QFont& newFont) {
  m_properties->SetBookQuoteFont(newFont);
  m_ui->m_bandList->setFont(m_properties->GetBookQuoteFont());
}

void BookViewLevelPropertiesWidget::OnPriceRangeItemChanged(
    QListWidgetItem* current, QListWidgetItem* previous) {
  if(current == nullptr) {
    return;
  }
  m_ui->m_individualColorButton->setStyleSheet(GetButtonStyle(
    m_properties->GetBookQuoteBackgroundColors()[
    m_ui->m_bandList->currentRow()]));
}

void BookViewLevelPropertiesWidget::OnTopGradientButtonClicked() {
  m_topGradient = QColorDialog::getColor(m_topGradient, this);
  if(!m_topGradient.isValid()) {
    return;
  }
  m_ui->m_topGradientButton->setStyleSheet(GetButtonStyle(m_topGradient));
}

void BookViewLevelPropertiesWidget::OnBottomGradientButtonClicked() {
  m_bottomGradient = QColorDialog::getColor(m_bottomGradient, this);
  if(!m_bottomGradient.isValid()) {
    return;
  }
  m_ui->m_bottomGradientButton->setStyleSheet(GetButtonStyle(m_bottomGradient));
}

void BookViewLevelPropertiesWidget::OnApplyGradientButtonClicked() {
  int topRed;
  int topGreen;
  int topBlue;
  int bottomRed;
  int bottomGreen;
  int bottomBlue;
  m_topGradient.getRgb(&topRed, &topGreen, &topBlue);
  m_bottomGradient.getRgb(&bottomRed, &bottomGreen, &bottomBlue);
  int bands = static_cast<int>(m_colorBandItems.size());
  for(int i = 0; i < bands; ++i) {
    int red = topRed + static_cast<int>((bottomRed - topRed) *
      static_cast<double>(i) / bands);
    int green = topGreen + static_cast<int>((bottomGreen - topGreen) *
      static_cast<double>(i) / bands);
    int blue = topBlue + static_cast<int>((bottomBlue - topBlue) *
      static_cast<double>(i) / bands);
    QColor color(red, green, blue);
    m_colorBandItems[i]->setBackground(color);
    m_properties->GetBookQuoteBackgroundColors()[i] = color;
  }
}

void BookViewLevelPropertiesWidget::OnBandColorButtonClicked() {
  QColor color = QColorDialog::getColor(
    m_properties->GetBookQuoteBackgroundColors()[
    m_ui->m_bandList->currentRow()], this);
  if(!color.isValid()) {
    return;
  }
  m_ui->m_bandList->currentItem()->setBackground(color);
  m_properties->GetBookQuoteBackgroundColors()[m_ui->m_bandList->currentRow()] =
    color;
  m_ui->m_individualColorButton->setStyleSheet(GetButtonStyle(color));
}

void BookViewLevelPropertiesWidget::OnBandCountValueChanged(int value) {
  if(value < m_ui->m_bandList->count()) {
    while(value < m_ui->m_bandList->count()) {
      m_ui->m_bandList->takeItem(m_ui->m_bandList->count() - 1);
      QListWidgetItem* item = m_colorBandItems.back();
      m_colorBandItems.pop_back();
      m_properties->GetBookQuoteBackgroundColors().pop_back();
      delete item;
    }
  } else if(value > m_ui->m_bandList->count()) {
    while(value > m_ui->m_bandList->count()) {
      QListWidgetItem* item = new QListWidgetItem(tr("Level ") +
        QString::number(m_ui->m_bandList->count() + 1));
      QColor color = m_properties->GetBookQuoteBackgroundColors().back();
      item->setBackground(color);
      item->setForeground(m_properties->GetBookQuoteForegroundColor());
      item->setTextAlignment(Qt::AlignCenter);
      m_colorBandItems.push_back(item);
      m_ui->m_bandList->addItem(item);
      m_properties->GetBookQuoteBackgroundColors().push_back(color);
    }
  }
}

void BookViewLevelPropertiesWidget::OnShowGridClicked(int state) {
  if(state == Qt::Unchecked) {
    m_properties->SetShowGrid(false);
  } else if(state == Qt::Checked) {
    m_properties->SetShowGrid(true);
  }
}

void BookViewLevelPropertiesWidget::OnShowBboClicked(int state) {
  if(state == Qt::Unchecked) {
    m_properties->SetShowBbo(false);
  } else if(state == Qt::Checked) {
    m_properties->SetShowBbo(true);
  }
}
