//#include <QApplication>
//#include "Spire/Spire/Resources.hpp"
//
//#include "Spire/Ui/TextAreaBox.hpp"
//#include <QHBoxLayout>
//
//using namespace Spire;
//
//int main(int argc, char** argv) {
//  auto application = new QApplication(argc, argv);
//  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
//  application->setApplicationName(QObject::tr("Scratch"));
//  initialize_resources();
//  auto w = new QWidget();
//  auto l = new QHBoxLayout(w);
//  auto t = new TextAreaBox();
//  l->addWidget(t);
//  w->resize(600, 400);
//  w->show();
//  application->exec();
//}
#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include <QTextDocument>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTextEdit>
#include <QScrollArea>

using namespace Spire;
using namespace Spire::Styles;

class TAB : public QWidget {
  public:

    TAB(QWidget* parent)
        : QWidget(parent),
          m_longest_line_length(0),
          m_longest_line_block(0) {
      setStyleSheet("QTextEdit { background-color: aqua; }");
      setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      m_text_edit = new QTextEdit(this);
      //layout->addWidget(m_text_edit);
      m_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      m_text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      m_text_edit->setLineWrapMode(QTextEdit::FixedPixelWidth);
      m_text_edit->document()->setDocumentMargin(0);
      m_text_edit->setFrameShape(QFrame::NoFrame);
      auto scroll_area = new QScrollArea(this);
      scroll_area->setWidget(m_text_edit);
      scroll_area->viewport()->setStyleSheet("QWidget { background-color: red; }");
      //scroll_area->setWidgetResizable(true);
      layout->addWidget(m_text_edit);
      connect(m_text_edit->document(), &QTextDocument::contentsChange, this,
        &TAB::on_contents_changed);
      connect(m_text_edit, &QTextEdit::textChanged, this,
        &TAB::on_text_changed);
      connect(m_text_edit->document()->documentLayout(),
        &QAbstractTextDocumentLayout::documentSizeChanged, [=] (auto size) {
          qDebug() << "doc size changed: " << size;
        });
      connect(m_text_edit->document()->documentLayout(),
        &QAbstractTextDocumentLayout::pageCountChanged, [=] (auto new_pages) {
          qDebug() << "page count changed: " << new_pages;
        });
      connect(m_text_edit->document()->documentLayout(),
        &QAbstractTextDocumentLayout::update, [=] (auto rect) {
          //qDebug() << "doc layout update: " << rect;
        });
      connect(m_text_edit->document()->documentLayout(),
        &QAbstractTextDocumentLayout::updateBlock, [=] (auto block) {
          //qDebug() << "doc layout block updated: " << block.blockNumber();
        });
      //connect(m_text_edit, &QTextEdit::cursorPositionChanged,
      //  [=] { qDebug() << "text edit cursor pos changed"; });
      //connect(m_text_edit->document(), &QTextDocument::blockCountChanged,
      //  [=] (auto count) { qDebug() << "block count: " << count; });
      //connect(m_text_edit->document(), &QTextDocument::cursorPositionChanged,
      //  [=] (auto cursor) { qDebug() << "doc cursor pos changed"; });
      //connect(m_text_edit->document(), &QTextDocument::modificationChanged,
      //  [=] (auto changed) { qDebug() << "mod. changed: " << changed; });
      //m_text_edit->verticalScrollBar()->installEventFilter(this);
    }

    QSize sizeHint() const override {
      auto sb_width = [&] {
        if(m_text_edit->verticalScrollBar()->isVisible()) {
          //qDebug() << "is visible";
          //return m_text_edit->verticalScrollBar()->width();
        }
        return 0;
      }();
      return m_text_edit->document()->size().toSize() + QSize(sb_width, 0);
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      //if(event->type() == QEvent::Show) {
      ////  m_text_edit->viewport()->setContentsMargins(0, 0,
      ////    m_text_edit->verticalScrollBar()->width(), 0);
      //  //m_text_edit->document()->setPageSize(
      //  //  {m_text_edit->document()->pageSize().width() +
      //  //  m_text_edit->verticalScrollBar()->width(), -1});
      //  //m_text_edit->document()->adjustSize();
      //  qDebug() << "show* *************************************************";
      //  auto width = m_text_edit->document()->pageSize().width() +
      //    m_text_edit->verticalScrollBar()->width();
      //  m_text_edit->document()->setPageSize({qreal(width), -1});
      //  m_text_edit->document()->adjustSize();
      //  m_text_edit->setLineWrapColumnOrWidth(width);
      //  updateGeometry();
      //} else if(event->type() == QEvent::Hide) {
      ////  m_text_edit->viewport()->setContentsMargins({});
      //  //m_text_edit->document()->setPageSize({
      //  //  m_text_edit->document()->pageSize().width() -
      //  //  m_text_edit->verticalScrollBar()->width(), -1});
      //  //m_text_edit->document()->adjustSize();

      //  auto width = m_text_edit->document()->pageSize().width() -
      //    m_text_edit->verticalScrollBar()->width();
      //  m_text_edit->document()->setPageSize({qreal(width), -1});
      //  m_text_edit->document()->adjustSize();
      //  m_text_edit->setLineWrapColumnOrWidth(width);
      //  updateGeometry();
      //}
      return QWidget::eventFilter(watched, event);
    }

  private:
    QTextEdit* m_text_edit;
    int m_longest_line_length;
    int m_longest_line_block;

    int get_text_length(const QString& text) {
      return m_text_edit->fontMetrics().horizontalAdvance(text) +
        m_text_edit->cursorWidth();
    }

    void on_contents_changed(int position, int removed, int added) {
      qDebug() << "contents changed";
      if(m_text_edit->document()->lineCount() == 5) {
        qDebug() << "five";
      }
      //qDebug() << "pre doc size: " << m_text_edit->document()->size();
      //qDebug() << "pre page size: " << m_text_edit->document()->pageSize();
      auto block = m_text_edit->document()->findBlock(position);
      if(block.isValid()) {
        auto line_length = get_text_length(block.text());
        if(line_length > m_longest_line_length) {
          m_longest_line_length = line_length;
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
      }
      //update_page_size();
      qDebug() << "longest: " << m_longest_line_length;
      //qDebug() << "doc size: " << m_text_edit->document()->size();
      //m_text_edit->document()->setTextWidth(m_longest_line_length);
      //m_text_edit->document()->setPageSize({
      //  qreal(m_longest_line_block + m_text_edit->cursorWidth()), -1});
      //m_text_edit->document()->adjustSize();
      //m_text_edit->setLineWrapColumnOrWidth(m_longest_line_length + m_text_edit->cursorWidth());
      m_text_edit->document()->setTextWidth(m_longest_line_length + m_text_edit->cursorWidth());
      //m_text_edit->document()->adjustSize();
      //qDebug() << "post doc size: " << m_text_edit->document()->size();
      //qDebug() << "post page size: " << m_text_edit->document()->pageSize();
      //qDebug() << "te size: " << m_text_edit->size();
      //qDebug() << "this size: " << size();
      updateGeometry();
    }

    void TAB::on_text_changed() {
      qDebug() << "text changed";
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  auto layout = new QHBoxLayout(w);
  auto t = new TAB(w);
  layout->addWidget(t);
  w->resize(400, 400);
  w->show();
  application->exec();
}
