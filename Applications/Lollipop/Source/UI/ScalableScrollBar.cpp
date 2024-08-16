#include "Spire/UI/ScalableScrollBar.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <QHBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QToolButton>
#include <QVBoxLayout>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::UI;

ScalableScrollBar::ScalableScrollBar(QWidget* parent)
    : QWidget(parent),
      m_layout(nullptr),
      m_scaleLayout(nullptr),
      m_direction(1),
      m_minimum(0),
      m_maximum(100),
      m_sliderMinimum(50),
      m_sliderMaximum(50),
      m_singleStep(1),
      m_pageStep(10),
      m_orientation(Qt::Orientation::Horizontal),
      m_mouseTarget(nullptr),
      m_pageScrollDirection(0) {
  m_decrementButton = new QToolButton(this);
  m_decrementButton->setMinimumSize(QSize(16, 16));
  m_decrementButton->setMaximumSize(QSize(16, 16));
  m_decrementButton->setAutoRepeat(true);
  m_incrementButton = new QToolButton(this);
  m_incrementButton->setMinimumSize(QSize(16, 16));
  m_incrementButton->setMaximumSize(QSize(16, 16));
  m_incrementButton->setAutoRepeat(true);
  m_scaleBackground = new QWidget(this);
  m_scaleBackground->setAutoFillBackground(true);
  m_scaleHandle = new QWidget(m_scaleBackground);
  m_topScaleHandle = new QFrame(m_scaleHandle);
  m_topScaleHandle->setMouseTracking(true);
  m_topScaleHandle->setFrameShape(QFrame::WinPanel);
  m_topScaleHandle->setFrameShadow(QFrame::Raised);
  m_topScaleHandle->setLineWidth(1);
  m_topScaleHandle->setMidLineWidth(0);
  m_scaleBody = new QFrame(m_scaleHandle);
  m_scaleBody->setFrameShape(QFrame::WinPanel);
  m_scaleBody->setFrameShadow(QFrame::Raised);
  m_scaleBody->setStyleSheet("background-color: rgb(165, 165, 165);");
  m_bottomScaleHandle = new QFrame(m_scaleHandle);
  m_bottomScaleHandle->setMouseTracking(true);
  m_bottomScaleHandle->setFrameShape(QFrame::WinPanel);
  m_bottomScaleHandle->setFrameShadow(QFrame::Raised);
  m_bottomScaleHandle->setLineWidth(1);
  m_bottomScaleHandle->setMidLineWidth(0);
  m_scaleHandle->installEventFilter(this);
  m_topScaleHandle->installEventFilter(this);
  m_bottomScaleHandle->installEventFilter(this);
  m_scaleBackground->installEventFilter(this);
  m_pageScrollTimer.setSingleShot(true);
  connect(&m_pageScrollTimer, &QTimer::timeout, this,
    &ScalableScrollBar::OnPageScrollTimeout);
  connect(m_incrementButton, &QToolButton::pressed, this,
    &ScalableScrollBar::OnIncrementButtonPressed);
  connect(m_decrementButton, &QToolButton::pressed, this,
    &ScalableScrollBar::OnDecrementButtonPressed);
  SetOrientation(Qt::Vertical);
  Refresh();
}

ScalableScrollBar::~ScalableScrollBar() {}

bool ScalableScrollBar::IsInverted() const {
  return m_direction == -1;
}

void ScalableScrollBar::SetInverted(bool inverted) {
  int direction;
  if(inverted) {
    direction = -1;
  } else {
    direction = 1;
  }
  if(direction == m_direction) {
    return;
  }
  m_direction = direction;
  Refresh();
}

int ScalableScrollBar::GetMinimum() const {
  return m_minimum;
}

void ScalableScrollBar::SetMinimum(int minimum) {
  if(minimum == m_minimum) {
    return;
  }
  m_minimum = minimum;
  m_rangeChangedSignal(m_minimum, m_maximum);
}

int ScalableScrollBar::GetMaximum() const {
  return m_maximum;
}

void ScalableScrollBar::SetMaximum(int maximum) {
  if(maximum == m_maximum) {
    return;
  }
  m_maximum = maximum;
  m_rangeChangedSignal(m_minimum, m_maximum);
}

int ScalableScrollBar::GetSliderMinimum() const {
  return m_sliderMinimum;
}

void ScalableScrollBar::SetSliderMinimum(int minimum) {
  minimum = std::max(minimum, m_minimum);
  if(m_sliderMinimum == minimum) {
    return;
  }
  int previousSliderMinimum = m_sliderMinimum;
  m_sliderMinimum = minimum;
  m_sliderChangedSignal(previousSliderMinimum, GetSliderMaximum(),
    GetSliderMinimum(), GetSliderMaximum());
  Refresh();
}

int ScalableScrollBar::GetSliderMaximum() const {
  return m_sliderMaximum;
}

void ScalableScrollBar::SetSliderMaximum(int maximum) {
  maximum = std::min(maximum, m_maximum);
  if(m_sliderMaximum == maximum) {
    return;
  }
  int previousSliderMaximum = m_sliderMaximum;
  m_sliderMaximum = maximum;
  m_sliderChangedSignal(GetSliderMinimum(), previousSliderMaximum,
    GetSliderMinimum(), GetSliderMaximum());
  Refresh();
}

void ScalableScrollBar::SetSliderRange(int minimum, int maximum) {
  minimum = std::max(minimum, m_minimum);
  maximum = std::min(maximum, m_maximum);
  if(m_sliderMinimum == minimum && m_sliderMaximum == maximum) {
    return;
  }
  int previousSliderMinimum = m_sliderMinimum;
  int previousSliderMaximum = m_sliderMaximum;
  m_sliderMinimum = minimum;
  m_sliderMaximum = maximum;
  m_sliderChangedSignal(previousSliderMinimum, previousSliderMaximum,
    GetSliderMinimum(), GetSliderMaximum());
  Refresh();
}

int ScalableScrollBar::GetSingleStep() const {
  return m_singleStep;
}

void ScalableScrollBar::SetSingleStep(int value) {
  m_singleStep = value;
}

int ScalableScrollBar::GetPageStep() const {
  return m_pageStep;
}

void ScalableScrollBar::SetPageStep(int value) {
  m_pageStep = value;
}

Qt::Orientation ScalableScrollBar::GetOrientation() const {
  return m_orientation;
}

void ScalableScrollBar::SetOrientation(Qt::Orientation orientation) {
  if(m_orientation == orientation) {
    return;
  }
  m_orientation = orientation;
  delete m_layout;
  delete m_scaleLayout;
  QSizePolicy sizePolicy;
  if(orientation == Qt::Vertical) {
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(
      m_scaleBackground->sizePolicy().hasHeightForWidth());
    m_scaleBackground->setSizePolicy(sizePolicy);
    m_scaleBackground->setMinimumSize(QSize(16, 0));
    m_scaleBackground->setMaximumSize(QSize(16, 16777215));
    m_scaleLayout = new QVBoxLayout(m_scaleHandle);
    m_scaleLayout->setSpacing(0);
    m_scaleLayout->setContentsMargins(0, 0, 0, 0);
    QSizePolicy scaleSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scaleSizePolicy.setHorizontalStretch(0);
    scaleSizePolicy.setVerticalStretch(0);
    scaleSizePolicy.setHeightForWidth(
      m_topScaleHandle->sizePolicy().hasHeightForWidth());
    m_topScaleHandle->setSizePolicy(scaleSizePolicy);
    m_topScaleHandle->setMinimumSize(QSize(16, 6));
    m_topScaleHandle->setMaximumSize(QSize(16, 6));
    m_bottomScaleHandle->setSizePolicy(scaleSizePolicy);
    m_bottomScaleHandle->setMinimumSize(QSize(16, 6));
    m_bottomScaleHandle->setMaximumSize(QSize(16, 6));
    sizePolicy.setHeightForWidth(m_scaleBody->sizePolicy().hasHeightForWidth());
    m_scaleBody->setSizePolicy(sizePolicy);
    m_scaleBody->setMinimumSize(QSize(16, 0));
    m_scaleBody->setMaximumSize(QSize(16, 16777215));
    m_decrementButton->setArrowType(Qt::UpArrow);
    m_incrementButton->setArrowType(Qt::DownArrow);
    m_scaleBackground->setFixedWidth(16);
    m_scaleHandle->setFixedWidth(16);
    setFixedWidth(16);
    m_topScaleHandle->setCursor(Qt::SizeVerCursor);
    m_bottomScaleHandle->setCursor(Qt::SizeVerCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  } else {
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setWidthForHeight(
      m_scaleBackground->sizePolicy().hasWidthForHeight());
    m_scaleBackground->setSizePolicy(sizePolicy);
    m_scaleBackground->setMinimumSize(QSize(0, 16));
    m_scaleBackground->setMaximumSize(QSize(16777215, 16));
    m_scaleLayout = new QHBoxLayout(m_scaleHandle);
    m_scaleLayout->setSpacing(0);
    m_scaleLayout->setContentsMargins(0, 0, 0, 0);
    QSizePolicy scaleSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scaleSizePolicy.setHorizontalStretch(0);
    scaleSizePolicy.setVerticalStretch(0);
    scaleSizePolicy.setWidthForHeight(
      m_topScaleHandle->sizePolicy().hasWidthForHeight());
    m_topScaleHandle->setSizePolicy(scaleSizePolicy);
    m_topScaleHandle->setMinimumSize(QSize(6, 16));
    m_topScaleHandle->setMaximumSize(QSize(6, 16));
    m_bottomScaleHandle->setSizePolicy(scaleSizePolicy);
    m_bottomScaleHandle->setMinimumSize(QSize(6, 16));
    m_bottomScaleHandle->setMaximumSize(QSize(6, 16));
    sizePolicy.setWidthForHeight(m_scaleBody->sizePolicy().hasWidthForHeight());
    m_scaleBody->setSizePolicy(sizePolicy);
    m_scaleBody->setMinimumSize(QSize(0, 16));
    m_scaleBody->setMaximumSize(QSize(16777215, 16));
    m_decrementButton->setArrowType(Qt::LeftArrow);
    m_incrementButton->setArrowType(Qt::RightArrow);
    m_scaleBackground->setFixedHeight(16);
    m_scaleHandle->setFixedHeight(16);
    setFixedHeight(16);
    m_topScaleHandle->setCursor(Qt::SizeHorCursor);
    m_bottomScaleHandle->setCursor(Qt::SizeHorCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  }
  m_scaleLayout->addWidget(m_topScaleHandle);
  m_scaleLayout->addWidget(m_scaleBody);
  m_scaleLayout->addWidget(m_bottomScaleHandle);
  m_layout->addWidget(m_decrementButton);
  m_layout->addWidget(m_scaleBackground);
  m_layout->addWidget(m_incrementButton);
}

void ScalableScrollBar::AdjustHandle(int offset) {
  if(offset >= 0) {
    offset = std::min(offset, GetSliderMinimum() - GetMinimum());
  } else {
    offset = std::max(offset, GetSliderMaximum() - GetMaximum());
  }
  int currentSliderMinimum = GetSliderMinimum();
  int currentSliderMaximum = GetSliderMaximum();
  SetSliderRange(currentSliderMinimum - offset, currentSliderMaximum - offset);
}

connection ScalableScrollBar::ConnectRangeChangedSignal(
    const RangeChangedSignal::slot_function_type& slot) const {
  return m_rangeChangedSignal.connect(slot);
}

connection ScalableScrollBar::ConnectSliderChangedSignal(
    const SliderChangedSignal::slot_function_type& slot) const {
  return m_sliderChangedSignal.connect(slot);
}

bool ScalableScrollBar::eventFilter(QObject* object, QEvent* event) {
  if(object == m_scaleBackground) {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(mouseEvent == nullptr) {
      return QWidget::eventFilter(object, event);
    }
    return ScaleBackgroundMouseEventFilter(*mouseEvent);
  } else if(object == m_scaleHandle || object == m_topScaleHandle ||
      object == m_bottomScaleHandle) {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(mouseEvent == nullptr) {
      return QWidget::eventFilter(object, event);
    }
    return ScaleHandleMouseEventFilter(*object, *mouseEvent);
  }
  return QWidget::eventFilter(object, event);
}

void ScalableScrollBar::resizeEvent(QResizeEvent* event) {
  Refresh();
}

int ScalableScrollBar::GetHandleLength() const {
  if(GetOrientation() == Qt::Vertical) {
    return m_topScaleHandle->frameGeometry().height();
  }
  return m_topScaleHandle->frameGeometry().width();
}

int ScalableScrollBar::PointToScale(const QPoint& point) const {
  int visualRange;
  int coordinate;
  if(GetOrientation() == Qt::Vertical) {
    visualRange = m_scaleBackground->height() - 2 * GetHandleLength();
    coordinate = point.y() - GetHandleLength();
  } else {
    visualRange = m_scaleBackground->width() - 2 * GetHandleLength();
    coordinate = point.x() - GetHandleLength();
  }
  int scaleRange = GetMaximum() - GetMinimum();
  int scaleValue;
  if(IsInverted()) {
    scaleValue = GetMaximum() - (coordinate * scaleRange) / visualRange;
  } else {
    scaleValue = GetMinimum() + (coordinate * scaleRange) / visualRange;
  }
  return scaleValue;
}

QPoint ScalableScrollBar::ScaleToPoint(int value) const {
  int visualRange;
  if(GetOrientation() == Qt::Vertical) {
    visualRange = m_scaleBackground->height() - 2 * GetHandleLength();
  } else {
    visualRange = m_scaleBackground->width() - 2 * GetHandleLength();
  }
  int scaleRange = GetMaximum() - GetMinimum();
  int position;
  if(IsInverted()) {
    position = GetHandleLength() + (visualRange * (GetMaximum() - value)) /
      scaleRange;
  } else {
    position = GetHandleLength() + (visualRange * (value - GetMinimum())) /
      scaleRange;
  }
  QPoint point;
  if(GetOrientation() == Qt::Vertical) {
    point = QPoint(0, position);
  } else {
    point = QPoint(position, 0);
  }
  return point;
}

void ScalableScrollBar::Refresh() {
  QPoint topPoint;
  if(IsInverted()) {
    topPoint = ScaleToPoint(GetSliderMaximum());
  } else {
    topPoint = ScaleToPoint(GetSliderMinimum());
  }
  if(GetOrientation() == Qt::Vertical) {
    topPoint.ry() -= GetHandleLength();
  } else {
    topPoint.rx() -= GetHandleLength();
  }
  m_scaleHandle->move(topPoint);
  QPoint bottomPoint;
  if(IsInverted()) {
     bottomPoint = ScaleToPoint(GetSliderMinimum());
  } else {
     bottomPoint = ScaleToPoint(GetSliderMaximum());
  }
  if(GetOrientation() == Qt::Vertical) {
    int height = bottomPoint.y() - topPoint.y() + GetHandleLength();
    m_scaleHandle->setFixedHeight(height);
  } else {
    int width = bottomPoint.x() - topPoint.x() + GetHandleLength();
    m_scaleHandle->setFixedWidth(width);
  }
}

void ScalableScrollBar::PageScroll() {
  int mousePosition;
  int topHandlePosition;
  int bottomHandlePosition;
  QPoint position = QCursor::pos();
  if(GetOrientation() == Qt::Vertical) {
    mousePosition = position.y();
    topHandlePosition = m_topScaleHandle->mapToGlobal(QPoint(0, 0)).y();
    bottomHandlePosition = m_bottomScaleHandle->mapToGlobal(
      QPoint(0, m_bottomScaleHandle->height())).y();
  } else {
    mousePosition = position.x();
    topHandlePosition = m_topScaleHandle->mapToGlobal(QPoint(0, 0)).x();
    bottomHandlePosition = m_bottomScaleHandle->mapToGlobal(
      QPoint(m_bottomScaleHandle->width(), 0)).x();
  }
  int direction;
  if(mousePosition < topHandlePosition) {
    if(IsInverted()) {
      direction = -1;
    } else {
      direction = 1;
    }
  } else if(mousePosition > bottomHandlePosition) {
    if(IsInverted()) {
      direction = 1;
    } else {
      direction = -1;
    }
  } else {
    return;
  }
  if(m_pageScrollDirection == 0) {
    m_pageScrollDirection = direction;
  }
  if(direction == m_pageScrollDirection) {
    AdjustHandle(direction * GetPageStep());
  }
}

bool ScalableScrollBar::ScaleBackgroundMouseEventFilter(QMouseEvent& event) {
  if(m_pageScrollDirection != 0) {
    if(event.type() == QEvent::MouseButtonRelease &&
        event.button() == Qt::LeftButton) {
      m_pageScrollDirection = 0;
      return true;
    }
    return QWidget::eventFilter(m_scaleBackground, &event);
  } else {
    if(event.type() == QEvent::MouseButtonPress &&
        event.button() == Qt::LeftButton) {
      PageScroll();
      m_pageScrollTimer.start(300);
      return true;
    }
    return QWidget::eventFilter(m_scaleBackground, &event);
  }
}

bool ScalableScrollBar::ScaleHandleMouseEventFilter(QObject& object,
    QMouseEvent& event) {
  if(event.type() == QEvent::MouseButtonPress) {
    if(event.button() != Qt::LeftButton) {
      return QWidget::eventFilter(&object, &event);
    }
    m_mouseTarget = &object;
    m_mouseValue = PointToScale(m_scaleBackground->mapFromGlobal(
      event.globalPos()));
    return true;
  }
  if(m_mouseTarget != &object) {
    m_mouseTarget = nullptr;
    return QWidget::eventFilter(&object, &event);
  }
  if(event.type() == QEvent::MouseButtonRelease) {
    m_mouseTarget = nullptr;
    return true;
  } else if(event.type() == QEvent::MouseMove) {
    if(m_mouseTarget == nullptr) {
      return true;
    }
    if(m_mouseValue > GetMaximum() || m_mouseValue < GetMinimum()) {
      m_mouseValue = PointToScale(m_scaleBackground->mapFromGlobal(
        event.globalPos()));
      return true;
    }
    if(&object == m_scaleHandle) {
      int mouseValue = PointToScale(m_scaleBackground->mapFromGlobal(
        event.globalPos()));
      int scaleDelta = m_mouseValue - mouseValue;
      m_mouseValue = mouseValue;
      AdjustHandle(scaleDelta);
      return true;
    } else if(&object == m_topScaleHandle) {
      if(IsInverted()) {
        int coordinate = std::max(PointToScale(m_scaleBackground->mapFromGlobal(
          event.globalPos())), GetSliderMinimum());
        if(coordinate >= GetMaximum()) {
          m_mouseTarget = nullptr;
        }
        SetSliderMaximum(coordinate);
      } else {
        int coordinate = std::min(PointToScale(m_scaleBackground->mapFromGlobal(
          event.globalPos())), GetSliderMaximum());
        if(coordinate <= GetMinimum()) {
          m_mouseTarget = nullptr;
        }
        SetSliderMinimum(coordinate);
      }
      return true;
    } else if(&object == m_bottomScaleHandle) {
      if(IsInverted()) {
        int coordinate = std::min(PointToScale(m_scaleBackground->mapFromGlobal(
          event.globalPos())), GetSliderMaximum());
        if(coordinate <= GetMinimum()) {
          m_mouseTarget = nullptr;
        }
        SetSliderMinimum(coordinate);
      } else {
        int coordinate = std::max(PointToScale(m_scaleBackground->mapFromGlobal(
          event.globalPos())), GetSliderMinimum());
        if(coordinate >= GetMaximum()) {
          m_mouseTarget = nullptr;
        }
        SetSliderMaximum(coordinate);
      }
      return true;
    }
  }
  return QWidget::eventFilter(&object, &event);
}

void ScalableScrollBar::OnPageScrollTimeout() {
  if(m_pageScrollDirection == 0) {
    return;
  }
  QPoint position = m_scaleBackground->mapFromGlobal(QCursor::pos());
  if(GetOrientation() == Qt::Vertical &&
      position.x() >= 0 && position.x() <= m_scaleBackground->width() ||
      GetOrientation() == Qt::Horizontal &&
      position.y() >= 0 && position.y() <= m_scaleBackground->height()) {
    PageScroll();
  }
  m_pageScrollTimer.start(50);
}

void ScalableScrollBar::OnIncrementButtonPressed() {
  int direction;
  if(IsInverted()) {
    direction = 1;
  } else {
    direction = -1;
  }
  AdjustHandle(direction * GetSingleStep());
}

void ScalableScrollBar::OnDecrementButtonPressed() {
  int direction;
  if(IsInverted()) {
    direction = -1;
  } else {
    direction = 1;
  }
  AdjustHandle(direction * GetSingleStep());
}
