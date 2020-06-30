#include "Spire/Ui/Ui.hpp"
#include <QIcon>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  // TODO: Revert this when Qt fixes the regression.
  // https://bugreports.qt.io/browse/QTBUG-81259
  auto svg_pixmap = QIcon(path).pixmap(box.width(), box.height());
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  auto painter = QPainter(&image);
  painter.drawPixmap(box.topLeft(), svg_pixmap);
  return image;
}

void Spire::apply_line_edit_style(QLineEdit* widget) {
  widget->setStyleSheet(QString(R"(
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
        .arg(scale_width(6)));
}

void Spire::apply_spin_box_style(QSpinBox* widget) {
  widget->setStyleSheet(QString(R"(
    QSpinBox {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %5px;
      padding-left: %4px;
    }

    QSpinBox:focus {
      border: %1px solid #4B23A0;
    }

    QSpinBox::up-button {
      border: none;
    }

    QSpinBox::down-button {
      border: none;
    }

    QSpinBox::up-arrow {
      height: %2px;
      image: url(:/Icons/arrow-up.svg);
      padding-top: %6px;
      width: %3px;
    }

    QSpinBox::down-arrow {
      height: %2px;
      image: url(:/Icons/arrow-down.svg);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(6)).arg(scale_width(6))
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(4)));
  widget->setContextMenuPolicy(Qt::NoContextMenu);
}

QHeaderView* Spire::make_fixed_header(QWidget* parent) {
  auto header = new QHeaderView(Qt::Horizontal, parent);
  header->setFixedHeight(scale_height(30));
  header->setStretchLastSection(true);
  header->setSectionsClickable(false);
  header->setSectionsMovable(false);
  header->setSectionResizeMode(QHeaderView::Fixed);
  header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}

QHeaderView* Spire::make_header(QWidget* parent) {
  auto header = make_fixed_header(parent);
  header->setSectionsMovable(true);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setStyleSheet(QString(R"(
    QHeaderView {
      padding-left: %1px;
    }

    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}
