#include <memory>
#include <QApplication>
#include <QHeaderView>
#include <QLayout>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/ui/item_padding_delegate.hpp"
#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/spire/resources.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Nexus;
using namespace spire;

//! Displays a table with horizontal header and loading widget.
class time_and_sales_table_view : public QScrollArea {
  public:

    //! Constructs a time_and_sales_table_view
    /*
      \param model The model to get the table data from.
      \param parent The parent to this widget.
    */
    time_and_sales_table_view(QAbstractItemModel* model,
      QWidget* parent = nullptr);

    //! Sets the model to get the table data from.
    /*
      \param model The model.
    */
    void set_model(QAbstractItemModel* model);

  protected:
    void resizeEvent(QResizeEvent* event) override;

  private:
    QHeaderView* m_header;
    QWidget* m_header_padding;
    QTableView* m_table;

    void on_rows_about_to_be_inserted();
};

time_and_sales_table_view::time_and_sales_table_view(
    QAbstractItemModel* model,QWidget* parent)
    : QScrollArea(parent) {
  m_header = new QHeaderView(Qt::Horizontal, this);
  m_header->setStretchLastSection(true);
  auto main_widget = new QWidget(this);
  auto layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_header_padding = new QWidget(this);
  m_header_padding->setFixedHeight(m_header->height());
  layout->addWidget(m_header_padding);
  m_table = new QTableView(this);
  m_table->setMinimumWidth(750);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->hide();
  m_table->verticalHeader()->hide();
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setItemDelegate(new item_padding_delegate(scale_width(5),
    new custom_variant_item_delegate(), this));
  connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
    &time_and_sales_table_view::on_rows_about_to_be_inserted);
  layout->addWidget(m_table);
  set_model(model);
  setWidgetResizable(true);
  setWidget(main_widget);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void time_and_sales_table_view::set_model(QAbstractItemModel* model) {
  m_header->setModel(model);
  m_table->setModel(model);
}

void time_and_sales_table_view::resizeEvent(QResizeEvent* event) {
  m_header->setFixedWidth(width());
}

void time_and_sales_table_view::on_rows_about_to_be_inserted() {
  if(m_table->model()->rowCount() > 0) {
    widget()->resize(width(),
      (m_table->model()->rowCount() + 1) * m_table->rowHeight(0));
  }
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto periodic_model = std::make_shared<periodic_time_and_sales_model>(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()));
  periodic_model->set_price(Money(Quantity(100)));
  periodic_model->set_price_range(
    time_and_sales_properties::price_range::AT_ASK);
  periodic_model->set_period(boost::posix_time::milliseconds(500));
  auto table_model = new time_and_sales_window_model(periodic_model,
      time_and_sales_properties());
  auto filter = new custom_variant_sort_filter_proxy_model();
  filter->setSourceModel(table_model);
  auto table = new time_and_sales_table_view(filter);
  table->show();
  application->exec();
}
