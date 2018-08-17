#include "spire/book_view/book_view_side_table_view.hpp"
#include <QHeaderView>
#include "spire/book_view/book_quote_table_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Spire;

BookViewSideTableView::BookViewSideTableView(QWidget* parent)
    : QTableView(parent) {
  horizontalHeader()->hide();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  verticalHeader()->hide();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionMode(QAbstractItemView::NoSelection);
  setFocusPolicy(Qt::NoFocus);
  setShowGrid(false);
}

void BookViewSideTableView::set_model(
    std::unique_ptr<BookQuoteTableModel> model) {
  m_model = std::move(model);
  auto filter = new CustomVariantSortFilterProxyModel(this);
  filter->setSourceModel(m_model.get());
  setModel(filter);
}

void BookViewSideTableView::set_properties(
    const BookViewProperties& properties) {
  m_model->set_properties(properties);
  QFontMetrics metrics(properties.get_book_quote_font());
  auto row_height = metrics.height() + scale_height(2);
  verticalHeader()->setDefaultSectionSize(row_height);
}
