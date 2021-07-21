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

#include <QPainter>
#include <QtMath>
#include <QPlainTextEdit>
#include <QApplication>
#include <QAbstractTextDocumentLayout>

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
      set(BackgroundColor(QColor::fromRgb(0xFF, 0x0, 0x0, 0))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(0)).
      set(vertical_padding(0));
    style.get(Body()).
      set(border_size(0)).
      set(LineHeight(1.25)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_width(7))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor::fromRgb(0, 0, 0)));
    style.get(ReadOnly() > Body()).
      set(BackgroundColor(QColor::fromRgb(0, 255, 0))).
      set(horizontal_padding(0)).
      set(vertical_padding(0));
    style.get(Body() / Body()).
      set(horizontal_padding(0)).
      set(vertical_padding(0));
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
    style.get(Placeholder()).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignTop)).
      set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() / Placeholder()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  class TextAreaBoxLayout : public QAbstractTextDocumentLayout {
    public:

      TextAreaBoxLayout(QTextEdit* edit):
        QAbstractTextDocumentLayout(edit->document()),
        m_text_edit(edit) {}

      QRectF blockBoundingRect(const QTextBlock &block) const override {
        //qDebug() << block.text();
        if(!block.isValid()) {
          return QRectF();
        }
        QTextLayout* tl = block.layout();
        if (!tl->lineCount()) {
          const_cast<TextAreaBoxLayout*>(this)->layoutBlock(block);
        }
        QRectF br;
        if (block.isVisible()) {
            br = QRectF(QPointF(0, 0), tl->boundingRect().bottomRight());
            if (tl->lineCount() == 1)
                br.setWidth(qMax(br.width(), tl->lineAt(0).naturalTextWidth()));
            qreal margin = document()->documentMargin();
            br.adjust(0, 0, margin, 0);
            if (!block.next().isValid()) {
              br.adjust(0, 0, 0, margin);
            }
        }
        return m_text_edit->rect();
      }

      QSizeF documentSize() const override {
        //return QSizeF(maximumWidth, document()->lineCount());
        if(document()->lineCount() == 0) {
          return {};
        }

      }

      void draw(QPainter *painter,
          const QAbstractTextDocumentLayout::PaintContext &context) override {
        painter->save();
        //qDebug() << "draw";
        for(auto i = 0; i < document()->blockCount(); ++i) {
          auto block = document()->findBlockByNumber(i);
          if(block.isValid()) {
            block.layout()->draw(painter, {0.0, 0.0});
          }
        }
        painter->restore();
        //QAbstractTextDocumentLayout::draw(painter, context);
      }

      QRectF frameBoundingRect(QTextFrame *frame) const override {
        return QRectF(0, 0, qMax(width, maximumWidth), qreal(INT_MAX));
      }

      int	hitTest(const QPointF &point,
          Qt::HitTestAccuracy accuracy) const override {
        qDebug() << "hitTest: " << point;
        // TODO: used to be = topBlock
        int currentBlockNumber = 0;
        QTextBlock currentBlock = document()->findBlockByNumber(currentBlockNumber);
        if (!currentBlock.isValid())
            return -1;

        auto *documentLayout = static_cast<TextAreaBoxLayout*>(
          document()->documentLayout());
        Q_ASSERT(documentLayout);

        QPointF offset;
        QRectF r = documentLayout->blockBoundingRect(currentBlock);
        while (currentBlock.next().isValid() && r.bottom() + offset.y() <= point.y()) {
            offset.ry() += r.height();
            currentBlock = currentBlock.next();
            ++currentBlockNumber;
            r = documentLayout->blockBoundingRect(currentBlock);
        }
        while (currentBlock.previous().isValid() && r.top() + offset.y() > point.y()) {
            offset.ry() -= r.height();
            currentBlock = currentBlock.previous();
            --currentBlockNumber;
            r = documentLayout->blockBoundingRect(currentBlock);
        }


        if (!currentBlock.isValid())
            return -1;
        QTextLayout *layout = currentBlock.layout();
        int off = 0;
        QPointF pos = point - offset;
        for (int i = 0; i < layout->lineCount(); ++i) {
            QTextLine line = layout->lineAt(i);
            const QRectF lr = line.naturalTextRect();
            if (lr.top() > pos.y()) {
                off = qMin(off, line.textStart());
            } else if (lr.bottom() <= pos.y()) {
                off = qMax(off, line.textStart() + line.textLength());
            } else {
                off = line.xToCursor(pos.x(), m_text_edit->overwriteMode() ?
                                     QTextLine::CursorOnCharacter : QTextLine::CursorBetweenCharacters);
                break;
            }
        }

        return currentBlock.position() + off;
      }

      int	pageCount() const override {
        return 1;
      }

    protected:
      void documentChanged(int from, int charsRemoved, int charsAdded) override {
        QTextDocument *doc = document();
        int newBlockCount = doc->blockCount();
        int charsChanged = charsRemoved + charsAdded;

        QTextBlock changeStartBlock = doc->findBlock(from);
        QTextBlock changeEndBlock = doc->findBlock(qMax(0, from + charsChanged - 1));
        bool blockVisibilityChanged = false;

        if (changeStartBlock == changeEndBlock && newBlockCount == blockCount) {
            QTextBlock block = changeStartBlock;
            if (block.isValid() && block.length()) {
                QRectF oldBr = blockBoundingRect(block);
                layoutBlock(block);
                QRectF newBr = blockBoundingRect(block);
                if (newBr.height() == oldBr.height()) {
                    if (!blockUpdate)
                        emit updateBlock(block);
                    return;
                }
            }
        } else {
            QTextBlock block = changeStartBlock;
            do {
                block.clearLayout();
                if (block.isVisible()
                        ? (block.lineCount() == 0)
                        : (block.lineCount() > 0)) {
                    blockVisibilityChanged = true;
                    block.setLineCount(block.isVisible() ? 1 : 0);
                }
                if (block == changeEndBlock)
                    break;
                block = block.next();
            } while(block.isValid());
        }

        //qDebug() << "bc: " << blockCount;
        if (newBlockCount != blockCount || blockVisibilityChanged) {
            int changeEnd = changeEndBlock.blockNumber();
            int blockDiff = newBlockCount - blockCount;
            int oldChangeEnd = changeEnd - blockDiff;

            if (maximumWidthBlockNumber > oldChangeEnd)
                maximumWidthBlockNumber += blockDiff;

            blockCount = newBlockCount;
            if (blockCount == 1)
                maximumWidth = blockWidth(doc->firstBlock());

            if (!blockDocumentSizeChanged)
                emit documentSizeChanged(documentSize());

            if (blockDiff == 1 && changeEnd == newBlockCount -1 ) {
                if (!blockUpdate) {
                    QTextBlock b = changeStartBlock;
                    for(;;) {
                        emit updateBlock(b);
                        if (b == changeEndBlock)
                            break;
                        b = b.next();
                    }
                }
                return;
            }
        }

        if (!blockUpdate)
            emit update(QRectF(0., -doc->documentMargin(), 1000000000., 1000000000.));
      }

    private:
      QTextEdit* m_text_edit;
      mutable int width = 0;
      mutable int maximumWidth = 0;
      mutable int maximumWidthBlockNumber = 0;
      mutable bool blockDocumentSizeChanged = false;
      mutable int blockCount = 0;
      mutable bool blockUpdate = false;

      void layoutBlock(const QTextBlock &block) {
          QTextDocument *doc = document();
          qreal margin = doc->documentMargin();
          qreal blockMaximumWidth = 0;

          qreal height = 0;
          QTextLayout *tl = block.layout();
          QTextOption option = doc->defaultTextOption();
          tl->setTextOption(option);

          int extraMargin = 0;
          if (option.flags() & QTextOption::AddSpaceForLineAndParagraphSeparators) {
              QFontMetrics fm(block.charFormat().font());
              extraMargin += fm.horizontalAdvance(QChar(0x21B5));
          }
          tl->beginLayout();
          qreal availableWidth = m_text_edit->width();
          if (availableWidth <= 0) {
              availableWidth = qreal(INT_MAX);
          }
          availableWidth -= 2*margin + extraMargin;
          while (1) {
              QTextLine line = tl->createLine();
              if (!line.isValid())
                  break;
              line.setLeadingIncluded(true);
              line.setLineWidth(availableWidth);
              line.setPosition(QPointF(margin, height));
              height += line.height();
              if (line.leading() < 0)
                  height += qCeil(line.leading());
              blockMaximumWidth = qMax(blockMaximumWidth, line.naturalTextWidth() + 2*margin);
          }
          tl->endLayout();

          int previousLineCount = doc->lineCount();
          const_cast<QTextBlock&>(block).setLineCount(block.isVisible() ? tl->lineCount() : 0);
          int lineCount = doc->lineCount();

          bool emitDocumentSizeChanged = previousLineCount != lineCount;
          if (blockMaximumWidth > maximumWidth) {
              // new longest line
              maximumWidth = blockMaximumWidth;
              maximumWidthBlockNumber = block.blockNumber();
              emitDocumentSizeChanged = true;
          } else if (block.blockNumber() == maximumWidthBlockNumber &&
                blockMaximumWidth < maximumWidth) {
              // longest line shrinking
              QTextBlock b = doc->firstBlock();
              maximumWidth = 0;
              QTextBlock maximumBlock;
              while (b.isValid()) {
                  qreal blockMaximumWidth = blockWidth(b);
                  if (blockMaximumWidth > maximumWidth) {
                      maximumWidth = blockMaximumWidth;
                      maximumBlock = b;
                  }
                  b = b.next();
              }
              if (maximumBlock.isValid()) {
                  maximumWidthBlockNumber = maximumBlock.blockNumber();
                  emitDocumentSizeChanged = true;
              }
          }
          if (emitDocumentSizeChanged && !blockDocumentSizeChanged)
            //qDebug() << "changed";
            Q_EMIT documentSizeChanged(documentSize());
      }

      qreal blockWidth(const QTextBlock &block) const {
        QTextLayout *layout = block.layout();
        if (!layout->lineCount())
            return 0; // only for layouted blocks
        qreal blockWidth = 0;
        for (int i = 0; i < layout->lineCount(); ++i) {
            QTextLine line = layout->lineAt(i);
            blockWidth = qMax(line.naturalTextWidth() + 8, blockWidth);
        }
        return blockWidth;
      }
  };
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
      m_longest_line_length(0),
      m_longest_line_block(0),
      m_document_height(0) {
  //new QPlainTextEdit();
  m_text_edit = new QTextEdit(this);
  m_text_edit->document()->setDocumentLayout(
    new TextAreaBoxLayout(m_text_edit));
  m_text_edit->setAcceptRichText(false);
  m_text_edit->document()->setDocumentMargin(0);
  m_text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setFrameShape(QFrame::NoFrame);
  connect(m_text_edit, &QTextEdit::textChanged, this,
    &TextAreaBox::on_text_changed);
  connect(m_text_edit->document()->documentLayout(),
    &QAbstractTextDocumentLayout::documentSizeChanged, this,
    &TextAreaBox::on_document_size);
  connect(m_text_edit->document(), &QTextDocument::contentsChange, this,
    &TextAreaBox::on_contents_changed);
  setFocusProxy(m_text_edit);
  //m_text_edit->installEventFilter(this);
  m_container_box = new Box(m_text_edit, this);
  m_container_box->setFocusPolicy(Qt::NoFocus);
  m_scroll_box = new ScrollBox(m_container_box, this);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER,
    ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  proxy_style(*this, *m_scroll_box);
  add_pseudo_element(*this, Placeholder());
  connect_style_signal(*this, [=] { on_style(); });
  connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_text_edit, &QTextEdit::cursorPositionChanged, this,
    &TextAreaBox::on_cursor_position);
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
  //update_display_text();
  update_placeholder_text();
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  //qDebug() << "dec: " << compute_decoration_size();
  //qDebug() << "lc: " << line_count();
  //qDebug() << "l h: " << m_line_height;
  //qDebug() << "long: " << m_longest_line_length;
  //qDebug() << "cursor: " << m_text_edit->cursorWidth();
  //auto a = compute_decoration_size() +
  //  QSize(m_longest_line_length + m_text_edit->cursorWidth(),
  //// TODO: what is magic number 3?
  //    line_count() * m_line_height + 3);
  ////qDebug() << a;
  //return a;
  return m_text_edit->document()->size().toSize() + QSize(2, 2) +
    compute_decoration_size();
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
  update_page_size();
  //update_display_text();
  update_placeholder_text();
  qDebug() << "te size: " << m_text_edit->size();
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
      });
  }
  for(auto& property : get_evaluated_block(*m_container_box)) {
    property.visit(
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
  // TODO: find out why border isn't being added.
  return decoration_size + QSize(2, 2);
}

bool TextAreaBox::is_placeholder_shown() const {
  return !is_read_only() && m_model->get_current().isEmpty() &&
    !m_placeholder_text.isEmpty();
}

bool TextAreaBox::is_scroll_bar_visible() const {
  return line_count() > visible_line_count();
}

int TextAreaBox::get_text_length(const QString& text) {
  return m_text_edit->fontMetrics().horizontalAdvance(text);
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

void TextAreaBox::update_placeholder_text() {
  //qDebug() << window()->focusWidget();
  if(is_placeholder_shown()) {
    // TODO: elide
    //m_placeholder->setText(m_placeholder_text);
    //qDebug() << "show";
    //m_placeholder->show();
    m_text_edit->setPlaceholderText(m_placeholder_text);
  } else {
    //qDebug() << "hide";
    //m_placeholder->hide();
  }
}

void TextAreaBox::commit_style() {
  auto stylesheet = QString(
    R"(QTextEdit {
      background: aqua;
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
  if(m_text_edit_styles.m_line_height && ((static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height) != m_line_height)) {
    m_line_height = static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height;
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
  //update_display_text();
}

void TextAreaBox::commit_placeholder_style() {
  //auto stylesheet = QString(
  //  R"(QLabel {
  //    background: red;
  //    border-width: 0px;
  //    padding: 0px;)");
  //m_placeholder_styles.m_styles.write(stylesheet);
  //auto alignment = m_placeholder_styles.m_alignment.value_or(
  //  Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  //if(alignment != m_placeholder->alignment()) {
  //  m_placeholder->setAlignment(alignment);
  //}
  //auto font = m_placeholder_styles.m_font.value_or(QFont());
  //if(m_placeholder_styles.m_size) {
  //  font.setPixelSize(*m_placeholder_styles.m_size);
  //}
  //m_placeholder->setFont(font);
  //if(stylesheet != m_placeholder->styleSheet()) {
  //  m_placeholder->setStyleSheet(stylesheet);
  //  m_placeholder->style()->unpolish(this);
  //  m_placeholder->style()->polish(this);
  //}
  //update_placeholder_text();
}

int TextAreaBox::line_count() const {
  auto num = 0;
  for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
    auto block = m_text_edit->document()->findBlockByNumber(i);
    if(block.isValid()) {
      num += block.layout()->lineCount();
    }
  }
  return std::max(1, num);
}

// TODO: reorder definitions alphabetically
void TextAreaBox::update_page_size() {
  //auto scroll_bar_width = [&] {
  //  if(is_scroll_bar_visible()) {
  //    //qDebug() << "sb is vis";
  //    return scale_width(15);
  //  }
  //  return 0;
  //}();
  //if(width() - 2 < m_longest_line_length) {
  //  //qDebug() << "width - 2";
  //  auto a = width() - 2 +
  //    m_text_edit->cursorWidth() - scroll_bar_width - compute_decoration_size().width();
  //  //qDebug() << "a: " << a;
  //  m_text_edit->document()->setTextWidth(a);
  //} else {
  //  //qDebug() << "longest line";
  //  m_text_edit->document()->setTextWidth(m_longest_line_length +
  //    m_text_edit->cursorWidth() - scroll_bar_width + compute_decoration_size().width() - 2);
  //}
  //qDebug() << "text width: " << m_text_edit->document()->textWidth();
  //qDebug() << "page size: " << m_text_edit->document()->pageSize();
  updateGeometry();
}

int TextAreaBox::visible_line_count() const {
  // TODO: get line height
  return (height()) / 15;
}

void TextAreaBox::on_contents_changed(int position, int removed, int added) {
  auto block = m_text_edit->document()->findBlock(position);
  if(block.isValid()) {
    auto line_length = get_text_length(block.text());
    if(line_length > m_longest_line_length) {
      m_longest_line_length = line_length;
      //qDebug() << "new block text: " << block.text();
      m_longest_line_block = block.blockNumber();
    } else if(block.blockNumber() == m_longest_line_block) {
      m_longest_line_length = 0;
      m_longest_line_block = 0;
      for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
        auto block = m_text_edit->document()->findBlockByNumber(i);
        if(block.isValid()) {
          if(get_text_length(block.text()) > m_longest_line_length) {
            m_longest_line_length = get_text_length(block.text());
            m_longest_line_block = block.blockNumber();
          }
        }
      }
    }
    m_added_width = get_text_length(block.text().mid(position, added - removed));
  }
  update_page_size();
}

void TextAreaBox::on_current(const QString& current) {
  if(current != m_text_edit->toPlainText()) {
    m_text_edit->setPlainText(current);
  }
}

void TextAreaBox::on_cursor_position() {
  // TOOD: clean up, use set vertical padding
  auto t = m_text_edit->visibleRegion().boundingRect().top();
  auto b = m_text_edit->visibleRegion().boundingRect().bottom();
  if(m_scroll_box->get_vertical_scroll_bar().isVisible() &&
      !m_text_edit->visibleRegion().boundingRect().contains(m_text_edit->cursorRect())) {
    if(m_text_edit->cursorRect().top() <= t) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().top() - scale_height(8));
    } else if(m_text_edit->cursorRect().bottom() >= b) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().bottom() -
        m_text_edit->visibleRegion().boundingRect().height() +
        scale_height(8));
    }
  }
}

void TextAreaBox::on_document_size(const QSizeF& size) {
  qDebug() << "doc size: " << size;
  //m_text_edit->setFixedSize(size.toSize());
  //m_text_edit->setFixedSize(size.toSize().width(), size.toSize().height() + 15);
  //qDebug() << "te size: " << m_text_edit->size();
  //m_container_box->setFixedSize(m_text_edit->width() + compute_decoration_size().width() + 2,
  //  m_text_edit->height());
  //qDebug() << "cont size: " << m_container_box->size();
  updateGeometry();
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
  if(m_model->get_current() != m_text_edit->toPlainText()) {
    m_model->set_current(m_text_edit->toPlainText());
  }
}

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*m_container_box);
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
