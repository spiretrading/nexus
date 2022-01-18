#include "Spire/Ui/TextAreaBox.hpp"
#include <QAbstractTextDocumentLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QTextDocument>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto ELLIPSES_CHAR = 0x2026;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(LineHeight(1.20)).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor(Qt::black))).
      set(TextColor(QColor(Qt::black))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
    style.get(Hover() || Focus()).set(border_color(QColor(0x4B23A0)));
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
    style.get(Placeholder()).set(TextColor(QColor(0xA0A0A0)));
    style.get(Disabled() / Placeholder()).set(TextColor(QColor(0xC8C8C8)));
    return style;
  }
}

class TextAreaBox::ContentSizedTextEdit : public QTextEdit {
  public:
    ContentSizedTextEdit(std::shared_ptr<TextModel> current)
        : m_current(std::move(current)),
          m_is_synchronizing(false) {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setFrameShape(QFrame::NoFrame);
      setLineWrapMode(QTextEdit::WidgetWidth);
      setAcceptRichText(false);
      horizontalScrollBar()->blockSignals(true);
      verticalScrollBar()->blockSignals(true);
      document()->setDocumentMargin(0);
      connect(this, &QTextEdit::textChanged, this, [=] { on_text_changed(); });
      connect(document()->documentLayout(),
        &QAbstractTextDocumentLayout::documentSizeChanged, this,
        [=] (const auto& size) { updateGeometry(); });
      setText(m_current->get());
      m_current_connection = m_current->connect_update_signal(
        [=] (const auto& value) { on_current(value); });
    }

    const std::shared_ptr<TextModel>& get_current() const {
      return m_current;
    }

    QSize sizeHint() const override {
      return document()->documentLayout()->documentSize().toSize();
    }

  private:
    std::shared_ptr<TextModel> m_current;
    bool m_is_synchronizing;
    scoped_connection m_current_connection;

    template<typename F>
    void synchronize(F&& f) {
      if(m_is_synchronizing) {
        return;
      }
      m_is_synchronizing = true;
      try {
        std::forward<F>(f)();
      } catch(const std::exception&) {
        m_is_synchronizing = false;
        throw;
      }
      m_is_synchronizing = false;
    }

    void on_text_changed() {
      synchronize([&] {
        m_current->set(toPlainText());
      });
    }

    void on_current(const QString& current) {
      synchronize([&] {
        setText(current);
      });
    }
};

class TextAreaBox::ElidedLabel : public QWidget {
  public:
    ElidedLabel()
      : m_line_height(0) {}

    void set_text(const QString &text) {
      m_text = text;
      m_text_layout.setText(m_text);
      update();
    }

    void set_text_color(const QColor& color) {
      m_text_color = color;
      update();
    }

    void set_alignment(Qt::Alignment alignment) {
      auto option = m_text_layout.textOption();
      option.setAlignment(alignment);
      m_text_layout.setTextOption(option);
      update();
    }

    void set_line_height(int line_height) {
      m_line_height = line_height;
      update();
    }

  protected:
    void changeEvent(QEvent* event) override {
      if(event->type() == QEvent::FontChange) {
        if(m_line_height == 0) {
          m_line_height = fontMetrics().height();
        }
        m_text_layout.setFont(font());
      }
    }

    void paintEvent(QPaintEvent *event) override {
      auto painter = QPainter(this);
      painter.setPen(m_text_color);
      painter.setFont(font());
      auto metrics = painter.fontMetrics();
      auto y = m_line_height - metrics.height();
      m_text_layout.beginLayout();
      for(auto line = m_text_layout.createLine(); line.isValid();
          line = m_text_layout.createLine()) {
        line.setLineWidth(width());
        auto next_line_y = y + m_line_height;
        if (height() >= next_line_y + m_line_height) {
          line.draw(&painter, QPoint(0, y));
          y = next_line_y;
        } else {
          auto last_line = m_text.mid(line.textStart());
          auto elided_last_line =
            metrics.elidedText(last_line, Qt::ElideRight, width());
          painter.drawText(QPoint(0, y + metrics.ascent()), elided_last_line);
          line = m_text_layout.createLine();
          break;
        }
      }
      m_text_layout.endLayout();
    }

  private:
    QString m_text;
    QTextLayout m_text_layout;
    QColor m_text_color;
    int m_line_height;
};

TextAreaBox::StyleProperties::StyleProperties(std::function<void ()> commit)
  : m_styles(std::move(commit)) {}

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_border_sizes = {};
  m_padding = {};
  m_alignment = Qt::AlignLeft;
  m_font = {};
  m_size = none;
  m_color = {};
}

TextAreaBox::TextAreaBox(QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(), parent) {}

TextAreaBox::TextAreaBox(QString current, QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

TextAreaBox::TextAreaBox(std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_text_edit_styles([=] { commit_style(); }),
      m_placeholder_styles([=] { commit_placeholder_style(); }),
      m_submission(current->get()) {
  m_text_edit = new ContentSizedTextEdit(std::move(current));
  m_text_edit->setObjectName(QString("0x%1").arg(
    reinterpret_cast<std::intptr_t>(this)));
  m_text_edit->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  m_text_edit->installEventFilter(this);
  m_scroll_box = new ScrollBox(m_text_edit);
  m_scroll_box->set(
    ScrollBox::DisplayPolicy::NEVER, ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->setFocusProxy(m_text_edit);
  setFocusProxy(m_scroll_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  proxy_style(*this, *m_scroll_box);
  add_pseudo_element(*this, Placeholder());
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_placeholder_style_connection =
    connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_text_edit, &QTextEdit::cursorPositionChanged, this,
    &TextAreaBox::on_cursor_position);
}

const std::shared_ptr<TextModel>& TextAreaBox::get_current() const {
  return m_text_edit->get_current();
}

const QString& TextAreaBox::get_submission() const {
  return m_submission;
}

void TextAreaBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
}

bool TextAreaBox::is_read_only() const {
  return m_text_edit->isReadOnly();
}

void TextAreaBox::set_read_only(bool read_only) {
  if(m_text_edit->isReadOnly() == read_only) {
    return;
  }
  m_text_edit->setReadOnly(read_only);
  if(read_only) {
    m_scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
    match(*this, ReadOnly());
  } else {
    m_scroll_box->set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
    unmatch(*this, ReadOnly());
  }
}

connection TextAreaBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool TextAreaBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusOut) {
    m_submission = m_text_edit->get_current()->get();
    m_submit_signal(m_submission);
  }
  return QWidget::eventFilter(watched, event);
}

void TextAreaBox::commit_placeholder_style() {
  auto font = m_placeholder_styles.m_font;
  if(m_placeholder_styles.m_size) {
    font.setPixelSize(*m_placeholder_styles.m_size);
  }
}

void TextAreaBox::commit_style() {
  auto stylesheet = QString(
    R"(#0x%1 {
      background: transparent;
      border-width: 0px;)").arg(reinterpret_cast<std::intptr_t>(this));
  m_text_edit_styles.m_styles.write(stylesheet);
  auto font = m_text_edit_styles.m_font;
  if(m_text_edit_styles.m_size) {
    font.setPixelSize(*m_text_edit_styles.m_size);
  }
  m_text_edit->setFont(font);
  if(m_text_edit_styles.m_line_height) {
    m_scroll_box->get_vertical_scroll_bar().set_line_size(
      m_text_edit->font().pixelSize() * *m_text_edit_styles.m_line_height);
  }
  if(stylesheet != m_text_edit->styleSheet()) {
    m_text_edit->setStyleSheet(stylesheet);
  }
}

bool TextAreaBox::is_placeholder_shown() const {
  return !is_read_only() && m_text_edit->get_current()->get().isEmpty() &&
    !m_placeholder_text.isEmpty();
}

QSize TextAreaBox::get_padding_size() const {
  auto& padding = m_text_edit_styles.m_padding;
  return {padding.left() + padding.right(), padding.top() + padding.bottom()};
}

void TextAreaBox::on_cursor_position() {
  if(m_scroll_box->get_vertical_scroll_bar().isVisible() &&
      !m_text_edit->visibleRegion().boundingRect().contains(
        m_text_edit->cursorRect())) {
    auto top = m_text_edit->visibleRegion().boundingRect().top();
    auto bottom = m_text_edit->visibleRegion().boundingRect().bottom();
    if(m_text_edit->cursorRect().top() <= top) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().top());
    } else if(m_text_edit->cursorRect().bottom() >= bottom) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().bottom() -
        m_text_edit->visibleRegion().boundingRect().height() +
        get_padding_size().height());
    }
  }
}

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  m_text_edit_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const BorderTopSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_border_sizes.setTop(size);
          });
        },
        [&] (const BorderRightSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_border_sizes.setRight(size);
          });
        },
        [&] (const BorderBottomSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_border_sizes.setBottom(size);
          });
        },
        [&] (const BorderLeftSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_border_sizes.setLeft(size);
          });
        },
        [&] (const PaddingTop& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_padding.setTop(size);
          });
        },
        [&] (const PaddingRight& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_padding.setRight(size);
          });
        },
        [&] (const PaddingBottom& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_padding.setBottom(size);
          });
        },
        [&] (const PaddingLeft& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_padding.setLeft(size);
          });
        },
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
            m_placeholder_styles.m_color = color;
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
        },
        [&] (const LineHeight& height) {
          stylist.evaluate(height, [=] (auto height) {
            m_placeholder_styles.m_line_height = height;
          });
        });
    }
  });
}
