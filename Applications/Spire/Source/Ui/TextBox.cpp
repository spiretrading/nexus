#include "Spire/Ui/TextBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"

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
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
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
    style.get(Placeholder()).set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() / Placeholder()).
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
  return TextStyle(Font(std::move(font)), TextColor(color));
}

void TextBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
  m_echo_mode = none;
}

TextBox::TextBox(QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(), parent) {}

TextBox::TextBox(QString current, QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

TextBox::TextBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent),
      m_line_edit_styles{[=] { commit_style(); }},
      m_placeholder_styles{[=] { commit_placeholder_style(); }},
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_rejected(false) {
  auto layers = new LayeredWidget(this);
  layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit = new QLineEdit(m_model->get_current());
  m_line_edit->setFrame(false);
  m_line_edit->setTextMargins(-2, 0, 0, 0);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_validator = new TextValidator(m_model, this);
  m_line_edit->setValidator(m_text_validator);
  m_line_edit->installEventFilter(this);
  layers->add(m_line_edit);
  m_placeholder = new QLabel();
  m_placeholder->setCursor(m_line_edit->cursor());
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setMargin(0);
  m_placeholder->setIndent(0);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layers->add(m_placeholder);
  m_box = new Box(layers);
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
  if(m_line_edit->isReadOnly() == read_only) {
    return;
  }
  m_line_edit->setReadOnly(read_only);
  m_line_edit->setCursorPosition(0);
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  update_display_text();
  update_placeholder_text();
}

connection
    TextBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection
    TextBox::connect_reject_signal(const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

QSize TextBox::sizeHint() const {
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
    m_line_edit->fontMetrics().horizontalAdvance(m_model->get_current()) +
      cursor_width, m_line_edit->fontMetrics().height());
  *m_size_hint += compute_decoration_size();
  return *m_size_hint;
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
    if(key_event.key() == Qt::Key_Up || key_event.key() == Qt::Key_Down) {
      key_event.ignore();
      return true;
    }
  } else if(event->type() == QEvent::Resize) {
    update_display_text();
    update_placeholder_text();
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

QSize TextBox::compute_decoration_size() const {
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

bool TextBox::is_placeholder_shown() const {
  return !is_read_only() && m_model->get_current().isEmpty() &&
    !m_placeholder_text.isEmpty();
}

void TextBox::elide_text() {
  auto font_metrics = m_line_edit->fontMetrics();
  auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
  auto elided_text = font_metrics.elidedText(
    m_model->get_current(), Qt::ElideRight, rect.width());
  m_text_validator->m_is_text_elided = elided_text != m_model->get_current();
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
  m_size_hint = none;
  updateGeometry();
}

void TextBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    auto font_metrics = m_placeholder->fontMetrics();
    auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
    auto elided_text =
      font_metrics.elidedText(m_placeholder_text, Qt::ElideRight, rect.width());
    m_placeholder->setText(elided_text);
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}

void TextBox::commit_style() {
  auto stylesheet = QString(
    R"(QLineEdit {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  m_line_edit_styles.m_styles.write(stylesheet);
  auto alignment = m_line_edit_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  if(alignment != m_line_edit->alignment()) {
    m_line_edit->setAlignment(alignment);
  }
  auto font = m_line_edit_styles.m_font.value_or(QFont());
  if(m_line_edit_styles.m_size) {
    font.setPixelSize(*m_line_edit_styles.m_size);
  }
  m_line_edit->setFont(font);
  if(m_line_edit_styles.m_echo_mode) {
    m_line_edit->setEchoMode(*m_line_edit_styles.m_echo_mode);
  }
  if(stylesheet != m_line_edit->styleSheet()) {
    m_line_edit->setStyleSheet(stylesheet);
    m_line_edit->style()->unpolish(this);
    m_line_edit->style()->polish(this);
  }
  update_display_text();
}

void TextBox::commit_placeholder_style() {
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

void TextBox::on_current(const QString& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
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
      if(!m_is_rejected) {
        m_is_rejected = true;
        match(*this, Rejected());
      }
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  update_placeholder_text();
}

void TextBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  m_line_edit_styles.clear();
  m_line_edit_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const TextColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_line_edit_styles.m_styles.set("color", color);
          });
        },
        [&] (const TextAlign& alignment) {
          stylist.evaluate(alignment, [=] (auto alignment) {
            m_line_edit_styles.m_alignment = alignment;
          });
        },
        [&] (const Font& font) {
          stylist.evaluate(font, [=] (const auto& font) {
            m_line_edit_styles.m_font = font;
          });
        },
        [&] (const FontSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_line_edit_styles.m_size = size;
          });
        },
        [&] (const EchoMode& mode) {
          stylist.evaluate(mode, [=] (auto mode) {
            m_line_edit_styles.m_echo_mode = mode;
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

TextBox* Spire::make_label(QString label, QWidget* parent) {
  auto text_box = new TextBox(std::move(label), parent);
  text_box->setDisabled(true);
  text_box->set_read_only(true);
  auto style = get_style(*text_box);
  style.get(ReadOnly() && Disabled()).
    set(TextColor(QColor::fromRgb(0, 0, 0)));
  set_style(*text_box, std::move(style));
  return text_box;
}
