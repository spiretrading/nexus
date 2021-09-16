#include "Spire/Ui/TagBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ListView.hpp"

using namespace Spire;

TagBox::TagBox(std::shared_ptr<ListModel> model, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_list_view = new ListView(std::move(model), this);
  auto input_box = make_input_box(m_list_view, this);
  layout->addWidget(input_box);
}

const std::shared_ptr<ListModel>& TagBox::get_model() const {
  return m_list_view->get_list_model();
}
