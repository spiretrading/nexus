#include "Spire/Ui/TextBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

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
    style.get(Disabled() > Placeholder()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

struct TextBox::TextValidator : QValidator {
  std::shared_ptr<TextModel> m_model;
  bool m_is_text_elided;

  TextValidator(std::shared_ptr<TextModel> model, QObject* parent = nullptr)
    : QValidator(parent),
      m_model(std::move(model)),
      m_is_text_elided(false) {}

  QValidator::State validate(QString& input, int& pos) const override {
    if(m_is_text_elided) {
      return QValidator::State::Acceptable;
    }
    if(input == m_model->get_current()) {
      auto state = m_model->get_state();
      if(state == QValidator::State::Invalid) {
        return state;
      }
      return QValidator::State::Acceptable;
    }
    auto current = std::move(input);
    auto state = m_model->set_current(current);
    input = m_model->get_current();
    if(state == QValidator::State::Invalid) {
      return state;
    }
    return QValidator::State::Acceptable;
  }
};

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(std::move(color)));
}

TextBox::TextBox(QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(), parent) {}

TextBox::TextBox(QString current, QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

TextBox::TextBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent),
      m_is_warning_displayed(true),
      m_model(std::move(model)),
      m_submission(m_model->get_current()) {
  std::shared_ptr<TextModel> m = std::make_shared<LocalTextModel>();
  m_layers = new LayeredWidget(this);
  m_line_edit = new QLineEdit(m_model->get_current());
  m_line_edit->setFrame(false);
  m_line_edit->setTextMargins(-2, 0, 0, 0);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_validator = new TextValidator(m_model, this);
  m_line_edit->setValidator(m_text_validator);
  m_line_edit->installEventFilter(this);
  m_layers->add(m_line_edit);
  m_placeholder = new QLabel();
  m_placeholder->setCursor(m_line_edit->cursor());
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setMargin(0);
  m_placeholder->setIndent(0);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_layers->add(m_placeholder);
  m_box = new Box(m_layers);
  m_box->setFocusProxy(m_line_edit);
  setCursor(m_line_edit->cursor());
  setFocusPolicy(m_line_edit->focusPolicy());
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_box);
  setFocusProxy(m_box);
  proxy_style(*this, *m_box);
  add_pseudo_element(*this, Placeholder());
  connect_style_signal(*this, [=] { on_style(); });
  connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<TextModel>& TextBox::get_model() const {
  return m_model;
}

const QString& TextBox::get_submission() const {
  return m_submission;
}

void TextBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
  update_placeholder_text();
}

bool TextBox::is_read_only() const {
  return m_line_edit->isReadOnly();
}

void TextBox::set_read_only(bool read_only) {
  m_line_edit->setReadOnly(read_only);
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  update_display_text();
  update_placeholder_text();
}

bool TextBox::is_warning_displayed() const {
  return m_is_warning_displayed;
}

void TextBox::set_warning_displayed(bool is_displayed) {
  m_is_warning_displayed = is_displayed;
}

connection TextBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection TextBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    auto focus_event = static_cast<QFocusEvent*>(event);
    if(focus_event->reason() != Qt::ActiveWindowFocusReason &&
        focus_event->reason() != Qt::PopupFocusReason) {
      m_text_validator->m_is_text_elided = false;
      if(m_line_edit->text() != m_model->get_current()) {
        m_line_edit->setText(m_model->get_current());
      }
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->lostFocus() &&
        focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      update_display_text();
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Up ||
        key_event.key() == Qt::Key_Down) {
      key_event.ignore();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TextBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_display_text();
    update_placeholder_text();
  }
  QWidget::changeEvent(event);
}

void TextBox::mousePressEvent(QMouseEvent* event) {
  if(is_placeholder_shown()) {
    m_line_edit->setFocus();
  }
  QWidget::mousePressEvent(event);
}

void TextBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_submission != m_model->get_current()) {
      m_model->set_current(m_submission);
    }
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TextBox::resizeEvent(QResizeEvent* event) {
  update_display_text();
  update_placeholder_text();
  QWidget::resizeEvent(event);
}

bool TextBox::is_placeholder_shown() const {
  return !is_read_only() && m_model->get_current().isEmpty() &&
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
  auto elided_text = font_metrics.elidedText(m_model->get_current(),
    Qt::ElideRight, rect.width());
  if(elided_text != m_model->get_current()) {
    m_text_validator->m_is_text_elided = true;
  }
  if(elided_text != m_line_edit->text()) {
    m_line_edit->setText(elided_text);
    m_line_edit->setCursorPosition(0);
  }
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_line_edit->text() != m_model->get_current()) {
    m_line_edit->setText(m_model->get_current());
  }
}

void TextBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    auto font_metrics = m_placeholder->fontMetrics();
    auto rect = m_placeholder->contentsRect();
    auto m = font_metrics.horizontalAdvance(QLatin1Char('x')) / 2 -
      m_placeholder->margin();
    auto text_direction = [&] {
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

void TextBox::on_current(const QString& current) {
  update_display_text();
  update_placeholder_text();
}

void TextBox::on_editing_finished() {
  if(!is_read_only()) {
    if(m_model->get_state() == QValidator::Acceptable) {
      m_submission = m_model->get_current();
      m_submit_signal(m_submission);
    } else {
      m_reject_signal(m_model->get_current());
      m_model->set_current(m_submission);
      if(is_warning_displayed()) {
        display_warning_indicator(*this);
      }
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  update_placeholder_text();
}

void TextBox::on_style() {
  auto computed_style = compute_style(*this);
  auto line_edit_style = QString(
    R"(QLineEdit {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  auto is_line_edit_updated = false;
  for(auto& property : computed_style.get_properties()) {
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
      [&] (const EchoMode& echo_mode) {
        auto computed_mode =
          echo_mode.get_expression().as<QLineEdit::EchoMode>();
        if(m_line_edit->echoMode() != computed_mode) {
          m_line_edit->setEchoMode(computed_mode);
          is_line_edit_updated = true;
        }
      });
  }
  merge(computed_style, compute_style(*this, Placeholder()));
  auto placeholder_style = QString(
    R"(QLabel {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  auto is_placeholder_updated = false;
  for(auto& property : computed_style.get_properties()) {
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
      });
  }
  line_edit_style += "}";
  if(line_edit_style != m_line_edit->styleSheet()) {
    m_line_edit->setStyleSheet(line_edit_style);
    m_line_edit->style()->unpolish(this);
    m_line_edit->style()->polish(this);
    is_line_edit_updated = true;
  }
  if(is_line_edit_updated) {
    update_display_text();
  }
  placeholder_style += "}";
  if(placeholder_style != m_placeholder->styleSheet()) {
    m_placeholder->setStyleSheet(placeholder_style);
    m_placeholder->style()->unpolish(this);
    m_placeholder->style()->polish(this);
    is_placeholder_updated = true;
  }
  if(is_placeholder_updated) {
    update_placeholder_text();
  }
}
