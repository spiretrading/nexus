#include "spire/book_view/book_view_side_table_view.hpp"

using namespace Spire;

BookViewSideTableView::BookViewSideTableView(QWidget* parent)
    : QTableView(parent) {

}

void BookViewSideTableView::set_model(
    std::shared_ptr<BookQuoteTableModel> model) {

}

void BookViewSideTableView::set_properties(
    const BookViewProperties& properties) {

}
