#include "spire/book_view/book_view_level_properties_widget.hpp"

using namespace spire;

book_view_level_properties_widget::book_view_level_properties_widget(
    const book_view_properties& properties, QWidget* parent)
    : QWidget(parent) {}

void book_view_level_properties_widget::apply(
    book_view_properties& properties) const {
}
