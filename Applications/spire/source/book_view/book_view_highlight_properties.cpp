#include "spire/book_view/book_view_highlight_properties_widget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace spire;

book_view_highlight_properties_widget::book_view_highlight_properties_widget(
    const book_view_properties& properties, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
}

void book_view_highlight_properties_widget::apply(
    book_view_properties& properties) const {
}
