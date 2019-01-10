#include "spire/book_view/book_quote_table_view.hpp"
#include <QHeaderView>
#include "spire/book_view/book_quote_table_model.hpp"
#include "spire/book_view/book_view_table_delegate.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Spire;

BookQuoteTableView::BookQuoteTableView(
    std::unique_ptr<BookQuoteTableModel> model, QWidget* parent)
    : QTableView(parent),
      m_model(std::move(model)) {
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
  setItemDelegate(new BookViewTableDelegate(new CustomVariantItemDelegate(),
    this));
  setModel(m_model.get());
}

void BookQuoteTableView::set_properties(const BookViewProperties& properties) {
  m_model->set_properties(properties);
  auto metrics = QFontMetrics(properties.get_book_quote_font());
  auto row_height = metrics.height() + scale_height(2);
  verticalHeader()->setDefaultSectionSize(row_height);
  setShowGrid(properties.get_show_grid());
  update();
}

void BookQuoteTableView::resizeEvent(QResizeEvent* event) {
  setColumnWidth(0, width() / 3);
  setColumnWidth(1, width() / 3);
  setColumnWidth(2, width() / 3);
  QTableView::resizeEvent(event);
}
