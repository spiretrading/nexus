#include "Spire/Ui/Ui.hpp"
#include <QIcon>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"

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

CheckBox* Spire::make_check_box(const QString& label, QWidget* parent) {
  auto check_box = new CheckBox(label, parent);
  auto check_box_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;
    outline: none;
    spacing: %2px;)")
    .arg(scale_height(12)).arg(scale_width(4));
  auto check_box_indicator_style = QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    height: %3px;
    width: %4px;)").arg(scale_height(1))
    .arg(scale_width(1)).arg(scale_height(15)).arg(scale_width(15));
  auto check_box_checked_style = QString(R"(
    image: url(:/Icons/check-with-box.svg);)");
  auto check_box_hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto check_box_focused_style = QString(R"(border-color: #4B23A0;)");
  check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  return check_box;
}

FlatButton* Spire::make_flat_button(const QString& label, QWidget* parent) {
  auto button = new FlatButton(label, parent);
  auto button_style = button->get_style();
  button_style.m_background_color = QColor("#EBEBEB");
  auto button_hover_style = button->get_hover_style();
  button_hover_style.m_background_color = QColor("#4B23A0");
  button_hover_style.m_text_color = Qt::white;
  auto button_focus_style = button->get_focus_style();
  button_focus_style.m_background_color = QColor("#EBEBEB");
  button_focus_style.m_border_color = QColor("#4B23A0");
  auto disabled_style = button->get_disabled_style();
  disabled_style.m_text_color = QColor("#A0A0A0");
  disabled_style.m_border_color = QColor("#C8C8C8");
  disabled_style.m_background_color = QColor("#C8C8C8");
  auto button_font = QFont();
  button_font.setFamily("Roboto");
  button_font.setPixelSize(scale_height(12));
  button->setFont(button_font);
  button->set_style(button_style);
  button->set_disabled_style(disabled_style);
  button->set_hover_style(button_hover_style);
  button->set_focus_style(button_focus_style);
  return button;
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
