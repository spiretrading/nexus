#include "Spire/Dashboard/TextDashboardCellRenderer.hpp"
#include <boost/variant/apply_visitor.hpp>
#include <QPainter>
#include "Nexus/Definitions/Ticker.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  struct VariantVisitor {
    QVariant operator ()(const string& value) const {
      return QString::fromStdString(value);
    }

    template<typename T>
    QVariant operator ()(const T& value) const {
      return QVariant::fromValue(value);
    }
  };
}

TextDashboardCellRenderer::TextDashboardCellRenderer(
    Ref<const DashboardCell> cell, Ref<UserProfile> userProfile)
    : DashboardCellRenderer{Ref(cell)},
      m_userProfile{userProfile.get()},
      m_pen{QColor{255, 255, 255}},
      m_alignment(Qt::AlignRight | Qt::AlignBottom),
      m_delegate{std::make_unique<CustomVariantItemDelegate>(
        Ref(*m_userProfile))},
      m_isTextStale(true) {
  UpdateText();
  m_cellUpdateConnection = GetCell().ConnectUpdateSignal(
    std::bind(&TextDashboardCellRenderer::OnCellUpdateSignal, this,
    std::placeholders::_1));
}

const QFont& TextDashboardCellRenderer::GetFont() const {
  return m_font;
}

void TextDashboardCellRenderer::SetFont(const QFont& font) {
  m_font = font;
}

const QPen& TextDashboardCellRenderer::GetPen() const {
  return m_pen;
}

QPen& TextDashboardCellRenderer::GetPen() {
  return m_pen;
}

QFlags<Qt::AlignmentFlag> TextDashboardCellRenderer::GetAlignment() const {
  return m_alignment;
}

void TextDashboardCellRenderer::SetAlignment(
    QFlags<Qt::AlignmentFlag> alignment) {
  m_alignment = alignment;
}

void TextDashboardCellRenderer::Draw(QPainter& painter, const QRect& region) {
  if(m_isTextStale) {
    UpdateText();
  }
  painter.setPen(m_pen);
  painter.setFont(m_font);
  painter.drawText(region, m_alignment, m_text);
}

connection TextDashboardCellRenderer::ConnectDrawSignal(
    const DrawSignal::slot_function_type& slot) const {
  return m_drawSignal.connect(slot);
}

void TextDashboardCellRenderer::UpdateText() {
  auto& values = GetCell().GetValues();
  if(values.empty()) {
    m_text = QObject::tr("N/A");
  } else {
    m_text = m_delegate->displayText(
      apply_visitor(VariantVisitor(), values.back()), QLocale());
  }
  m_isTextStale = false;
}

void TextDashboardCellRenderer::OnCellUpdateSignal(
    const DashboardCell::Value& value) {
  if(m_isTextStale) {
    return;
  }
  m_isTextStale = true;
  m_drawSignal();
}
