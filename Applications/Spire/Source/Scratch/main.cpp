#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include "Spire/Spire/Resources.hpp"

#include <QScrollArea>
#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>

using namespace Spire;

namespace {
  class ContentSizedTextEdit : public QTextEdit {
    public:
      explicit ContentSizedTextEdit(QWidget* parent = nullptr)
          : QTextEdit(parent) {
        setLineWrapMode(QTextEdit::WidgetWidth);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        document()->setDocumentMargin(0);
        setFrameShape(QFrame::NoFrame);
        connect(
          this, &QTextEdit::textChanged, this, [=] { on_text_changed(); });
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setStyleSheet(R"(
          QTextEdit {
            padding: 7px 8px 7px 8px;
          })");
      }

      explicit ContentSizedTextEdit(
          const QString& text, QWidget* parent = nullptr)
          : ContentSizedTextEdit(parent) {
        setText(text);
      }

      void set_read_only(bool read_only) {
        if(read_only != isReadOnly()) {
          setReadOnly(read_only);
          updateGeometry();
        }
      }

      QSize sizeHint() const override {
        auto margins = contentsMargins();
        auto desired_size = QSize(get_longest_line(),
          document()->size().toSize().height());
        //qDebug() << "des. size: " << desired_size;
        //qDebug() << "cur. size: " << size();
        auto size = desired_size + QSize(//document()->size().toSize() + QSize(
          margins.left() + margins.right(), margins.top() + margins.bottom());
        //qDebug() << "doc size: " << size;
        if(!isReadOnly()) {
          size.rwidth() += cursorWidth();
        }
        return size;
      }

      QSize minimumSizeHint() const override {
        return QSize();
      }

    protected:
      //void resizeEvent(QResizeEvent* event) override {
      //  updateGeometry();
      //  QTextEdit::resizeEvent(event);
      //}

      void on_text_changed() {
        updateGeometry();
      }

      int get_longest_line() const {
        auto longest = 0;
        for(auto i = 0; i < document()->blockCount(); ++i) {
          auto block = document()->findBlockByNumber(i);
          if(block.isValid()) {
            longest = std::max(longest,
              fontMetrics().horizontalAdvance(block.text()));
          }
        }
        return longest;
      }

      int get_text_height() const {
        auto text_height = 0;
        for(auto i = 0; i < document()->blockCount(); ++i) {
          auto block = document()->findBlockByNumber(i);
          if(block.isValid()) {
            auto length = fontMetrics().horizontalAdvance(block.text());
            auto a = static_cast<int>(std::ceil(
              static_cast<double>(length) / static_cast<double>(width())));
            text_height += a * 13;
          }
        }
        return std::max(13, text_height);
      }
  };

  class ScrollArea : public QScrollArea {
    public:

      ScrollArea(ContentSizedTextEdit* edit, QWidget* parent = nullptr)
          : QScrollArea(parent),
            m_text_edit(edit) {
        setFocusProxy(edit);
        setWidget(edit);
        setWidgetResizable(true);
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        verticalScrollBar()->installEventFilter(this);
        connect(m_text_edit, &QTextEdit::textChanged, [=] {
          updateGeometry();
        });
        setStyleSheet(R"(
          QScrollArea {
            border: 1px solid #C8C8C8;
          }

          ScrollArea {
            background-color: red;
          }

          QScrollArea:hover {
            border: 1px solid #4B23A0;
          }

          QScrollArea:focus {
            border: 1px solid #4B23A0;
          })");
      }

      QSize sizeHint() const override {
        return m_text_edit->sizeHint() + QSize(2, 2);
      }

      QSize minimumSizeHint() const override {
        return QSize();
      }

    protected:
      void resizeEvent(QResizeEvent* event) override {
        if(verticalScrollBar()->isVisible()) {
          m_text_edit->setFixedWidth(width() -
            verticalScrollBar()->width() - 2);
        } else {
          m_text_edit->setFixedWidth(width() - 2);
        }
        updateGeometry();
        QScrollArea::resizeEvent(event);
      }

    private:
      ContentSizedTextEdit* m_text_edit;
  };
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto l = new QHBoxLayout(window);
  auto text_edit = new ContentSizedTextEdit(window);
  auto scroll_area = new ScrollArea(text_edit, window);
  l->addWidget(scroll_area);
  window->resize(scale(400, 400));
  window->show();
  application->exec();
}
