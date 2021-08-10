#include "Spire/Ui/ScrollableListBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace Spire;

ScrollableListBox::ScrollableListBox(ListView* list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(list_view) {
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  m_scroll_box = new ScrollBox(m_list_view);
  layout->addWidget(m_scroll_box);
  setLayout(layout);
}
