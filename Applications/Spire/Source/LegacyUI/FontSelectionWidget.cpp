#include "Spire/LegacyUI/FontSelectionWidget.hpp"
#include <QFontDialog>
#include "ui_FontSelectionWidget.h"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::LegacyUI;

FontSelectionWidget::FontSelectionWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_FontSelectionWidget>()),
      m_ignoreFontChange(false) {
  m_ui->setupUi(this);
  connect(m_ui->m_advancedFontButton, &QPushButton::clicked, this,
    &FontSelectionWidget::OnAdvancedFontButtonClicked);
  connect(m_ui->m_fontComboBox, &QFontComboBox::currentFontChanged, this,
    &FontSelectionWidget::OnFontChanged);
  connect(m_ui->m_sizeSpinBox, static_cast<void (QSpinBox::*)(int)>(
    &QSpinBox::valueChanged), this, &FontSelectionWidget::OnFontSizeChanged);
}

FontSelectionWidget::~FontSelectionWidget() {}

const QFont& FontSelectionWidget::GetFont() const {
  return m_font;
}

void FontSelectionWidget::SetFont(const QFont& font) {
  if(font == m_font) {
    return;
  }
  QFont oldFont = m_font;
  m_font = font;
  m_ui->m_sizeSpinBox->setValue(m_font.pointSize());
  m_ignoreFontChange = true;
  m_ui->m_fontComboBox->setCurrentFont(font);
  m_ignoreFontChange = false;
  m_fontChangedSignal(oldFont, m_font);
}

connection FontSelectionWidget::ConnectFontChangedSignal(
    const FontChangedSignal::slot_type& slot) const {
  return m_fontChangedSignal.connect(slot);
}

void FontSelectionWidget::OnAdvancedFontButtonClicked() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, m_font, this);
  if(ok) {
    SetFont(font);
  }
}

void FontSelectionWidget::OnFontChanged(const QFont& font) {
  if(m_ignoreFontChange) {
    return;
  }
  SetFont(font);
}

void FontSelectionWidget::OnFontSizeChanged(int value) {
  QFont resizedFont(m_font);
  resizedFont.setPointSize(value);
  SetFont(resizedFont);
}
