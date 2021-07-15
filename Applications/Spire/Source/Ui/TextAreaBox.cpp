#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
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
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

#include <QApplication>

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
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(LineHeight(1.25)).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor::fromRgb(0, 0, 0)));
      // TODO: the ScrollBox seems to maintain its own padding,
      //        so remove it with this
      //set(horizontal_padding(0)).
      //set(vertical_padding(0));
    style.get(Any() >> is_a<Box>()).
      set(border_size(0));
      //set(horizontal_padding(scale_width(8))).
      //set(vertical_padding(scale_height(7)));
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

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
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
      m_is_rejected(false),
      m_document_height(0) {
  //setObjectName("tab");
  //setStyleSheet("#tab { background-color: red; }");
  //m_layers = new LayeredWidget(this);
  //layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // TODO: set current
  m_text_edit = new TextEdit(this, this);
  m_text_edit->setAcceptRichText(false);
  // TODO: reset to 0
  m_text_edit->document()->setDocumentMargin(0);
  m_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setFrameShape(QFrame::NoFrame);
  //m_text_edit->setLineWrapMode(QTextEdit::NoWrap);
  //m_text_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_text_edit);
  m_text_edit->installEventFilter(this);
  //m_text_edit->verticalScrollBar()->setDisabled(true);
  //m_text_edit->verticalScrollBar()->setValue(0);
  //m_layers->add(m_text_edit);
  m_placeholder = new QLabel();
  m_placeholder->setCursor(m_text_edit->cursor());
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setWordWrap(true);
  m_placeholder->setMargin(0);
  m_placeholder->setIndent(0);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_placeholder->setFocusPolicy(Qt::NoFocus);
  //m_layers->add(m_placeholder);
  //auto box = new Box(layers, this);
  //box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //box->setFocusPolicy(Qt::NoFocus);
  //box->setFocusProxy(m_text_edit);
  //m_layer_container = new Box(m_layers);
  m_text_edit_box = new Box(m_text_edit, this);
  m_text_edit_box->setObjectName("text_edit_box");
  //m_text_edit_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_scroll_box = new ScrollBox(m_text_edit_box, this);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER,
    ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  //m_container_box = new Box(scroll_box, this);
  //m_container_box->setFocusPolicy(Qt::NoFocus);
  //layout->addWidget(m_container_box);
  //auto scroll_area = new ScrollBox();
  //scroll_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //m_box = new Box();
  //m_box->setFocusProxy(m_text_edit);
  //setCursor(m_text_edit->cursor());
  //setFocusPolicy(m_text_edit->focusPolicy());
  //auto layout = new QHBoxLayout(this);
  //layout->setContentsMargins(0, 0, 0, 0);
  //layout->addWidget(m_box);
  //setFocusProxy(m_box);
  proxy_style(*this, *m_scroll_box);
  add_pseudo_element(*this, Placeholder());
  connect_style_signal(*this, [=] { on_style(); });
  connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_text_edit, &QTextEdit::cursorPositionChanged, this,
    &TextAreaBox::on_cursor_position);
  //connect(m_text_edit->document()->documentLayout(),
  //  &QAbstractTextDocumentLayout::documentSizeChanged,
  //    // TODO: don't capture by reference, created method
  //    [&] (const auto& size) {
  //      m_document_height = size.toSize().height();    
  //      qDebug() << "doc height: " << m_document_height;
  //      //if(size.toSize() != m_text_edit->size()) {
  //      //  m_text_edit->setFixedSize(size.toSize());
  //      //}
  //      //if(size.height() != line_count() * m_line_height) {
  //      //  for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
  //      //    auto block = m_text_edit->document()->findBlockByNumber(i);
  //      //    if(block.isValid()) {
  //      //      auto cursor = m_text_edit->textCursor();
  //      //      cursor.setPosition(block.position());
  //      //      m_text_edit->setTextCursor(cursor);
  //      //      auto block_format = cursor.blockFormat();
  //      //      block_format.setLineHeight(m_line_height,
  //      //        QTextBlockFormat::FixedHeight);
  //      //      cursor.setBlockFormat(block_format);
  //      //      m_text_edit->setTextCursor(cursor);
  //      //    }
  //      //  }
  //      //}
  //      //qDebug() << size;
  //    });
  connect(m_text_edit, &QTextEdit::textChanged, this,
    &TextAreaBox::on_text_changed);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) { on_current(value); });

  //qApp->installEventFilter(this);
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
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  elide_current();
  update_display_text();
  update_placeholder_text();
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  //if(m_size_hint) {
  //  return *m_size_hint;
  //}
  //auto cursor_width = [&] {
  //  if(is_read_only()) {
  //    return 0;
  //  }
  //  return 1;
  //}();
  //m_size_hint.emplace(
  //  m_text_edit->fontMetrics().horizontalAdvance(m_model->get_current()) +
  //    cursor_width, m_text_edit->fontMetrics().height());
  //*m_size_hint += compute_decoration_size();
  //return m_text_edit->sizeHint() + compute_decoration_size();
  return m_text_edit->sizeHint();
}

bool TextAreaBox::eventFilter(QObject* watched, QEvent* event) {
  //if(event->type() == QEvent::FocusIn) {
  //  qDebug() << watched;
  //}
  //qDebug() << m_text_edit->size();
  if(event->type() == QEvent::FocusIn) {
    auto focus_event = static_cast<QFocusEvent*>(event);
    if(focus_event->reason() != Qt::ActiveWindowFocusReason &&
        focus_event->reason() != Qt::PopupFocusReason) {
      //m_text_validator->m_is_text_elided = false;
      //if(m_text_edit->toPlainText() != m_model->get_current()) {
      //  m_text_edit->setText(m_model->get_current());
      //}
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->lostFocus() &&
        focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      on_editing_finished();
      update_display_text();
    }
  } else if(event->type() == QEvent::Resize) {
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
  //qDebug() << "press";
  m_text_edit->setFocus();
  if(is_placeholder_shown()) {
    m_text_edit->setFocus();
  }
  QWidget::mousePressEvent(event);
}

void TextAreaBox::resizeEvent(QResizeEvent* event) {
  //qDebug() << m_layers->size();
  //qDebug() << "min: " << minimumSize();
  //qDebug() << "max: " << maximumSize();
  update_text_box_size();
  update_display_text();
  update_placeholder_text();
  QWidget::resizeEvent(event);
}

QSize TextAreaBox::compute_decoration_size() const {
  auto decoration_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*m_scroll_box)) {
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

void TextAreaBox::elide_current() {
  //m_text_edit->blockSignals(true);
  //if(line_count() > visible_line_count() && visible_line_count() > 0) {
  //  while(line_count() - 1 > visible_line_count()) {
  //    qDebug() << "lc: " << line_count();
  //    qDebug() << "vlc: " << visible_line_count();
  //    auto cursor = m_text_edit->textCursor();
  //    cursor.movePosition(QTextCursor::End);
  //    cursor.select(QTextCursor::LineUnderCursor);
  //    cursor.removeSelectedText();
  //    //cursor.deletePreviousChar(); // Added to trim the newline char when removing last line
  //    m_text_edit->setTextCursor(cursor);
  //  }
  //  auto cursor = m_text_edit->textCursor();
  //  cursor.setPosition(m_text_edit->toPlainText().length());
  //  cursor.deletePreviousChar();
  //  cursor.deletePreviousChar();
  //  cursor.deletePreviousChar();
  //  cursor.insertText("...");
  //}
  //m_text_edit->blockSignals(false);
}

void TextAreaBox::update_display_text() {
  //if(!isEnabled() || is_read_only() || !hasFocus()) {
  //  elide_text();
  //} else if(m_text_edit->toPlainText() != m_model->get_current()) {
  //  m_text_edit->setText(m_model->get_current());
  //}
  //}
  m_size_hint = none;
  updateGeometry();
}

void TextAreaBox::update_placeholder_text() {
  //qDebug() << window()->focusWidget();
  if(is_placeholder_shown()) {
    // TODO: elide
    m_placeholder->setText(m_placeholder_text);
    //qDebug() << "show";
    m_placeholder->show();
  } else {
    //qDebug() << "hide";
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
    //qDebug() << *m_text_edit_styles.m_line_height;
    m_line_height = static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height;
    //qDebug() << m_line_height;
    m_scroll_box->get_vertical_scroll_bar().set_line_size(m_line_height);
    auto cursor_pos = m_text_edit->textCursor().position();
    for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
      auto block = m_text_edit->document()->findBlockByNumber(i);
      if(block.isValid()) {
        auto cursor = m_text_edit->textCursor();
        cursor.setPosition(block.position());
        m_text_edit->setTextCursor(cursor);
        auto block_format = cursor.blockFormat();
        block_format.setLineHeight(m_line_height,
          QTextBlockFormat::FixedHeight);
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
  m_text_edit->set_decoration_size(compute_decoration_size());
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

int TextAreaBox::line_count() const {
  auto count = 0;
  QStringList ret;
  QTextBlock tb = m_text_edit->document()->begin();
  while(tb.isValid())
  {
    QString blockText = tb.text();
    //Q_ASSERT(tb.layout());
    if(!tb.layout())
      continue;
    for(int i = 0; i != tb.layout()->lineCount(); ++i)
    {
      QTextLine line = tb.layout()->lineAt(i);
      ret.append(blockText.mid(line.textStart(), line.textLength()));
    }
    tb = tb.next();
  }
  return ret.size();
}

void TextAreaBox::update_text_box_size() {
  // TODO: optimize to so this isn't called too many times.
  auto h_adjust = [&] {
    if(line_count() > visible_line_count()) {
      // TODO: unshown scroll bar gives 100 as width, either fix or get this
      //      value from somewhere else.
      return scale_width(15);
    }
    return 0;
  }();
  //qDebug() << "size set";
  //qDebug() << "dec size: " << compute_decoration_size().width();
  m_text_edit->set_scroll_bar_visible(line_count() > visible_line_count());
  m_text_edit_box->setFixedSize(m_text_edit->size());
  //m_text_edit_box->setFixedSize(
  //  width() - h_adjust - compute_decoration_size().width(),
  //  std::max(
  //    m_text_edit->document()->documentLayout()->documentSize().toSize().height(),
  //    m_line_height));// - compute_decoration_size().height());
  //m_text_edit_box->
  //m_scroll_box->get_vertical_scroll_bar().set_page_size(10);
  //m_text_edit->document()->documentLayout()->update();
}

int TextAreaBox::visible_line_count() const {
  return (m_text_edit->viewport()->height() -
    compute_decoration_size().height()) / m_line_height;
}

void TextAreaBox::on_current(const QString& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
  update_display_text();
  update_placeholder_text();
}

void TextAreaBox::on_cursor_position() {
  // TOOD: clean up, use set vertical padding
  //static auto last = m_text_edit->cursorRect();
  auto t = m_text_edit->visibleRegion().boundingRect().top();
  auto b = m_text_edit->visibleRegion().boundingRect().bottom();
  //qDebug() << "************************************";
  //qDebug() << "vr top: " << t;
  //qDebug() << "vr bottom: " << b;
  //qDebug() << "cur: " << m_text_edit->cursorRect();
  if(m_scroll_box->get_vertical_scroll_bar().isVisible()) {
    if(m_text_edit->cursorRect().top() <= t) {
    //if(m_text_edit->cursorRect().y() < m_text_edit->viewport()->y()) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().top() - scale_height(8));
    } else if(m_text_edit->cursorRect().bottom() >= b) {
    //} else if(m_text_edit->cursorRect().y() > m_text_edit->viewport()->y()) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().bottom() -
        m_text_edit->visibleRegion().boundingRect().height() +
        scale_height(8));
    }
  }
  //last = m_text_edit->cursorRect();
  //update();
}

void TextAreaBox::on_editing_finished() {
  if(!is_read_only()) {
    if(m_model->get_state() == QValidator::Acceptable) {
      m_submission = m_model->get_current();
      m_submit_signal(m_submission);
    } else {
      m_model->set_current(m_submission);
      if(!m_is_rejected) {
        m_is_rejected = true;
        match(*this, Rejected());
      }
    }
  }
}

void TextAreaBox::on_text_changed() {
  //update_placeholder_text();
  m_model->set_current(m_text_edit->toPlainText());
  // TODO: put in method and call from resizeEvent, etc.
  //qDebug() << line_count();
  //qDebug() << "doc height: " << m_text_edit->document()->size().toSize().height();
  update_text_box_size();
  //m_layer_container->setFixedSize(m_text_edit->size());
  //qDebug() << "text change size: " << m_text_edit->size();
  //qDebug() << "text change pos: " << m_text_edit->pos();
  //qDebug() << m_text_edit->textCursor().blockFormat().headingLevel();
  //qDebug() << m_text_edit->textCursor().blockFormat().lineHeight();
  //qDebug() << m_text_edit->textCursor().verticalMovementX();
  //qDebug() << "vbar: " << m_text_edit->verticalScrollBar()->value();
  //on_cursor_position();
  //m_text_edit->verticalScrollBar()->setValue(0);
  //m_text_edit->ensureCursorVisible();
  //auto a = m_text_edit->textCursor();
  //a.setVerticalMovementX(0);
  //m_text_edit->setTextCursor(a);
  //qDebug() << m_text_edit->textCursor().blockFormat().topMargin();
  //qDebug() << m_text_edit->textCursor().blockFormat().bottomMargin();
  //updateGeometry();
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
