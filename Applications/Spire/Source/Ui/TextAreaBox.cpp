#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTextBlock>
#include <QTextDocument>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
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
      set(LineHeight(1.25)).
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(7)));
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
    style.get(Rejected()).
      set(BackgroundColor(chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
        linear(QColor(0xFFF1F1), revert, milliseconds(300))))).
      set(border_color(
        chain(timeout(QColor(0xB71C1C), milliseconds(550)), revert)));
    style.get(Placeholder()).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignTop)).
      set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() / Placeholder()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

struct TextAreaBox::TextValidator : QValidator {
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

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
  //m_echo_mode = none;
}

TextAreaBox::TextAreaBox(QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(), parent) {}

TextAreaBox::TextAreaBox(QString current, QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(std::move(current)),
      parent) {}

TextAreaBox::TextAreaBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent),
      m_text_edit_styles{[=] { commit_style(); }},
      m_placeholder_styles{[=] { commit_placeholder_style(); }},
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_rejected(false) {
  auto layers = new LayeredWidget(this);
  layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_edit = new QTextEdit(m_model->get_current());
  // TODO
  //m_text_edit->setFrame(false);
  m_text_edit->setAcceptRichText(false);
  m_text_edit->document()->setDocumentMargin(0);
  m_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setFrameShape(QFrame::NoFrame);
  // TODO
  //m_text_edit->setTextMargins(-2, 0, 0, 0);
  m_text_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_validator = new TextValidator(m_model, this);
  // TODO
  //m_text_edit->setValidator(m_text_validator);
  m_text_edit->installEventFilter(this);
  layers->add(m_text_edit);
  m_placeholder = new QLabel();
  m_placeholder->setCursor(m_text_edit->cursor());
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setWordWrap(true);
  m_placeholder->setMargin(0);
  m_placeholder->setIndent(0);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //m_placeholder->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  layers->add(m_placeholder);
  m_box = new Box(layers);
  m_box->setFocusProxy(m_text_edit);
  setCursor(m_text_edit->cursor());
  setFocusPolicy(m_text_edit->focusPolicy());
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_box);
  setFocusProxy(m_box);
  proxy_style(*this, *m_box);
  add_pseudo_element(*this, Placeholder());
  connect_style_signal(*this, [=] { on_style(); });
  connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  //connect(m_line_edit, &QLineEdit::editingFinished, this,
  //  &TextAreaBox::on_editing_finished);
  //connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
  connect(m_text_edit, &QTextEdit::textChanged, this, &TextAreaBox::on_text_edited);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<TextModel>& TextAreaBox::get_model() const {
  return m_model;
}

const QString& TextAreaBox::get_submission() const {
  return m_submission;
}

void TextAreaBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
  update_placeholder_text();
}

bool TextAreaBox::is_read_only() const {
  return m_text_edit->isReadOnly();
}

void TextAreaBox::set_read_only(bool read_only) {
  m_text_edit->setReadOnly(read_only);
  // TODO
  //m_text_edit->setCursorPosition(0);
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  update_display_text();
  update_placeholder_text();
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto cursor_width = [&] {
    if(is_read_only()) {
      return 0;
    }
    return 1;
  }();
  m_size_hint.emplace(
    m_text_edit->fontMetrics().horizontalAdvance(m_model->get_current()) +
      cursor_width, m_text_edit->fontMetrics().height());
  *m_size_hint += compute_decoration_size();
  return *m_size_hint;
}

bool TextAreaBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    auto focus_event = static_cast<QFocusEvent*>(event);
    if(focus_event->reason() != Qt::ActiveWindowFocusReason &&
        focus_event->reason() != Qt::PopupFocusReason) {
      m_text_validator->m_is_text_elided = false;
      if(m_text_edit->toPlainText() != m_model->get_current()) {
        m_text_edit->setText(m_model->get_current());
      }
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->lostFocus() &&
        focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      update_display_text();
    }
  }/* else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Up || key_event.key() == Qt::Key_Down) {
      key_event.ignore();
      return true;
    }
  } */else if(event->type() == QEvent::Resize) {
    update_display_text();
    update_placeholder_text();
  }
  return QWidget::eventFilter(watched, event);
}

void TextAreaBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_display_text();
    update_placeholder_text();
  }
  QWidget::changeEvent(event);
}

void TextAreaBox::mousePressEvent(QMouseEvent* event) {
  if(is_placeholder_shown()) {
    m_text_edit->setFocus();
  }
  QWidget::mousePressEvent(event);
}

void TextAreaBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_submission != m_model->get_current()) {
      m_model->set_current(m_submission);
    }
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TextAreaBox::resizeEvent(QResizeEvent* event) {
  update_display_text();
  update_placeholder_text();
  QWidget::resizeEvent(event);
}

QSize TextAreaBox::compute_decoration_size() const {
  auto decoration_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*m_box)) {
    property.visit(
      [&] (std::in_place_type_t<BorderTopSize>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<BorderBottomSize>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<PaddingTop>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingRight>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<PaddingBottom>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingLeft>, int size) {
        decoration_size.rwidth() += size;
      });
  }
  return decoration_size;
}

bool TextAreaBox::is_placeholder_shown() const {
  return !is_read_only() && m_model->get_current().isEmpty() &&
    !m_placeholder_text.isEmpty();
}

void TextAreaBox::elide_text() {
  // TODO
  //auto font_metrics = m_text_edit->fontMetrics();
  //auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
  //auto elided_text = font_metrics.elidedText(
  //  m_model->get_current(), Qt::ElideRight, rect.width());
  //m_text_validator->m_is_text_elided = elided_text != m_model->get_current();
  //if(elided_text != m_text_edit->toPlainText()) {
  //  // TODO
  //  m_text_edit->setText(m_model->get_current());//elided_text);
  //  // TODO
  //  //m_text_edit->setCursorPosition(0);
  //}
}

void TextAreaBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_text_edit->toPlainText() != m_model->get_current()) {
    m_text_edit->setText(m_model->get_current());
  }
  m_size_hint = none;
  updateGeometry();
}

void TextAreaBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    //auto font_metrics = m_placeholder->fontMetrics();
    //auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
    //auto elided_text =
    //  font_metrics.elidedText(m_placeholder_text, Qt::ElideRight, rect.width());
    //qDebug() << elided_text;
    //m_placeholder->setText(elided_text);
    m_placeholder->setText(m_placeholder_text);
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}

void TextAreaBox::commit_style() {
  auto stylesheet = QString(
    R"(QTextEdit {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  m_text_edit_styles.m_styles.write(stylesheet);
  auto alignment = m_text_edit_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  if(alignment != m_text_edit->alignment()) {
    // TODO: deal with selections
    auto cursor_pos = m_text_edit->textCursor().position();
    for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
      auto block = m_text_edit->document()->findBlockByNumber(i);
      if(block.isValid()) {
        auto cursor = m_text_edit->textCursor();
        cursor.setPosition(block.position());
        m_text_edit->setTextCursor(cursor);
        m_text_edit->setAlignment(alignment);
      }
    }
    auto cursor = m_text_edit->textCursor();
    cursor.setPosition(cursor_pos);
    m_text_edit->setTextCursor(cursor);
  }
  auto font = m_text_edit_styles.m_font.value_or(QFont());
  if(m_text_edit_styles.m_size) {
    font.setPixelSize(*m_text_edit_styles.m_size);
  }
  m_text_edit->setFont(font);
  if(m_text_edit_styles.m_line_height) {
    auto cursor_pos = m_text_edit->textCursor().position();
    for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
      auto block = m_text_edit->document()->findBlockByNumber(i);
      if(block.isValid()) {
        auto cursor = m_text_edit->textCursor();
        cursor.setPosition(block.position());
        m_text_edit->setTextCursor(cursor);
        auto block_format = cursor.blockFormat();
        auto line_height = static_cast<double>(font.pixelSize()) *
          *m_text_edit_styles.m_line_height;
        block_format.setLineHeight(line_height, QTextBlockFormat::FixedHeight);
        cursor.setBlockFormat(block_format);
        m_text_edit->setTextCursor(cursor);
      }
    }
    auto cursor = m_text_edit->textCursor();
    cursor.setPosition(cursor_pos);
    m_text_edit->setTextCursor(cursor);
  }
  if(stylesheet != m_text_edit->styleSheet()) {
    m_text_edit->setStyleSheet(stylesheet);
    m_text_edit->style()->unpolish(this);
    m_text_edit->style()->polish(this);
  }
  update_display_text();
}

void TextAreaBox::commit_placeholder_style() {
  auto stylesheet = QString(
    R"(QLabel {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  m_placeholder_styles.m_styles.write(stylesheet);
  auto alignment = m_placeholder_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  if(alignment != m_placeholder->alignment()) {
    m_placeholder->setAlignment(alignment);
  }
  auto font = m_placeholder_styles.m_font.value_or(QFont());
  if(m_placeholder_styles.m_size) {
    font.setPixelSize(*m_placeholder_styles.m_size);
  }
  m_placeholder->setFont(font);
  if(stylesheet != m_placeholder->styleSheet()) {
    m_placeholder->setStyleSheet(stylesheet);
    m_placeholder->style()->unpolish(this);
    m_placeholder->style()->polish(this);
  }
  update_placeholder_text();
}

void TextAreaBox::on_current(const QString& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
  update_display_text();
  update_placeholder_text();
}

void TextAreaBox::on_editing_finished() {
  if(!is_read_only()) {
    if(m_model->get_state() == QValidator::Acceptable) {
      m_submission = m_model->get_current();
      m_submit_signal(m_submission);
    } else {
      //m_reject_signal(m_model->get_current());
      m_model->set_current(m_submission);
      if(!m_is_rejected) {
        m_is_rejected = true;
        match(*this, Rejected());
      }
    }
  }
}

void TextAreaBox::on_text_edited() {
  update_placeholder_text();
  // TODO: validate
  m_model->set_current(m_text_edit->toPlainText());
}

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  m_text_edit_styles.clear();
  m_text_edit_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const TextColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_text_edit_styles.m_styles.set("color", color);
          });
        },
        [&] (const TextAlign& alignment) {
          stylist.evaluate(alignment, [=] (auto alignment) {
            m_text_edit_styles.m_alignment = alignment;
          });
        },
        [&] (const Font& font) {
          stylist.evaluate(font, [=] (const auto& font) {
            m_text_edit_styles.m_font = font;
          });
        },
        [&] (const FontSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_size = size;
          });
        },
        [&] (const LineHeight& height) {
          stylist.evaluate(height, [=] (auto height) {
            m_text_edit_styles.m_line_height = height;
          });
        });
    }
  });
  auto& placeholder_stylist = *find_stylist(*this, Placeholder());
  merge(block, placeholder_stylist.get_computed_block());
  m_placeholder_styles.clear();
  m_placeholder_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const TextColor& color) {
          placeholder_stylist.evaluate(color, [=] (auto color) {
            m_placeholder_styles.m_styles.set("color", color);
          });
        },
        [&] (const TextAlign& alignment) {
          placeholder_stylist.evaluate(alignment, [=] (auto alignment) {
            m_placeholder_styles.m_alignment = alignment;
          });
        },
        [&] (const Font& font) {
          placeholder_stylist.evaluate(font, [=] (auto font) {
            m_placeholder_styles.m_font = font;
          });
        },
        [&] (const FontSize& size) {
          placeholder_stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_size = size;
          });
        });
    }
  });
}
