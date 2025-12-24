#include "Spire/Charting/ChartPlotView.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Charting/CandlestickChartPlot.hpp"
#include "Spire/Charting/ChartPlotViewWindowSettings.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  ChartValue GetScale(const ChartPlotView::AxisParameters& parameters) {
    if(parameters.m_type->GetCompatibility(Spire::MoneyType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      return ChartValue(Money::CENT);
    } else if(parameters.m_type->GetCompatibility(
        DateTimeType::GetInstance()) == CanvasType::Compatibility::EQUAL) {
      return ChartValue(minutes(5));
    }
    return ChartValue();
  }

  const auto CROSSHAIR_COLOR = QColor{255, 255, 255};
}

ChartPlotView::Properties ChartPlotView::Properties::GetDefault() {
  Properties properties;
  properties.m_backgroundColor = QColor(13, 13, 13);
  properties.m_gridLineColor = QColor(40, 40, 40);
  properties.m_gridLabelColor = QColor(200, 200, 200);
  properties.m_gridLabelFont = QFont("Arial", 8);
  properties.m_uptickColor = QColor(0, 255, 0);
  properties.m_downtickColor = QColor(255, 0, 0);
  properties.m_outlineColor = QColor(117, 117, 117);
  return properties;
}

ChartPlotView::AxisParameters::AxisParameters() {}

ChartPlotView::AxisParameters::AxisParameters(std::shared_ptr<NativeType> type,
    ChartValue min, ChartValue max, ChartValue increment)
    : m_type(std::move(type)),
      m_min(min),
      m_max(max),
      m_increment(increment) {}

ChartPlotView::ChartPlotView(QWidget* parent)
    : QWidget(parent),
      m_interactionMode(ChartInteractionMode::NONE),
      m_isDragging(false) {
  setAutoFillBackground(true);
  setMouseTracking(true);
}

void ChartPlotView::Initialize(Ref<UserProfile> userProfile,
    const Properties& properties) {
  m_userProfile = userProfile.get();
  m_properties = properties;
  QPalette p(palette());
  p.setColor(QPalette::Window, m_properties.m_backgroundColor);
  setPalette(p);
}

ChartInteractionMode ChartPlotView::GetInteractionMode() const {
  return m_interactionMode;
}

void ChartPlotView::SetInteractionMode(ChartInteractionMode interactionMode) {
  if(m_interactionMode == interactionMode) {
    return;
  }
  m_interactionMode = interactionMode;
  if(m_interactionMode == ChartInteractionMode::PAN) {
    setCursor(QCursor(Qt::OpenHandCursor));
  } else if(m_interactionMode == ChartInteractionMode::ZOOM) {
    setCursor(QCursor(QPixmap(":/icons/magnifying_glass_cursor.png")));
  } else {
    unsetCursor();
  }
}

const ChartPlotView::Properties& ChartPlotView::GetProperties() const {
  return m_properties;
}

void ChartPlotView::SetProperties(const Properties& properties) {
  m_properties = properties;
  update();
}

const ChartPlotView::AxisParameters& ChartPlotView::GetXAxisParameters() const {
  return m_xAxisParameters;
}

void ChartPlotView::SetXAxisParameters(const AxisParameters& parameters) {
  m_xAxisParameters = parameters;
  m_xAxisParametersChangedSignal(m_xAxisParameters);
  update();
}

const ChartPlotView::AxisParameters& ChartPlotView::GetYAxisParameters() const {
  return m_yAxisParameters;
}

void ChartPlotView::SetYAxisParameters(const AxisParameters& parameters) {
  m_yAxisParameters = parameters;
  m_yAxisParametersChangedSignal(m_yAxisParameters);
  update();
}

void ChartPlotView::Plot(const std::shared_ptr<ChartPlot>& plot) {
  m_plots.push_back(plot);
  m_plotConnections.add(plot->ConnectUpdateSignal(
    std::bind(&ChartPlotView::OnPlotUpdate, this)));
  update();
}

void ChartPlotView::Clear() {
  m_plots.clear();
  m_plotConnections.disconnect();
  update();
}

connection ChartPlotView::ConnectXAxisParametersChangedSignal(
    const AxisParametersChangedSignal::slot_function_type& slot) const {
  return m_xAxisParametersChangedSignal.connect(slot);
}

connection ChartPlotView::ConnectYAxisParametersChangedSignal(
    const AxisParametersChangedSignal::slot_function_type& slot) const {
  return m_yAxisParametersChangedSignal.connect(slot);
}

connection ChartPlotView::ConnectBeginPanSignal(
    const BeginPanSignal::slot_function_type& slot) const {
  return m_beginPanSignal.connect(slot);
}

connection ChartPlotView::ConnectEndPanSignal(
    const EndPanSignal::slot_function_type& slot) const {
  return m_endPanSignal.connect(slot);
}

std::unique_ptr<WindowSettings> ChartPlotView::GetWindowSettings() const {
  return std::make_unique<ChartPlotViewWindowSettings>(*this);
}

void ChartPlotView::mouseMoveEvent(QMouseEvent* event) {
  if(m_isDragging) {
    auto initial = ComputeChartPoint(m_previousMousePosition);
    auto current = ComputeChartPoint(event->pos());
    m_previousMousePosition = event->pos();
    Drag(std::get<0>(initial) - std::get<0>(current),
      std::get<1>(initial) - std::get<1>(current));
  } else {
    update();
  }
}

void ChartPlotView::mousePressEvent(QMouseEvent* event) {
  if(m_interactionMode == ChartInteractionMode::PAN) {
    m_beginPanSignal();
    setCursor(QCursor(Qt::ClosedHandCursor));
    m_isDragging = true;
    m_previousMousePosition = event->pos();
  } else if(m_interactionMode == ChartInteractionMode::ZOOM) {
    int percentage;
    if(event->button() == Qt::LeftButton) {
      percentage = 80;
    } else if(event->button() == Qt::RightButton) {
      percentage = 125;
    } else {
      return;
    }
    auto mousePoint = ComputeChartPoint(event->pos());
    Zoom(std::get<0>(mousePoint), std::get<1>(mousePoint), percentage);
  }
}

void ChartPlotView::mouseReleaseEvent(QMouseEvent* event) {
  if(m_interactionMode == ChartInteractionMode::PAN) {
    setCursor(QCursor(Qt::OpenHandCursor));
    m_isDragging = false;
    m_endPanSignal();
  }
}

void ChartPlotView::paintEvent(QPaintEvent* event) {
  PaintGrids();
  PaintChartPlots();
  PaintCrossHairs();
}

void ChartPlotView::PaintGrids() {
  QPainter painter(this);
  QPen gridPen(m_properties.m_gridLineColor, 1, Qt::DashLine);
  QPen labelPen(m_properties.m_gridLabelColor);
  auto xScale = GetScale(m_xAxisParameters);
  auto chartWidth = m_xAxisParameters.m_max - m_xAxisParameters.m_min;
  auto gridPoint = m_xAxisParameters.m_min +
    (xScale - (m_xAxisParameters.m_min % xScale));
  QRectF previousTextBox(-100, 0, 0, 0);
  while(true) {
    painter.setPen(gridPen);
    auto topPoint = ComputeScreenPoint(gridPoint, m_yAxisParameters.m_min);
    auto bottomPoint = ComputeScreenPoint(gridPoint, m_yAxisParameters.m_max);
    if(topPoint.x() > GetChartWidth()) {
      break;
    }
    auto label = LoadLabel(gridPoint, *GetXAxisParameters().m_type);
    auto boundingBox = painter.fontMetrics().boundingRect(label);
    QRectF textBox(topPoint.x() - boundingBox.width() / 2, topPoint.y() + 4,
      boundingBox.width() + 4, boundingBox.height());
    if(textBox.x() > previousTextBox.x() + previousTextBox.width()) {
      painter.drawLine(topPoint, bottomPoint);
      painter.setPen(labelPen);
      painter.setFont(m_properties.m_gridLabelFont);
      painter.drawText(textBox, Qt::AlignLeft | Qt::AlignVCenter, label);
      previousTextBox = textBox;
    }
    gridPoint += xScale;
  }
  auto yScale = GetScale(m_yAxisParameters);
  auto chartHeight = m_yAxisParameters.m_max - m_yAxisParameters.m_min;
  gridPoint = m_yAxisParameters.m_min +
    (yScale - (m_yAxisParameters.m_min % yScale));
  previousTextBox = QRectF(0, height() + 100, 0, 0);
  while(true) {
    painter.setPen(gridPen);
    auto leftPoint = ComputeScreenPoint(m_xAxisParameters.m_min, gridPoint);
    auto rightPoint = ComputeScreenPoint(m_xAxisParameters.m_max, gridPoint);
    if(leftPoint.y() <= 0) {
      break;
    }
    auto label = LoadLabel(gridPoint, *GetYAxisParameters().m_type);
    auto boundingBox = painter.fontMetrics().boundingRect(label);
    QRectF textBox(rightPoint.x() + 4,
      rightPoint.y() - boundingBox.height() / 2, boundingBox.width() + 4,
      boundingBox.height());
    if(textBox.y() + textBox.height() < previousTextBox.y()) {
      painter.drawLine(leftPoint, rightPoint);
      painter.setPen(labelPen);
      painter.setFont(m_properties.m_gridLabelFont);
      painter.drawText(textBox, Qt::AlignLeft | Qt::AlignVCenter, label);
      previousTextBox = textBox;
    }
    gridPoint += yScale;
  }
}

void ChartPlotView::wheelEvent(QWheelEvent* event) {
  if(m_interactionMode == ChartInteractionMode::PAN) {
    auto x = m_xAxisParameters.m_min +
      (m_xAxisParameters.m_max - m_xAxisParameters.m_min) / 2;
    auto y = m_yAxisParameters.m_min +
      (m_yAxisParameters.m_max - m_yAxisParameters.m_min) / 2;
    int percentage;
    if(event->angleDelta().y() > 0) {
      percentage = 80;
    } else if(event->angleDelta().y() < 0) {
      percentage = 125;
    } else {
      QWidget::wheelEvent(event);
      return;
    }
    Zoom(x, y, percentage);
    return;
  } else if(m_interactionMode == ChartInteractionMode::ZOOM) {
    if(event->angleDelta().y() > 0) {
      m_beginPanSignal();
      Drag(m_xAxisParameters.m_increment, ChartValue());
      m_endPanSignal();
    } else if(event->angleDelta().y() < 0) {
      m_beginPanSignal();
      Drag(-m_xAxisParameters.m_increment, ChartValue());
      m_endPanSignal();
    } else {
      QWidget::wheelEvent(event);
      return;
    }
    return;
  }
  QWidget::wheelEvent(event);
}

void ChartPlotView::PaintChartPlots() {
  for(const auto& plot : m_plots) {
    if(auto candlestick = dynamic_cast<CandlestickChartPlot*>(plot.get())) {
      PaintCandlestickChartPlot(*candlestick);
    }
  }
}

void ChartPlotView::PaintCrossHairs() {
  auto mousePosition = mapFromGlobal(QCursor::pos());
  if(mousePosition.x() < 0 || mousePosition.x() > GetChartWidth() ||
      mousePosition.y() < 0 || mousePosition.y() > GetChartHeight()) {
    return;
  }
  QPainter painter{this};
  QPen gridPen{CROSSHAIR_COLOR, 1, Qt::DashLine};
  painter.setPen(gridPen);
  auto chartWidth = m_xAxisParameters.m_max - m_xAxisParameters.m_min;
  painter.drawLine(QPoint{mousePosition.x(), 0},
    QPoint{mousePosition.x(), GetChartHeight()});
  painter.drawLine(QPoint{0, mousePosition.y()},
    QPoint{GetChartWidth(), mousePosition.y()});
  auto chartPoint = ComputeChartPoint(mousePosition);
  PaintHorizontalCursor(QPoint{mousePosition.x(), GetChartHeight()},
    std::get<0>(chartPoint));
  PaintVerticalCursor(QPoint{GetChartWidth(), mousePosition.y()},
    std::get<1>(chartPoint));
}

void ChartPlotView::PaintCandlestickChartPlot(
    const CandlestickChartPlot& plot) {
  auto bodyTop = std::max(plot.GetValue().get_open(),
    plot.GetValue().get_close());
  auto bodyBottom = std::min(plot.GetValue().get_open(),
    plot.GetValue().get_close());
  auto bodyTopLeft = ComputeScreenPoint(plot.GetValue().get_start(), bodyTop);
  bodyTopLeft.rx() += 1;
  auto bodyBottomRight = ComputeScreenPoint(plot.GetValue().get_end(),
    bodyBottom);
  bodyBottomRight.rx() -= 1;
  QRect body(bodyTopLeft, bodyBottomRight);
  QBrush bodyBrush(Qt::SolidPattern);
  if(plot.GetValue().get_open() >= plot.GetValue().get_close()) {
    bodyBrush.setColor(m_properties.m_downtickColor);
  } else {
    bodyBrush.setColor(m_properties.m_uptickColor);
  }
  QPainter painter(this);
  painter.setBrush(bodyBrush);
  QPen bodyPen(m_properties.m_outlineColor);
  painter.setPen(bodyPen);
  if(body.right() >= GetChartWidth()) {
    body.setRight(GetChartWidth() - 1);
  }
  if(body.bottom() >= GetChartHeight()) {
    body.setBottom(GetChartHeight() - 1);
  }
  if(body.top() < GetChartHeight() && body.left() < GetChartWidth()) {
    painter.drawRect(body);
  }
  QBrush shadowBrush(m_properties.m_outlineColor, Qt::SolidPattern);
  QPen shadowPen(m_properties.m_outlineColor);
  painter.setBrush(shadowBrush);
  painter.setPen(shadowPen);
  auto barWidth = plot.GetValue().get_end() - plot.GetValue().get_start();
  auto shadowLeft = plot.GetValue().get_start() + (4 * barWidth) / 9;
  auto shadowRight = plot.GetValue().get_start() + (5 * barWidth) / 9;
  auto highShadowTop = plot.GetValue().get_high();
  auto highShadowBottom = bodyTop;
  auto highShadowTopLeft = ComputeScreenPoint(shadowLeft, highShadowTop);
  auto highShadowBottomRight =
    ComputeScreenPoint(shadowRight, highShadowBottom);
  highShadowBottomRight.ry() -= 1;
  if(body.width() <= 3) {
    highShadowBottomRight.rx() = highShadowTopLeft.x();
  }
  QRect highShadow(highShadowTopLeft, highShadowBottomRight);
  if(highShadow.right() >= GetChartWidth()) {
    highShadow.setRight(GetChartWidth() - 1);
  }
  if(highShadow.bottom() >= GetChartHeight()) {
    highShadow.setBottom(GetChartHeight() - 1);
  }
  if(highShadow.top() < GetChartHeight() &&
      highShadow.left() < GetChartWidth()) {
    painter.drawRect(highShadow);
  }
  auto lowShadowTop = bodyBottom;
  auto lowShadowBottom = plot.GetValue().get_low();
  auto lowShadowTopLeft = ComputeScreenPoint(shadowLeft, lowShadowTop);
  auto lowShadowBottomRight = ComputeScreenPoint(shadowRight, lowShadowBottom);
  lowShadowTopLeft.ry() += 1;
  if(body.width() <= 3) {
    lowShadowBottomRight.rx() = lowShadowTopLeft.x();
  }
  QRect lowShadow(lowShadowTopLeft, lowShadowBottomRight);
  if(lowShadow.right() >= GetChartWidth()) {
    lowShadow.setRight(GetChartWidth() - 1);
  }
  if(lowShadow.bottom() >= GetChartHeight()) {
    lowShadow.setBottom(GetChartHeight() - 1);
  }
  if(lowShadow.top() < GetChartHeight() && lowShadow.left() < GetChartWidth()) {
    painter.drawRect(lowShadow);
  }
}

void ChartPlotView::PaintHorizontalCursor(const QPoint& position,
    const ChartValue& value) {
  const auto WIDTH = scale_width(55);
  const auto HEIGHT = height() - position.y();
  QPainter painter{this};
  QPoint bodyTopLeft{position.x() - WIDTH / 2, position.y()};
  QPoint bodyBottomRight{position.x() + WIDTH / 2, position.y() + HEIGHT};
  QRect body{bodyTopLeft, bodyBottomRight};
  QBrush bodyBrush{Qt::SolidPattern};
  bodyBrush.setColor(QColor{155, 155, 155});
  painter.setBrush(bodyBrush);
  painter.drawRect(body);
  QRectF textBox{static_cast<qreal>(body.x()), static_cast<qreal>(body.y()),
    static_cast<qreal>(body.width()), static_cast<qreal>(body.height())};
  painter.drawText(textBox, Qt::AlignHCenter | Qt::AlignVCenter,
    LoadLabel(value, *GetXAxisParameters().m_type));
}

void ChartPlotView::PaintVerticalCursor(const QPoint& position,
    const ChartValue& value) {
  const auto WIDTH = width() - position.x();
  const auto HEIGHT = scale_height(16);
  QPainter painter{this};
  QPoint bodyTopLeft{position.x(), position.y() - HEIGHT / 2};
  QPoint bodyBottomRight{position.x() + WIDTH, position.y() + HEIGHT / 2};
  QRect body{bodyTopLeft, bodyBottomRight};
  QBrush bodyBrush{Qt::SolidPattern};
  bodyBrush.setColor(QColor{155, 155, 155});
  painter.setBrush(bodyBrush);
  painter.drawRect(body);
  QRectF textBox{static_cast<qreal>(body.x() + 4), static_cast<qreal>(body.y()),
    static_cast<qreal>(body.width()), static_cast<qreal>(body.height())};
  painter.drawText(textBox, Qt::AlignLeft | Qt::AlignVCenter,
    LoadLabel(value, *GetYAxisParameters().m_type));
}

void ChartPlotView::Zoom(ChartValue x, ChartValue y, int percentage) {
  auto xRange = (percentage *
    (m_xAxisParameters.m_max - m_xAxisParameters.m_min)) / 100;
  auto yRange = (percentage *
    (m_yAxisParameters.m_max - m_yAxisParameters.m_min)) / 100;
  m_xAxisParameters.m_min = x - (xRange / 2);
  m_xAxisParameters.m_max = x + (xRange / 2);
  m_yAxisParameters.m_min = y - (yRange / 2);
  m_yAxisParameters.m_max = y + (yRange / 2);
  m_xAxisParametersChangedSignal(m_xAxisParameters);
  m_yAxisParametersChangedSignal(m_yAxisParameters);
  update();
}

void ChartPlotView::Drag(ChartValue xIncrement, ChartValue yIncrement) {
  m_xAxisParameters.m_min += xIncrement;
  m_xAxisParameters.m_max += xIncrement;
  m_yAxisParameters.m_min += yIncrement;
  m_yAxisParameters.m_max += yIncrement;
  m_xAxisParametersChangedSignal(m_xAxisParameters);
  m_yAxisParametersChangedSignal(m_yAxisParameters);
  update();
}

std::tuple<ChartValue, ChartValue> ChartPlotView::ComputeChartPoint(
    const QPoint& point) {
  auto xRange = m_xAxisParameters.m_max - m_xAxisParameters.m_min;
  auto xValue = m_xAxisParameters.m_min +
    (point.x() * xRange) / GetChartWidth();
  auto yRange = m_yAxisParameters.m_max - m_yAxisParameters.m_min;
  auto yValue = m_yAxisParameters.m_max -
    (point.y() * yRange) / GetChartHeight();
  return std::make_tuple(xValue, yValue);
}

QPoint ChartPlotView::ComputeScreenPoint(ChartValue x, ChartValue y) {
  auto deltaX = x - m_xAxisParameters.m_min;
  auto deltaXRange = m_xAxisParameters.m_max - m_xAxisParameters.m_min;
  auto xPoint = static_cast<int>((GetChartWidth() * deltaX) / deltaXRange);
  auto deltaY = y - m_yAxisParameters.m_min;
  auto deltaYRange = m_yAxisParameters.m_max - m_yAxisParameters.m_min;
  auto yPoint = GetChartHeight() -
    static_cast<int>((GetChartHeight() * deltaY) / deltaYRange);
  return QPoint(xPoint, yPoint);
}

QString ChartPlotView::LoadLabel(ChartValue value,
    const NativeType& type) const {
  if(type.GetCompatibility(MoneyType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    auto v = value.ToMoney();
    if(v >= Money::ONE) {
      v = floor_to(v, Money::CENT / 1000);
    } else {
      v = floor_to(v, Money::CENT / 10000);
    }
    return QString::fromStdString(lexical_cast<string>(v));
  } else if(type.GetCompatibility(DateTimeType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    auto v = to_local_time(value.ToDateTime());
    auto s = to_simple_string(v).substr(12, 8);
    if(v.time_of_day().seconds() == 0) {
      s = s.substr(0, 5);
    }
    return QString::fromStdString(s);
  } else if(type.GetCompatibility(IntegerType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    return QString::number(static_cast<int>(value.ToQuantity()));
  }
  return QString();
}

int ChartPlotView::GetChartWidth() const {
  static const int LABEL_WIDTH = scale_width(40);
  return width() - LABEL_WIDTH;
}

int ChartPlotView::GetChartHeight() const {
  static const int LABEL_HEIGHT = scale_height(20);
  return height() - LABEL_HEIGHT;
}

void ChartPlotView::OnPlotUpdate() {
  update();
}
