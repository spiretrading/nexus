#include "Spire/BookView/BookQuoteTableView.hpp"
#include <QHeaderView>
#include "Spire/BookView/BookQuoteTableModel.hpp"
#include "Spire/BookView/BookViewTableDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Spire;

BookQuoteTableView::BookQuoteTableView(
    BookQuoteTableModel* model, QWidget* parent)
    : QTableView(parent),
      m_model(model) {
  m_model->setParent(this);
  setStyleSheet(QString(R"(
    border: none;
    gridline-color: #C8C8C8;)"));
  horizontalHeader()->hide();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  verticalHeader()->hide();
  verticalHeader()->setMinimumSectionSize(0);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setSelectionMode(QAbstractItemView::NoSelection);
  setFocusPolicy(Qt::NoFocus);
  setItemDelegate(new BookViewTableDelegate(new CustomVariantItemDelegate(),
    this));
  setModel(m_model);
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

QStyleOptionViewItem BookQuoteTableView::viewOptions() const {
  auto options = QTableView::viewOptions();
  options.locale.setNumberOptions(QLocale::DefaultNumberOptions);
  return options;
}
