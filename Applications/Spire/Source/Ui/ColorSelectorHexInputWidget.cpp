#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRegExpValidator>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto FONT_HEIGHT() {
    static auto height = scale_height(12);
    return height;
  }

  auto color_name(const QColor& color) {
    return color.name().toUpper().remove('#');
  }
}

ColorSelectorHexInputWidget::ColorSelectorHexInputWidget(
    const QColor& current_color, QWidget* parent)
    : QWidget(parent),
      m_color_name(color_name(current_color)) {
  setStyleSheet("background-color: white;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto hex_label = new QLabel(tr("HEX"), this);
  auto label_style = QString(R"(
      QLabel {
        background-color: #FFFFFF;
        font-family: Roboto;
        font-size: %1px;
      }
    )").arg(FONT_HEIGHT());
  hex_label->setStyleSheet(label_style);
  hex_label->setFixedSize(scale(29, 26));
  layout->addWidget(hex_label);
  auto hash_label = new QLabel("#", this);
  hash_label->setFixedSize(scale(14, 26));
  hash_label->setStyleSheet(label_style);
  layout->addWidget(hash_label);
  m_line_edit = new QLineEdit(m_color_name, this);
  m_line_edit->setValidator(new QRegExpValidator(QRegExp("^[0-9a-fA-F]{1,6}$"),
    this));
  m_line_edit->setFixedHeight(26);
  connect(m_line_edit, &QLineEdit::textChanged, this,
    &ColorSelectorHexInputWidget::on_text_changed);
  m_line_edit->installEventFilter(this);
  apply_line_edit_style(m_line_edit);
  layout->addWidget(m_line_edit);
}

void ColorSelectorHexInputWidget::set_color(const QColor& color) {
  if(color_name(color) != m_color_name) {
    m_color_name = color_name(color);
    m_line_edit->setText(m_color_name);
  }
}

connection ColorSelectorHexInputWidget::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

bool ColorSelectorHexInputWidget::eventFilter(QObject* watched,
    QEvent* event) {
  if(watched == m_line_edit && event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
      auto text = m_line_edit->text();
      if(text.length() == 6 || text.length() == 3) {
        auto color = QColor();
        color.setNamedColor(QString("#%1").arg(m_line_edit->text()));
        m_color_name = color_name(color);
        m_color_signal(color);
        if(text.length() == 3) {
          m_line_edit->setText(m_color_name);
        }
        m_line_edit->clearFocus();
      } else {
        m_line_edit->setText(m_color_name);
      }
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorHexInputWidget::on_text_changed(const QString& text) {
  m_line_edit->setText(text.toUpper());
}
