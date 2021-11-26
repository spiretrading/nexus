#include "Spire/Ui/TextBox.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void apply_label_style(TextBox& text_box) {
    update_style(text_box, [&] (auto& style) {
      style.get(Any()).
        set(border_size(0)).
        set(vertical_padding(0));
      style.get(ReadOnly() && Disabled()).
        set(TextColor(QColor(Qt::black)));
    });
    text_box.setDisabled(true);
    text_box.set_read_only(true);
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(text_style(font, QColor(Qt::black))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
    style.get(Hover() || Focus()).
      set(border_color(QColor(0x4B23A0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent))).
      set(horizontal_padding(0));
    style.get(Disabled()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border_color(QColor(0xC8C8C8))).
      set(TextColor(QColor(0xC8C8C8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
    style.get(Rejected()).
      set(BackgroundColor(chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
        linear(QColor(0xFFF1F1), revert, milliseconds(300))))).
      set(border_color(
        chain(timeout(QColor(0xB71C1C), milliseconds(550)), revert)));
    style.get(Placeholder()).set(TextColor(QColor(0xA0A0A0)));
    style.get(Disabled() / Placeholder()).
      set(TextColor(QColor(0xC8C8C8)));
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
    auto state = m_model->test(input);
    if(state == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    return QValidator::State::Acceptable;
  }
};

class TextBox::PlaceholderBox : public Box {
  public:
    explicit PlaceholderBox(QWidget* body)
      : Box(body),
        m_is_placeholder_visible(false) {}

    const QString& get_placeholder() const {
      return m_placeholder;
    }

    void set_placeholder(const QString& placeholder) {
      m_placeholder = placeholder;
      update_elision();
      update();
    }

    void set_placeholder_visible(bool is_visible) {
      if(is_visible != m_is_placeholder_visible) {
        m_is_placeholder_visible = is_visible;
        update();
      }
    }

    void update_style() {
      m_margins = {};
      m_alignment = Qt::AlignLeft;
      m_font = {};
      m_text_color = {};
      auto& stylist = find_stylist(*parentWidget());
      auto block = stylist.get_computed_block();
      for(auto& property : block) {
        property.visit(
          [&] (const BorderLeftSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setLeft(m_margins.left() + size);
            });
          },
          [&] (const BorderTopSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setTop(m_margins.top() + size);
            });
          },
          [&] (const BorderRightSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setRight(m_margins.right() + size);
            });
          },
          [&] (const BorderBottomSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setBottom(m_margins.bottom() + size);
            });
          },
          [&] (const PaddingLeft& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setLeft(m_margins.left() + size);
            });
          },
          [&] (const PaddingTop& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setTop(m_margins.top() + size);
            });
          },
          [&] (const PaddingRight& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setRight(m_margins.right() + size);
            });
          },
          [&] (const PaddingBottom& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_margins.setBottom(m_margins.bottom() + size);
            });
          });
      }
      auto& placeholder_stylist = *find_stylist(*parentWidget(), Placeholder());
      merge(block, placeholder_stylist.get_computed_block());
      for(auto& property : block) {
        property.visit(
          [&] (const TextColor& color) {
            placeholder_stylist.evaluate(color, [=] (auto color) {
              m_text_color = color;
            });
          },
          [&] (const TextAlign& alignment) {
            placeholder_stylist.evaluate(alignment, [=] (auto alignment) {
              m_alignment = alignment;
            });
          },
          [&] (const Font& font) {
            placeholder_stylist.evaluate(font, [=] (auto font) {
              m_font = font;
            });
          },
          [&] (const FontSize& size) {
            placeholder_stylist.evaluate(size, [=] (auto size) {
              m_font.setPixelSize(size);
            });
          });
      }
      update_elision();
      update();
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
      Box::paintEvent(event);
      if(m_is_placeholder_visible) {
        auto painter = QPainter(this);
        painter.setFont(m_font);
        painter.setPen(m_text_color);
        painter.drawText(rect() - m_margins, m_alignment, m_elided_placeholder);
      }
    }

    void resizeEvent(QResizeEvent* event) override {
      update_elision();
      Box::resizeEvent(event);
    }

  private:
    QString m_placeholder;
    QString m_elided_placeholder;
    bool m_is_placeholder_visible;
    QMargins m_margins;
    Qt::Alignment m_alignment;
    QFont m_font;
    QColor m_text_color;

    void update_elision() {
      m_elided_placeholder = QFontMetrics(m_font).elidedText(m_placeholder,
        Qt::ElideRight, width() - m_margins.left() - m_margins.right());
    }
};

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(color));
}

TextBox::StyleProperties::StyleProperties(std::function<void ()> commit)
  : m_styles(std::move(commit)) {}

void TextBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
  m_text_color = QColor();
  m_echo_mode = none;
}

TextBox::TextBox(QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(), parent) {}

TextBox::TextBox(QString current, QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

TextBox::TextBox(std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_line_edit_styles([=] { commit_style(); }),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_rejected(false),
      m_has_update(false) {
  m_line_edit = new QLineEdit(m_current->get());
  m_line_edit->setFrame(false);
  m_line_edit->setTextMargins(-2, 0, -4, 0);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_validator = new TextValidator(m_current, this);
  m_line_edit->setValidator(m_text_validator);
  m_line_edit->installEventFilter(this);
  setCursor(m_line_edit->cursor());
  setFocusPolicy(m_line_edit->focusPolicy());
  m_box = new PlaceholderBox(m_line_edit);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_box);
  setFocusProxy(m_box);
  proxy_style(*this, *m_box);
  add_pseudo_element(*this, Placeholder());
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_placeholder_style_connection =
    connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
  m_current_connection = m_current->connect_update_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<TextModel>& TextBox::get_current() const {
  return m_current;
}

const QString& TextBox::get_submission() const {
  return m_submission;
}

void TextBox::set_placeholder(const QString& placeholder) {
  m_box->set_placeholder(placeholder);
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
  setCursor(m_line_edit->cursor());
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  update_display_text();
  update_placeholder_text();
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
    m_line_edit->fontMetrics().horizontalAdvance(m_current->get()) +
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
      if(m_line_edit->text() != m_current->get()) {
        m_line_edit->setText(m_current->get());
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
    } else if(key_event.key() == Qt::Key_Enter ||
        key_event.key() == Qt::Key_Return) {
      m_has_update = true;
    }
  } else if(event->type() == QEvent::Resize) {
    update_display_text();
  }
  return QWidget::eventFilter(watched, event);
}

void TextBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_display_text();
  }
  QWidget::changeEvent(event);
}

void TextBox::mousePressEvent(QMouseEvent* event) {
  QWidget::mousePressEvent(event);
  event->accept();
  event->setLocalPos(m_line_edit->mapFromGlobal(event->globalPos()));
  QCoreApplication::sendEvent(m_line_edit, event);
}

void TextBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_submission != m_current->get()) {
      m_current->set(m_submission);
      return;
    }
  }
  QWidget::keyPressEvent(event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  update_display_text();
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

bool TextBox::is_placeholder_visible() const {
  return !is_read_only() && m_current->get().isEmpty() &&
    !m_box->get_placeholder().isEmpty();
}

void TextBox::elide_text() {
  auto font_metrics = m_line_edit->fontMetrics();
  auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
  auto elided_text = font_metrics.elidedText(
    m_current->get(), Qt::ElideRight, rect.width());
  m_text_validator->m_is_text_elided = elided_text != m_current->get();
  if(elided_text != m_line_edit->text()) {
    m_line_edit->setText(elided_text);
    m_line_edit->setCursorPosition(0);
  }
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_line_edit->text() != m_current->get()) {
    m_line_edit->setText(m_current->get());
  }
  m_size_hint = none;
  updateGeometry();
}

void TextBox::update_placeholder_text() {
  m_box->set_placeholder_visible(is_placeholder_visible());
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
  }
  update_display_text();
}

void TextBox::on_current(const QString& current) {
  m_has_update = true;
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
  update_display_text();
  update_placeholder_text();
}

void TextBox::on_editing_finished() {
  if(!is_read_only() && m_has_update) {
    if(m_current->get_state() == QValidator::Acceptable) {
      m_submission = m_current->get();
      m_has_update = false;
      m_submit_signal(m_submission);
    } else {
      m_reject_signal(m_current->get());
      m_current->set(m_submission);
      if(!m_is_rejected) {
        m_is_rejected = true;
        match(*this, Rejected());
      }
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_current->set(text);
}

void TextBox::on_style() {
  auto& stylist = find_stylist(*this);
  m_line_edit_styles.clear();
  m_line_edit_styles.m_styles.buffer([&] {
    for(auto& property : stylist.get_computed_block()) {
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
  m_box->update_style();
}

TextBox* Spire::make_label(QString label, QWidget* parent) {
  auto text_box = new TextBox(std::move(label), parent);
  apply_label_style(*text_box);
  return text_box;
}

TextBox* Spire::make_label(std::shared_ptr<TextModel> model, QWidget* parent) {
  auto text_box = new TextBox(std::move(model), parent);
  apply_label_style(*text_box);
  return text_box;
}
