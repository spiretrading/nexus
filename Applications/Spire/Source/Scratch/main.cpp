#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include "Spire/Spire/Resources.hpp"

#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>

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
      void resizeEvent(QResizeEvent* event) override {
        document()->setTextWidth(-1);
        updateGeometry();
        QTextEdit::resizeEvent(event);
      }

      void on_text_changed() {
        document()->setTextWidth(-1);
        updateGeometry();
      }
  };
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = new QWidget();
  auto vertical_layout = new QVBoxLayout();
  vertical_layout->addSpacerItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto box = new ContentSizedTextEdit();
  vertical_layout->addWidget(box);
  vertical_layout->addSpacerItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  auto horizontal_layout = new QHBoxLayout();
  horizontal_layout->addSpacerItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  horizontal_layout->addLayout(vertical_layout);
  horizontal_layout->addSpacerItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
  window->setLayout(horizontal_layout);
  window->show();
  application->exec();
}
