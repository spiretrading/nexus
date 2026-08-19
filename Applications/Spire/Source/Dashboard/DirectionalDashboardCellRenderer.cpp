#include "Spire/Dashboard/DirectionalDashboardCellRenderer.hpp"
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

DirectionalDashboardCellRenderer::DirectionalDashboardCellRenderer(
    Ref<const DashboardCell> cell, Ref<UserProfile> userProfile)
    : DashboardCellRenderer{Ref(cell)},
      m_userProfile{userProfile.get()},
      m_pen{QColor{255, 255, 255}},
      m_alignment(Qt::AlignRight | Qt::AlignBottom),
      m_delegate{std::make_unique<CustomVariantItemDelegate>(
        Ref(*m_userProfile))},
      m_isTextStale(true),
      m_direction{0} {
  UpdateText();
  m_cellUpdateConnection = GetCell().ConnectUpdateSignal(
    std::bind(&DirectionalDashboardCellRenderer::OnCellUpdateSignal, this,
    std::placeholders::_1));
}

const QFont& DirectionalDashboardCellRenderer::GetFont() const {
  return m_font;
}

void DirectionalDashboardCellRenderer::SetFont(const QFont& font) {
  m_font = font;
}

const QPen& DirectionalDashboardCellRenderer::GetPen() const {
  return m_pen;
}

QPen& DirectionalDashboardCellRenderer::GetPen() {
  return m_pen;
}

QFlags<Qt::AlignmentFlag>
    DirectionalDashboardCellRenderer::GetAlignment() const {
  return m_alignment;
}

void DirectionalDashboardCellRenderer::SetAlignment(
    QFlags<Qt::AlignmentFlag> alignment) {
  m_alignment = alignment;
}

void DirectionalDashboardCellRenderer::Draw(
    DashboardPainter& painter, const QRect& region) {
  if(m_isTextStale) {
    UpdateText();
  }
  painter.SetPen(m_pen);
  painter.SetFont(m_font);
  painter.GetPainter().drawText(region, m_alignment, m_text);
}

connection DirectionalDashboardCellRenderer::ConnectDrawSignal(
    const DrawSignal::slot_function_type& slot) const {
  return m_drawSignal.connect(slot);
}

void DirectionalDashboardCellRenderer::UpdateText() {
  auto& values = GetCell().GetValues();
  if(values.empty()) {
    m_text = QObject::tr("N/A");
  } else {
    auto variant = apply_visitor(VariantVisitor(), values.back());
    if(variant.type() == QVariant::Type::Double) {
      m_text = QString::number(100 * variant.toDouble(), 'f', 2) + "%";
    } else {
      m_text = m_delegate->displayText(variant, QLocale());
    }
  }
  m_isTextStale = false;
}

void DirectionalDashboardCellRenderer::OnCellUpdateSignal(
    const DashboardCell::Value& value) {
  if(m_lastValue.is_initialized()) {
    if(value > *m_lastValue) {
      m_direction = 1;
      m_pen.setColor(QColor(0, 255, 0));
    } else if(value < *m_lastValue) {
      m_direction = -1;
      m_pen.setColor(QColor(255, 0, 0));
    }
  }
  m_lastValue = value;
  if(m_isTextStale) {
    return;
  }
  m_isTextStale = true;
  m_drawSignal();
}
