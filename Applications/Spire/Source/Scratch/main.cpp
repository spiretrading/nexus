#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/TagBox.hpp"

using namespace Spire;

struct TagTester : public QWidget {
  TagBox* m_tag_box;
  std::shared_ptr<ListModel<int>> m_tags;

  explicit TagTester(TagBox& tag_box, std::shared_ptr<ListModel<int>> tags,
      QWidget* parent = nullptr)
      : QWidget(parent),
        m_tag_box(&tag_box),
        m_tags(std::move(tags)) {
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_tag_box, 1, Qt::AlignCenter);
    auto button = new QPushButton("Test", this);
    layout->addWidget(button, 0, Qt::AlignBottom);
    connect(button, &QPushButton::clicked, this, &TagTester::onButtonClicked);
  }

  void onButtonClicked() {
    auto value = m_tags->get(0);
    m_tags->set(0, value + 1);
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto current = std::make_shared<LocalTextModel>();
  auto tags = std::make_shared<ArrayListModel<int>>();
  tags->push(1);
  auto tag_box = new TagBox(tags, current);
  auto tester = TagTester(*tag_box, tags);
  tester.show();
  application.exec();
}
