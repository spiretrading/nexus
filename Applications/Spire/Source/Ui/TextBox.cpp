#include "Spire/Ui/TextBox.hpp"
#include <array>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(horizontal_padding(scale_width(8)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0))).
      set(horizontal_padding(0));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0)));
    style.get(Placeholder()).set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Placeholder() && Disabled()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  const auto WARNING_FRAME_COUNT = 10;

  auto get_fade_out_step(int start, int end) {
    return (end - start) / WARNING_FRAME_COUNT;
  }

  auto get_color_step(const QColor& start_color, const QColor& end_color) {
    return std::array{get_fade_out_step(start_color.red(), end_color.red()),
      get_fade_out_step(start_color.green(), end_color.green()),
      get_fade_out_step(start_color.blue(), end_color.blue())};
  }

  auto get_fade_out_color(const QColor& color, const std::array<int, 3>& steps,
      int frame) {
    return QColor(color.red() + steps[0] * frame,
      color.green() + steps[1] * frame, color.blue() + steps[2] * frame);
  }
}

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(std::move(color)));
}

TextBox::TextBox(QWidget* parent)
  : TextBox({}, parent) {}

TextBox::TextBox(const QString& current, QWidget* parent)
    : StyledWidget(parent),
      m_current(current),
      m_submission(m_current) {
  m_content_layer = new QWidget(this);
  m_line_edit = new QLineEdit(m_current, m_content_layer);
  m_line_edit->setFrame(false);
  m_line_edit->setTextMargins(-1, 0, 0, 0);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit->installEventFilter(this);
  m_placeholder = new QLabel(m_content_layer);
  m_placeholder->setCursor(m_line_edit->cursor());
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setMargin(0);
  m_placeholder->setIndent(0);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit->stackUnder(m_placeholder);
  auto content_layout = new QHBoxLayout(m_content_layer);
  content_layout->setContentsMargins(0, 0, 0, 0);
  content_layout->addWidget(m_line_edit);
  m_box = new Box(*m_content_layer, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_box);
  set_style(DEFAULT_STYLE());
  setFocusProxy(m_line_edit);
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
}

const QString& TextBox::get_current() const {
  return m_current;
}

void TextBox::set_current(const QString& value) {
  m_current = value;
  update_display_text();
  update_placeholder_text();
  m_current_signal(m_current);
}

const QString& TextBox::get_submission() const {
  return m_submission;
}

void TextBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
  update_placeholder_text();
}

void TextBox::set_read_only(bool read_only) {
  m_line_edit->setReadOnly(read_only);
  update_display_text();
  update_placeholder_text();
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

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

void TextBox::style_updated() {
  update_display_text();
  update_placeholder_text();
  StyledWidget::style_updated();
}

bool TextBox::test_selector(const Styles::Selector& element,
    const Styles::Selector& selector) const {
  return selector.visit(
    [&] (ReadOnly) {
      return is_read_only();
    },
    [&] {
      return StyledWidget::test_selector(element, selector);
    });
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      if(m_line_edit->text() != m_current) {
        m_line_edit->setText(m_current);
      }
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->lostFocus() &&
        focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      update_display_text();
    }
  }
  return StyledWidget::eventFilter(watched, event);
}

void TextBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_display_text();
    update_placeholder_text();
  }
  StyledWidget::changeEvent(event);
}

void TextBox::mousePressEvent(QMouseEvent* event) {
  if(is_placeholder_shown()) {
    m_line_edit->setFocus();
  }
  StyledWidget::mousePressEvent(event);
}

void TextBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_submission != m_current) {
      set_current(m_submission);
    }
  } else {
    StyledWidget::keyPressEvent(event);
  }
}

void TextBox::paintEvent(QPaintEvent* event) {
  auto line_edit_computed_style = compute_style();
  auto placeholder_computed_style = compute_style(Placeholder());
  auto placeholder_style = QString(
    R"(QLabel {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  auto line_edit_style = QString(
    R"(QLineEdit {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  auto is_line_edit_updated = false;
  for(auto& property : line_edit_computed_style.get_properties()) {
    property.visit(
      [&] (const TextColor& color) {
        line_edit_style += "color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const TextAlign& alignment) {
        auto computed_alignment =
          alignment.get_expression().as<Qt::Alignment>();
        if(computed_alignment != m_line_edit->alignment()) {
          m_line_edit->setAlignment(computed_alignment);
          is_line_edit_updated = true;
        }
      },
      [&] (const Font& font) {
        auto computed_font = font.get_expression().as<QFont>();
        if(m_line_edit_font != computed_font) {
          m_line_edit_font = computed_font;
          m_line_edit->setFont(computed_font);
          is_line_edit_updated = true;
        }
      },
      [] {});
  }
  auto is_placeholder_updated = false;
  if(is_placeholder_shown()) {
    for(auto& property : placeholder_computed_style.get_properties()) {
      property.visit(
        [&] (const TextColor& color) {
          placeholder_style += "color: " +
            color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
        },
        [&] (const TextAlign& alignment) {
          auto computed_alignment =
            alignment.get_expression().as<Qt::Alignment>();
          if(computed_alignment != m_placeholder->alignment()) {
            m_placeholder->setAlignment(computed_alignment);
            is_placeholder_updated = true;
          }
        },
        [&] (const Font& font) {
          auto computed_font = font.get_expression().as<QFont>();
          if(m_placeholder_font != computed_font) {
            m_placeholder_font = computed_font;
            m_placeholder->setFont(computed_font);
            is_placeholder_updated = true;
          }
        },
        [] {});
    }
    if(!m_default_box_style) {
      auto style_sheet = StyleSheet();
      style_sheet.get(
        Any()).set_override(Rule::Override::EXCLUSIVE).get_block() =
        std::move(placeholder_computed_style);
      m_default_box_style = m_box->get_style();
      m_box->set_style(std::move(style_sheet));
    }
  } else if(m_default_box_style) {
    m_box->set_style(std::move(*m_default_box_style));
    m_default_box_style = none;
  }
  line_edit_style += "}";
  if(line_edit_style != m_line_edit->styleSheet()) {
    m_line_edit->setStyleSheet(line_edit_style);
    is_line_edit_updated = true;
  }
  if(is_line_edit_updated) {
    update_display_text();
  }
  placeholder_style += "}";
  if(placeholder_style != m_placeholder->styleSheet()) {
    m_placeholder->setStyleSheet(placeholder_style);
    is_placeholder_updated = true;
  }
  if(is_placeholder_updated) {
    update_placeholder_text();
  }
  StyledWidget::paintEvent(event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  m_placeholder->resize(m_content_layer->size());
  update_display_text();
  update_placeholder_text();
  StyledWidget::resizeEvent(event);
}

void TextBox::on_editing_finished() {
  if(!is_read_only()) {
    m_submission = m_current;
    m_submit_signal(m_submission);
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_current = text;
  update_placeholder_text();
  m_current_signal(m_current);
}

bool TextBox::is_placeholder_shown() const {
  return !is_read_only() && m_current.isEmpty() &&
    !m_placeholder_text.isEmpty();
}

QString TextBox::get_elided_text(const QFontMetrics& font_metrics,
    const QString& text) const {
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
  return font_metrics.elidedText(text, Qt::ElideRight, rect.width());
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
  auto elided_text = font_metrics.elidedText(m_current, Qt::ElideRight,
    rect.width());
  if(elided_text != m_line_edit->text()) {
    m_line_edit->setText(elided_text);
    m_line_edit->setCursorPosition(0);
  }
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_line_edit->text() != m_current) {
    m_line_edit->setText(m_current);
  }
}

void TextBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    auto font_metrics = m_placeholder->fontMetrics();
    auto rect = m_placeholder->contentsRect();
    auto m = font_metrics.horizontalAdvance(QLatin1Char('x')) / 2 -
      m_placeholder->margin();
    auto text_direction = [=] () -> Qt::LayoutDirection {
      if(m_placeholder_text.isRightToLeft()) {
        return Qt::RightToLeft;
      } else {
        return Qt::LeftToRight;
      }
    }();
    auto align = QStyle::visualAlignment(text_direction,
      QFlag(m_placeholder->alignment()));
    if(align & Qt::AlignLeft) {
      rect.setLeft(rect.left() + m);
    } else if(align & Qt::AlignRight) {
      rect.setRight(rect.right() - m);
    } else if(align & Qt::AlignTop) {
      rect.setTop(rect.top() + m);
    } else if(align & Qt::AlignBottom) {
      rect.setBottom(rect.bottom() - m);
    }
    auto placeholder_text = font_metrics.elidedText(m_placeholder_text,
      Qt::ElideRight, rect.width());
    m_placeholder->setText(placeholder_text);
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}

void Spire::display_warning_indicator(StyledWidget& widget) {
  const auto WARNING_DURATION = 300;
  const auto WARNING_FADE_OUT_DELAY = 250;
  const auto WARNING_BACKGROUND_COLOR = QColor("#FFF1F1");
  const auto WARNING_BORDER_COLOR = QColor("#B71C1C");
  auto time_line = new QTimeLine(WARNING_DURATION, &widget);
  time_line->setFrameRange(0, WARNING_FRAME_COUNT);
  time_line->setEasingCurve(QEasingCurve::Linear);
  auto computed_style = widget.compute_style();
  auto computed_background_color = [&] {
    if(auto color = find<BackgroundColor>(computed_style)) {
      return color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto background_color_step =
    get_color_step(WARNING_BACKGROUND_COLOR, computed_background_color);
  auto computed_border_color = [&] {
    if(auto border_color = find<BorderTopColor>(computed_style)) {
      return border_color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto border_color_step =
    get_color_step(WARNING_BORDER_COLOR, computed_border_color);
  QObject::connect(time_line, &QTimeLine::frameChanged,
    [=, &widget] (auto frame) {
      auto frame_background_color = get_fade_out_color(WARNING_BACKGROUND_COLOR,
        background_color_step, frame);
      auto frame_border_color =
        get_fade_out_color(WARNING_BORDER_COLOR, border_color_step, frame);
      auto animated_style = widget.get_style();
      animated_style.get(Any()).
        set(BackgroundColor(frame_background_color)).
        set(border_color(frame_border_color));
      widget.set_style(std::move(animated_style));
    });
  auto style = widget.get_style();
  QObject::connect(time_line, &QTimeLine::finished, [=, &widget] () mutable {
    widget.set_style(std::move(style));
    time_line->deleteLater();
  });
  auto animated_style = StyleSheet();
  animated_style.get(Any()) = style.get(Any());
  animated_style.get(Any()).set(BackgroundColor(WARNING_BACKGROUND_COLOR));
  animated_style.get(Any()).set(border_color(WARNING_BORDER_COLOR));
  widget.set_style(std::move(animated_style));
  QTimer::singleShot(WARNING_FADE_OUT_DELAY, &widget, [=] {
    time_line->start();
  });
}
