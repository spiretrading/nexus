#include "Spire/Ui/TextBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(
        text_style(QFont("Roboto", scale_width(12)), QColor::fromRgb(0, 0, 0))).
      set(horizontal_padding(scale_width(8)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(0));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0));
    return style;
  }
}

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(std::move(color)));
}

TextBox::TextBox(QWidget* parent)
  : TextBox({}, parent) {}

TextBox::TextBox(const QString& current, QWidget* parent)
    : StyledWidget(parent),
      m_current(current) {
  m_line_edit = new QLineEdit(m_current, this);
  m_line_edit->setFrame(false);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_line_edit);
  setLayout(layout);
  m_line_edit->installEventFilter(this);
  set_style(DEFAULT_STYLE());
  setFocusProxy(m_line_edit);
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::selectionChanged, this,
    &TextBox::on_selection_changed);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
}

const QString& TextBox::get_current() const {
  return m_current;
}

void TextBox::set_current(const QString& value) {
  m_current = value;
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else {
    m_line_edit->setText(m_current);
  }
}

const QString& TextBox::get_submission() const {
  return m_submission;
}

void TextBox::set_read_only(bool read_only) {
  m_line_edit->setReadOnly(read_only);
}

bool TextBox::is_read_only() const {
  return m_line_edit->isReadOnly();
}

connection TextBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection TextBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void TextBox::style_updated() {
  elide_text();
}

bool TextBox::test_selector(const Styles::Selector& selector) const {
  return selector.visit(
    [&] (ReadOnly) {
      return is_read_only();
    },
    [&] {
      return StyledWidget::test_selector(selector);
    });
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_line_edit) {
    switch(event->type()) {
      case QEvent::ReadOnlyChange:
      case QEvent::EnabledChange:
        elide_text();
        break;
      case QEvent::FocusIn:
        if(!is_read_only()) {
          auto reason = static_cast<QFocusEvent*>(event)->reason();
          if(reason != Qt::ActiveWindowFocusReason &&
              reason != Qt::PopupFocusReason) {
            m_line_edit->setText(m_current);
          }
        }
        break;
      case QEvent::FocusOut:
        {
          auto reason = static_cast<QFocusEvent*>(event)->reason();
          if(reason != Qt::ActiveWindowFocusReason &&
              reason != Qt::PopupFocusReason) {
            elide_text();
          }
        }
        break;
      case QEvent::KeyPress:
        if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
          m_current = m_submission;
          m_line_edit->setText(m_current);
          m_current_signal(m_current);
        }
        break;
      }
  }
  return QWidget::eventFilter(watched, event);
}

void TextBox::paintEvent(QPaintEvent* event) {
  auto computed_style = compute_style();
  auto style = QString("QLineEdit {");
  style += "border-style: solid;";
  for(auto& property : computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        style += "background-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderTopSize& size) {
        style += "border-top-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const BorderRightSize& size) {
        style += "border-right-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const BorderBottomSize& size) {
        style += "border-bottom-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const BorderLeftSize& size) {
        style += "border-left-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const BorderTopColor& color) {
        style += "border-top-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderRightColor& color) {
        style += "border-right-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderBottomColor& color) {
        style += "border-bottom-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderLeftColor& color) {
        style += "border-left-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const TextColor& color) {
        style += "color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const Font& font) {
        setFont(font.get_expression().as<QFont>());
      },
      [&] (const PaddingTop& size) {
        style += "padding-top: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const PaddingRight& size) {
        style += "padding-right: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const PaddingBottom& size) {
        style += "padding-bottom: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      },
      [&] (const PaddingLeft& size) {
        style += "padding-left: " + QString::number(
          size.get_expression().as<int>()) + "px;";
      });
  }
  style += "}";
  m_line_edit->setStyleSheet(style);
  StyledWidget::paintEvent(event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  elide_text();
  QWidget::resizeEvent(event);
}

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

void TextBox::on_editing_finished() {
  if(!is_read_only()) {
    m_submission = m_current;
    m_submit_signal(m_submission);
  }
}

void TextBox::on_selection_changed() {
  if(is_read_only()) {
    auto text = m_line_edit->selectedText();
    if(text.endsWith(QChar(0x2026)) || text.endsWith(QStringLiteral("..."))) {
      m_line_edit->setText(m_current);
      m_line_edit->setSelection(0, m_current.length());
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_current = text;
  m_current_signal(m_current);
}

void TextBox::elide_text() {
  auto font_metrics = m_line_edit->fontMetrics();
  auto option = QStyleOptionFrame();
  option.initFrom(m_line_edit);
  option.rect = m_line_edit->contentsRect();
  option.lineWidth = 0;
  option.midLineWidth = 0;
  option.state |= QStyle::State_Sunken;
  if(is_read_only()) {
    option.state |= QStyle::State_ReadOnly;
  }
  option.features = QStyleOptionFrame::None;
  auto rect = m_line_edit->style()->subElementRect(QStyle::SE_LineEditContents,
    &option, m_line_edit);
  m_line_edit->setText(font_metrics.elidedText(m_current, Qt::ElideRight,
    rect.width()));
  m_line_edit->setCursorPosition(0);
}
