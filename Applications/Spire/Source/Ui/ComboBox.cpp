#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::Queries;
using namespace Spire;

ComboBox::QueryModel::Query ComboBox::QueryModel::Query::make_empty_query() {
  return {std::string(), SnapshotLimit::Unlimited()};
}

ComboBox::ComboBox(std::shared_ptr<QueryModel> model,QWidget* parent)
  : QWidget(parent),
    m_model(std::move(model)) {}
