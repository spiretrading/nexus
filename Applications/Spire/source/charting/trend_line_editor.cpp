#include "spire/charting/trend_line_editor.hpp"

using namespace Spire;

TrendLineEditor::TrendLineEditor(QWidget* parent)
  : QWidget(parent),
    m_color(QColor("#FFCA19")),
    m_line_style(Qt::SolidLine) {}
