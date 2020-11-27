#include "Spire/Ui/TextInputWidget.hpp"
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

namespace {
  const auto BORDER_SIZE = 1;
  const auto HORIZONTAL_MARGIN = 2;

  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

TextInputWidget::TextInputWidget(QWidget* parent)
  : TextInputWidget({}, parent) {}

TextInputWidget::TextInputWidget(QString text, QWidget* parent)
    : QLineEdit(std::move(text), parent) {
  setAttribute(Qt::WA_Hover);
  setContextMenuPolicy(Qt::NoContextMenu);
  setStyleSheet(QString(R"(
    QLineEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %4px;
    }

    QLineEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12))
        .arg(PADDING()));
}

void TextInputWidget::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::OtherFocusReason) {
    selectAll();
  }
  QLineEdit::focusInEvent(event);
}

void TextInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Delete:
      setText("");
    case Qt::Key_Enter:
    case Qt::Key_Return:
      Q_EMIT editingFinished();
    case Qt::Key_Up:
    case Qt::Key_Down:
      return;
  }
  QLineEdit::keyPressEvent(event);
}

void TextInputWidget::paintEvent(QPaintEvent* event) {
  if(hasFocus()) {
    QLineEdit::paintEvent(event);
    return;
  }
  auto painter = QPainter(this);
  painter.fillRect(rect(), Qt::white);
  if(underMouse() && isEnabled()) {
    painter.setPen(QColor("#4B23A0"));
  } else {
    painter.setPen(QColor("#C8C8C8"));
  }
  painter.drawRect(0, 0, width() - 1, height() - 1);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  auto metrics = QFontMetrics(font);
  auto elided_text = metrics.elidedText(text(), Qt::ElideRight,
    width() - scale_width(16));
  painter.setPen(Qt::black);
  auto text_rect = QRect(PADDING() + BORDER_SIZE + HORIZONTAL_MARGIN,
    BORDER_SIZE + ((height() - BORDER_SIZE - metrics.height()) / 2),
    width() - (2 * BORDER_SIZE) - (2 * HORIZONTAL_MARGIN), metrics.height());
  painter.drawText(text_rect, elided_text);
}
