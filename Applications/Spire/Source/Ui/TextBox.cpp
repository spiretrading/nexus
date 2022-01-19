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

Highlight::Highlight()
  : Highlight(0) {}

Highlight::Highlight(int position)
  : Highlight(position, position) {}

Highlight::Highlight(int start, int end)
  : m_start(start),
    m_end(end) {}

bool Spire::is_collapsed(const Highlight& highlight) {
  return highlight.m_start == highlight.m_end;
}

int Spire::get_size(const Highlight& highlight) {
  return highlight.m_end - highlight.m_start;
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
    if(m_model->test(input) == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    return QValidator::State::Acceptable;
  }
};

class TextBox::EditableTextBox : public QLineEdit {
  public:

    EditableTextBox(std::shared_ptr<TextModel> current,
        std::shared_ptr<HighlightModel> highlight, TextBox* text_box)
        : QLineEdit(current->get(), text_box),
          m_text_box(text_box),
          m_current(std::move(current)),
          m_submission(m_current->get()),
          m_highlight(std::move(highlight)),
          m_text_validator(new TextValidator(m_current, this)),
          m_is_rejected(false),
          m_has_update(false),
          m_is_handling_key_press(false) {
      setObjectName(QString("0x%1").arg(reinterpret_cast<std::intptr_t>(this)));
      setFrame(false);
      setTextMargins(-2, 0, -4, 0);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      auto& current_highlight = m_highlight->get();
      setCursorPosition(current_highlight.m_end);
      setSelection(current_highlight.m_start, get_size(current_highlight));
      setValidator(m_text_validator);
      installEventFilter(this);
      connect(this, &QLineEdit::editingFinished, this,
        &EditableTextBox::on_editing_finished);
      connect(
        this, &QLineEdit::textEdited, this, &EditableTextBox::on_text_edited);
      m_current_connection = m_current->connect_update_signal(
        [=] (const auto& value) { on_current(value); });
      connect(this, &QLineEdit::cursorPositionChanged, this,
        std::bind_front(&EditableTextBox::on_cursor_position, this));
      connect(this, &QLineEdit::selectionChanged, this,
        std::bind_front(&EditableTextBox::on_selection, this));
      m_highlight->connect_update_signal(
        std::bind_front(&EditableTextBox::on_highlight, this));
      m_text_box->setCursor(cursor());
      m_text_box->setFocusPolicy(focusPolicy());
      m_text_box->setFocusProxy(this);
      m_text_box->installEventFilter(this);
      auto layout = new QHBoxLayout(m_text_box);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(this);
      add_pseudo_element(*m_text_box, Placeholder());
      m_placeholder_style_connection = connect_style_signal(
        *m_text_box, Placeholder(), [=] { on_placeholder_style(); });
    }

    void set_display_text(const QString& text, bool is_elided) {
      m_text_validator->m_is_text_elided = is_elided;
      setText(text);
    }

    const QString& get_submission() const {
      return m_submission;
    }

    void set_placeholder(const QString& placeholder) {
      m_placeholder = placeholder;
      elide_placeholder_text();
      update();
    }

    void set_style(
        const BoxGeometry& geometry, const TextStyleProperties& text_style) {
      m_text_box->layout()->setContentsMargins(
        geometry.get_border_left() + geometry.get_padding_left(),
        geometry.get_border_top() + geometry.get_padding_top(),
        geometry.get_border_right() + geometry.get_padding_right(),
        geometry.get_border_bottom() + geometry.get_padding_bottom());
      auto stylesheet = QString(
        R"(#0x%1 {
          background: transparent;
          border-width: 0px;
          padding: 0px;)").arg(reinterpret_cast<std::intptr_t>(this));
      auto map = StyleSheetMap([] {});
      map.set("color", text_style.m_text_color);
      map.write(stylesheet);
      if(text_style.m_alignment != alignment()) {
        setAlignment(text_style.m_alignment);
      }
      auto font = text_style.m_font;
      font.setPixelSize(text_style.m_size);
      setFont(font);
      if(text_style.m_echo_mode != echoMode()) {
        setEchoMode(text_style.m_echo_mode);
      }
      if(stylesheet != styleSheet()) {
        setStyleSheet(stylesheet);
      }
    }

    connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const {
      return m_submit_signal.connect(slot);
    }

    connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const {
      return m_reject_signal.connect(slot);
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<QMouseEvent*>(event);
        // TODO: based on the original, the TextBox should recieve this event first,
        //        but calling sendEvent winds up back here in an infinite loop.
        //QCoreApplication::sendEvent(m_text_box, event);
        e->accept();
        e->setLocalPos(mapFromGlobal(e->globalPos()));
        mousePressEvent(e);
      } else if(event->type() == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent*>(event);
        if(e->key() == Qt::Key_Escape) {
          if(m_submission != m_current->get()) {
            m_current->set(m_submission);
            return true;
          }
        } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
          m_has_update = true;
          on_editing_finished();
          return true;
        } else if(!m_is_handling_key_press) {
          m_is_handling_key_press = true;
          QLineEdit::keyPressEvent(e);
          m_is_handling_key_press = false;
          if(e->isAccepted()) {
            return true;
          }
        }
      }
      return QLineEdit::eventFilter(watched, event);
    }
    void focusInEvent(QFocusEvent* event) override {
      auto focus_event = static_cast<QFocusEvent*>(event);
      if(focus_event->reason() != Qt::ActiveWindowFocusReason &&
          focus_event->reason() != Qt::PopupFocusReason) {
        m_text_validator->m_is_text_elided = false;
        if(text() != m_current->get()) {
          setText(m_current->get());
        }
      }
      QLineEdit::focusInEvent(event);
    }
    void focusOutEvent(QFocusEvent* event) override {
      auto focusEvent = static_cast<QFocusEvent*>(event);
      if(focusEvent->lostFocus() &&
          focusEvent->reason() != Qt::ActiveWindowFocusReason &&
          focusEvent->reason() != Qt::PopupFocusReason) {
        m_text_box->update_display_text();
      }
      QLineEdit::focusOutEvent(event);
    }
    void paintEvent(QPaintEvent* event) override {
      if(is_placeholder_visible()) {
        auto painter = QPainter(this);
        painter.setPen(m_placeholder_style.m_text_color);
        painter.setFont(m_placeholder_style.m_font);
        painter.drawText(contentsRect(),
          m_placeholder_style.m_alignment, m_elided_placeholder);
      }
      QLineEdit::paintEvent(event);
    }
    void resizeEvent(QResizeEvent* event) override {
      elide_placeholder_text();
      QLineEdit::resizeEvent(event);
    }

  private:
    mutable SubmitSignal m_submit_signal;
    mutable RejectSignal m_reject_signal;
    TextBox* m_text_box;
    std::shared_ptr<TextModel> m_current;
    QString m_submission;
    std::shared_ptr<HighlightModel> m_highlight;
    TextValidator* m_text_validator;
    QString m_placeholder;
    QString m_elided_placeholder;
    TextStyleProperties m_placeholder_style;
    bool m_is_rejected;
    bool m_has_update;
    bool m_is_handling_key_press;
    scoped_connection m_current_connection;
    scoped_connection m_placeholder_style_connection;

    void elide_placeholder_text() {
      m_elided_placeholder =
        fontMetrics().elidedText(m_placeholder, Qt::ElideRight, width());
    }
    bool is_placeholder_visible() const {
      return !isReadOnly() &&
        !m_placeholder.isEmpty() && m_current->get().isEmpty();
    }
    void on_current(const QString& current) {
      m_has_update = true;
      if(m_is_rejected) {
        m_is_rejected = false;
        unmatch(*m_text_box, Rejected());
      }
    }
    void on_cursor_position(int old_position, int new_position) {
      if(hasSelectedText()) {
        on_selection();
      } else if(m_highlight->get() != Highlight(cursorPosition())) {
        m_highlight->set(Highlight(cursorPosition()));
      }
    }
    void on_editing_finished() {
      if(!isReadOnly() && m_has_update) {
        if(m_current->get_state() == QValidator::Acceptable) {
          m_submission = m_current->get();
          m_has_update = false;
          m_submit_signal(m_submission);
        } else {
          m_reject_signal(m_current->get());
          m_current->set(m_submission);
          if(!m_is_rejected) {
            m_is_rejected = true;
            match(*m_text_box, Rejected());
          }
        }
      }
    }
    void on_highlight(const Highlight& highlight) {
      if(is_collapsed(highlight)) {
        auto blocker = QSignalBlocker(this);
        deselect();
        setCursorPosition(highlight.m_end);
      } else if(highlight !=
          Highlight(selectionStart(), selectionEnd())) {
        auto blocker = QSignalBlocker(this);
        setSelection(highlight.m_start, get_size(highlight));
      }
    }
    void on_placeholder_style() {
      auto& placeholder_stylist = *find_stylist(*m_text_box, Placeholder());
      m_placeholder_style = TextStyleProperties();
      for(auto& property : placeholder_stylist.get_computed_block()) {
        property.visit(
          [&] (const TextColor& color) {
            placeholder_stylist.evaluate(color, [=] (auto color) {
              m_placeholder_style.m_text_color = color;
            });
          },
          [&] (const TextAlign& alignment) {
            placeholder_stylist.evaluate(alignment, [=] (auto alignment) {
              m_placeholder_style.m_alignment = alignment;
            });
          },
          [&] (const Font& font) {
            placeholder_stylist.evaluate(font, [=] (auto font) {
              m_placeholder_style.m_font = font;
            });
          },
          [&] (const FontSize& size) {
            placeholder_stylist.evaluate(size, [=] (auto size) {
              m_placeholder_style.m_size = size;
            });
          });
      }
      if(is_placeholder_visible()) {
        update();
      }
    }
    void on_selection() {
      if(!hasSelectedText()) {
        on_cursor_position(0, cursorPosition());
      } else if(m_highlight->get() !=
          Highlight(selectionStart(), selectionEnd())) {
        m_highlight->set({selectionStart(), selectionEnd()});
      }
    }
    void on_text_edited(const QString& text) {
      m_current->set(text);
    }
};

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(color));
}

TextBox::TextStyleProperties::TextStyleProperties()
  : m_alignment(Qt::AlignLeft | Qt::AlignVCenter),
    m_size(scale_height(12)),
    m_echo_mode(QLineEdit::Normal) {}

TextBox::TextBox(QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(), parent) {}

TextBox::TextBox(QString current, QWidget* parent)
  : TextBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

TextBox::TextBox(std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_highlight(std::make_shared<LocalValueModel<Highlight>>()),
      m_editable_text_box(nullptr) {
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  m_current_connection = m_current->connect_update_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<TextModel>& TextBox::get_current() const {
  return m_current;
}

const QString& TextBox::get_submission() const {
  if(m_editable_text_box) {
    return m_editable_text_box->get_submission();
  }
  return m_current->get();
}

const std::shared_ptr<HighlightModel>& TextBox::get_highlight() const {
  return m_highlight;
}

void TextBox::set_placeholder(const QString& placeholder) {
  m_placeholder = placeholder;
  if(m_editable_text_box) {
    m_editable_text_box->set_placeholder(m_placeholder);
  }
}

bool TextBox::is_read_only() const {
  return m_editable_text_box && m_editable_text_box->isReadOnly();
}

void TextBox::set_read_only(bool read_only) {
  if(!m_editable_text_box && read_only) {
    return;
  } else if(!m_editable_text_box) {
    initialize_editable_text_box();
  }
  m_editable_text_box->setReadOnly(read_only);
  m_editable_text_box->setCursorPosition(0);
  setCursor(m_editable_text_box->cursor());
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  update_display_text();
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
  auto metrics = QFontMetrics(m_text_style.m_font);
  m_size_hint.emplace(
    metrics.horizontalAdvance(m_current->get()) +
      cursor_width, metrics.height());
  *m_size_hint += compute_decoration_size();
  return *m_size_hint;
}

void TextBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    if(!m_editable_text_box && isEnabled()) {
      initialize_editable_text_box();
    }
    update_display_text();
  }
  QWidget::changeEvent(event);
}

void TextBox::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  m_box_painter.paint(painter);
  if(!m_editable_text_box && !m_current->get().isEmpty()) {
    painter.setPen(m_text_style.m_text_color);
    painter.setFont(m_text_style.m_font);
    painter.drawText(
      m_geometry.get_content_area(), m_text_style.m_alignment, m_display_text);
  }
}

void TextBox::resizeEvent(QResizeEvent* event) {
  m_geometry.set_size(size());
  update_display_text();
  QWidget::resizeEvent(event);
}

void TextBox::showEvent(QShowEvent* event) {
  if(!m_editable_text_box && (is_read_only() || isEnabled())) {
    initialize_editable_text_box();
  }
  QWidget::showEvent(event);
}

QSize TextBox::compute_decoration_size() const {
  return m_geometry.get_geometry().size() -
    m_geometry.get_content_area().size();
}

void TextBox::elide_text() {
  auto font_metrics = QFontMetrics(m_text_style.m_font);
  m_display_text = font_metrics.elidedText(
    m_current->get(), Qt::ElideRight, m_geometry.get_content_area().width());
  if(m_editable_text_box && m_display_text != m_editable_text_box->text()) {
    m_editable_text_box->set_display_text(
      m_display_text, m_display_text != m_current->get());
    m_editable_text_box->setCursorPosition(0);
  }
}

void TextBox::initialize_editable_text_box() {
  m_editable_text_box = new EditableTextBox(m_current, m_highlight, this);
  m_editable_text_box->set_placeholder(m_placeholder);
  on_style();
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_editable_text_box &&
      m_editable_text_box->text() != m_current->get()) {
    m_display_text = m_current->get();
    m_editable_text_box->set_display_text(m_display_text, false);
  }
  m_size_hint = none;
  updateGeometry();
  update();
}

void TextBox::on_current(const QString& current) {
  update_display_text();
}

void TextBox::on_style() {
  auto& stylist = find_stylist(*this);
  m_text_style = TextStyleProperties();
  for(auto& property : stylist.get_computed_block()) {
    apply(property, m_geometry, stylist);
    apply(property, m_box_painter, stylist);
    property.visit(
      [&] (const TextColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_text_style.m_text_color = color;
        });
      },
      [&] (const TextAlign& alignment) {
        stylist.evaluate(alignment, [=] (auto alignment) {
          m_text_style.m_alignment = alignment;
        });
      },
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (const auto& font) {
          m_text_style.m_font = font;
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_text_style.m_size = size;
        });
      },
      [&] (const EchoMode& mode) {
        stylist.evaluate(mode, [=] (auto mode) {
          m_text_style.m_echo_mode = mode;
        });
      });
  }
  if(m_editable_text_box) {
    m_editable_text_box->set_style(m_geometry, m_text_style);
  }
  update_display_text();
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
