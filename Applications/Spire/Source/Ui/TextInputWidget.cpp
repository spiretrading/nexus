#include "Spire/Ui/TextInputWidget.hpp"
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

TextInputWidget::TextInputWidget(QWidget* parent)
  : TextInputWidget({}, parent) {}

TextInputWidget::TextInputWidget(QString text, QWidget* parent)
    : QLineEdit(std::move(text), parent) {
  setContextMenuPolicy(Qt::NoContextMenu);
  set_default_style();
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
  painter.setPen(QColor("#C8C8C8"));
  painter.drawRect(0, 0, width() - 1, height() - 1);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  auto metrics = QFontMetrics(font);
  auto elided_text = metrics.elidedText(text(), Qt::ElideRight,
    width() - scale_width(16));
  painter.setPen(Qt::black);
  //painter.drawText(QPoint(0, 0), elided_text);
  //auto vscroll = r.y() + (r.height() - fm.height() + 1) / 2;
  auto r = QRect(9 + 2, 1 + (26 - metrics.height() + 1) / 2,
    width() - 2 - (2 * 2), metrics.height());
  painter.drawText(r, elided_text);
}

void TextInputWidget::set_cell_style() {
  setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;
  )").arg(scale_height(12)).arg(scale_width(5)));
}

void TextInputWidget::set_default_style() {
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
        .arg(scale_width(8)));
}
