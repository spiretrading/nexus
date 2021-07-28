#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include "Spire/Spire/Resources.hpp"

#include <QScrollArea>
#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>
#include <QPushButton>

using namespace Spire;

namespace {
  class ContentSizedTextEdit : public QTextEdit {
    public:
      explicit ContentSizedTextEdit(QWidget* parent = nullptr)
          : QTextEdit(parent) {
        setLineWrapMode(QTextEdit::NoWrap);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        document()->setDocumentMargin(0);
        setFrameShape(QFrame::NoFrame);
        connect(
          this, &QTextEdit::textChanged, this, [=] { on_text_changed(); });
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
        auto size = document()->size().toSize() + QSize(
          margins.left() + margins.right(), margins.top() + margins.bottom());
        if(!isReadOnly()) {
          size.rwidth() += cursorWidth();
        }
        return size;
      }

      QSize minimumSizeHint() const override {
        return QSize();
      }

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override {
        if(event->type() == QEvent::Resize) {
          setFixedWidth(static_cast<QResizeEvent*>(event)->size().width() - 4);
        }
        return QWidget::eventFilter(watched, event);
      }

      void resizeEvent(QResizeEvent* event) override {
        update_text_wrap();
        QTextEdit::resizeEvent(event);
      }

      void on_text_changed() {
        update_text_wrap();
      }

      void update_text_wrap() {
        if(size().width() < get_longest_line()) {
          setLineWrapMode(QTextEdit::WidgetWidth);
        } else {
          document()->setTextWidth(-1);
          setLineWrapMode(QTextEdit::NoWrap);
        }
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
  };
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  //auto vertical_layout = new QVBoxLayout();
  //vertical_layout->addSpacerItem(
  //  new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  //auto box = new ContentSizedTextEdit();
  //vertical_layout->addWidget(box);
  //vertical_layout->addSpacerItem(
  //  new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  //auto horizontal_layout = new QHBoxLayout();
  //horizontal_layout->addSpacerItem(
  //  new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  //horizontal_layout->addLayout(vertical_layout);
  //horizontal_layout->addSpacerItem(
  //  new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  //window->setLayout(horizontal_layout);
  auto l = new QHBoxLayout(window);
  auto text_edit = new ContentSizedTextEdit(window);
  auto inc = new QPushButton("+", window);
  l->addWidget(inc);
  QObject::connect(inc, &QPushButton::clicked, [=] {
    text_edit->setFixedWidth(text_edit->width() + 10);
  });
  auto dec = new QPushButton("-", window);
  l->addWidget(dec);
  QObject::connect(dec, &QPushButton::clicked, [=] {
    text_edit->setFixedWidth(text_edit->width() - 10);
  });
  //auto scroll_area = new QScrollArea(window);
  //scroll_area->setWidget(text_edit);
  //scroll_area->setWidgetResizable(true);
  //scroll_area->viewport()->installEventFilter(text_edit);
  //l->addWidget(scroll_area);
  l->addWidget(text_edit);
  window->resize(scale(400, 400));
  window->show();
  application->exec();
}
