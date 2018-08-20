#include "spire/book_view/book_view_side_table_view.hpp"
#include <QHeaderView>
#include "spire/book_view/book_quote_table_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/item_padding_delegate.hpp"

using namespace Spire;

BookViewSideTableView::BookViewSideTableView(QWidget* parent)
    : QTableView(parent) {
  setStyleSheet(QString(R"(
    border: none;
    gridline-color: #C8C8C8;)"));
  horizontalHeader()->hide();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  verticalHeader()->hide();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setSelectionMode(QAbstractItemView::NoSelection);
  setFocusPolicy(Qt::NoFocus);
  setItemDelegate(new ItemPaddingDelegate(scale_width(4),
    new CustomVariantItemDelegate(), this));
}

void BookViewSideTableView::set_model(
    std::unique_ptr<BookQuoteTableModel> model) {
  m_model = std::move(model);
  setModel(m_model.get());
}

void BookViewSideTableView::set_properties(
    const BookViewProperties& properties) {
  m_model->set_properties(properties);
  QFontMetrics metrics(properties.get_book_quote_font());
  auto row_height = metrics.height() + scale_height(2);
  verticalHeader()->setDefaultSectionSize(row_height);
  setShowGrid(properties.get_show_grid());
  update();
}

void BookViewSideTableView::resizeEvent(QResizeEvent* event) {
  setColumnWidth(0, width() / 3);
  setColumnWidth(1, width() / 3);
  setColumnWidth(2, width() / 3);
}
