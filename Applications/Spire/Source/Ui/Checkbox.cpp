#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QStylePainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

namespace {
  const auto& CHECK_ICON() {
    static auto icon = imageFromSvg(":/Icons/check.svg", scale(16, 16));
    return icon;
  }

  const auto& CHECK_DISABLED_COLOR() {
    static auto color = QColor("#C8C8C8");
    return color;
  }
}

Checkbox::Checkbox(QWidget* parent)
  : QCheckBox("", parent) {}

Checkbox::Checkbox(const QString& label, QWidget* parent)
    : QCheckBox(label, parent),
      m_is_read_only(false) {
  setObjectName("Checkbox");
  setStyleSheet(QString(R"(
    #Checkbox {
      background-color: #00000000;
      color: #000000;
      font-family: "Roboto";
      font-size: %1px;
      outline: none;
    }
    #Checkbox::indicator {
      background-color: #FFFFFF;
      border: %2px solid #C8C8C8;
      height: %3px;
      width: %4px;
    }
    #Checkbox::indicator:hover {
      border-color: #4B23A0;
    }
    #Checkbox::indicator:focus {
      border-color: #4B23A0;
    }
    #Checkbox::indicator:read-only {
      background-color: #00000000;
      border-color: #00000000;
    }
    #Checkbox:disabled {
      color: #C8C8C8;
    }
    #Checkbox::indicator:disabled {
      background-color: #F5F5F5;
    }
    #Checkbox::indicator:read-only:disabled {
      background-color: #00000000;
      border-color: #00000000;
    })").arg(scale_width(12)).arg(scale_width(1)).
    arg(scale_height(16)).arg(scale_width(16)));
}

void Checkbox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    if(m_is_read_only) {
      setAttribute(Qt::WA_TransparentForMouseEvents, true);
      setFocusPolicy(Qt::NoFocus);
    } else {
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
      setFocusPolicy(Qt::StrongFocus);
    }
    update();
  }
}

void Checkbox::paintEvent(QPaintEvent* event) {
  auto painter = QStylePainter(this);
  auto option = QStyleOptionButton();
  initStyleOption(&option);
  if(m_is_read_only) {
    option.state |= QStyle::State_ReadOnly;
  }
  auto style = painter.style();
  auto indicator_rect = style->subElementRect(QStyle::SE_CheckBoxIndicator,
    &option, this);
  option.rect = indicator_rect;
  painter.drawPrimitive(QStyle::PE_IndicatorCheckBox, option);
  if(isChecked()) {
    auto icon = QPixmap::fromImage(CHECK_ICON());
    if(!isEnabled()) {
      auto image_painter = QPainter(&icon);
      image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
      image_painter.fillRect(icon.rect(), CHECK_DISABLED_COLOR());
    }
    painter.drawItemPixmap(indicator_rect, Qt::AlignCenter, icon);
  }
  auto alignment = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
  auto font_metrics = fontMetrics();
  auto label_width = style->itemTextRect(font_metrics, QRect(), alignment, true,
    text()).width();
  auto contents_rect = contentsRect();
  indicator_rect = style->visualRect(option.direction, contents_rect,
    indicator_rect);
  auto label_x = contents_rect.width() - label_width;
  if(label_x <= indicator_rect.right()) {
    label_x = indicator_rect.right() + 1;
    label_width = contents_rect.right() - indicator_rect.right();
  }
  auto label_rect = QRect(label_x, contents_rect.y(), label_width,
    contents_rect.height());
  label_rect = style->visualRect(option.direction, contents_rect, label_rect);
  painter.drawItemText(label_rect, alignment, palette(), true,
    font_metrics.elidedText(text(), Qt::ElideRight, label_rect.width(),
    Qt::TextShowMnemonic));
}
