#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

namespace {
  class TextAreaBox : public QWidget {
    public:

      TextAreaBox() {
        auto layout = new QHBoxLayout();
        m_text_edit = new QTextEdit(this);
        layout->addWidget(m_text_edit);
        setLayout(layout);
      }

    private:
      QTextEdit* m_text_edit;
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
  auto box = new TextAreaBox();
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
