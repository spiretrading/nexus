#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::Queries;
using namespace Spire;

ComboBox::QueryModel::UnaryPredicate::UnaryPredicate(std::any value)
	: m_value(displayTextAny(value)) {}

bool ComboBox::QueryModel::UnaryPredicate::operator()(
		const std::any& value) const {
	return m_value == displayTextAny(value);
}

bool ComboBox::QueryModel::BinaryPredicate::operator()(
		const std::any& first, const std::any& second) {
	return displayTextAny(first) == displayTextAny(second);
}

ComboBox::QueryModel::Query ComboBox::QueryModel::Query::make_empty_query() {
	return {std::string(), SnapshotLimit::Unlimited()};
}

ComboBox::ComboBox(std::shared_ptr<QueryModel> model, QWidget* parent)
	: QWidget(parent),
		m_model(std::move(model)) {}
