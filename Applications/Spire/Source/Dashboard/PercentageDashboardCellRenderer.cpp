#include "Spire/Dashboard/PercentageDashboardCellRenderer.hpp"
#include <QPainter>

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

PercentageDashboardCellRenderer::PercentageDashboardCellRenderer(
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
    std::bind(&PercentageDashboardCellRenderer::OnCellUpdateSignal, this,
    std::placeholders::_1));
}

const QFont& PercentageDashboardCellRenderer::GetFont() const {
  return m_font;
}

void PercentageDashboardCellRenderer::SetFont(const QFont& font) {
  m_font = font;
}

const QPen& PercentageDashboardCellRenderer::GetPen() const {
  return m_pen;
}

QPen& PercentageDashboardCellRenderer::GetPen() {
  return m_pen;
}

QFlags<Qt::AlignmentFlag>
    PercentageDashboardCellRenderer::GetAlignment() const {
  return m_alignment;
}

void PercentageDashboardCellRenderer::SetAlignment(
    QFlags<Qt::AlignmentFlag> alignment) {
  m_alignment = alignment;
}

void PercentageDashboardCellRenderer::Draw(
    QPainter& painter, const QRect& region) {
  if(m_isTextStale) {
    UpdateText();
  }
  painter.setPen(m_pen);
  painter.setFont(m_font);
  painter.drawText(region, m_alignment, m_text);
}

connection PercentageDashboardCellRenderer::ConnectDrawSignal(
    const DrawSignal::slot_function_type& slot) const {
  return m_drawSignal.connect(slot);
}

void PercentageDashboardCellRenderer::UpdateText() {
  auto& values = GetCell().GetValues();
  if(values.empty()) {
    m_text = QObject::tr("N/A");
    m_pen.setColor(QColor(255, 255, 255));
  } else {
    auto percentage = boost::get<double>(values.back());
    m_text = QString::number(100 * percentage, 'f', 2) + "%";
    if(percentage > 0) {
      m_pen.setColor(QColor(0, 255, 0));
    } else if(percentage < 0) {
      m_pen.setColor(QColor(255, 0, 0));
    } else {
      m_pen.setColor(QColor(255, 255, 255));
    }
  }
  m_isTextStale = false;
}

void PercentageDashboardCellRenderer::OnCellUpdateSignal(
    const DashboardCell::Value& value) {
  if(m_isTextStale) {
    return;
  }
  m_isTextStale = true;
  m_drawSignal();
}
