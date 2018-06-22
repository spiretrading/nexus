#include <memory>
#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWheelEvent>
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

    //! Sets the time_and_sales_properties of the table.
    /*
      \param properties The properties the table will be updated to have.
    */
    void set_properties(const time_and_sales_properties& properties);

  protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private:
    QHeaderView* m_header;
    QWidget* m_header_padding;
    QTableView* m_table;
    QTimer m_h_scroll_bar_timer;
    QTimer m_v_scroll_bar_timer;

    void fade_out_horizontal_scroll_bar();
    void fade_out_vertical_scroll_bar();
    bool within_horizontal_scroll_bar(const QPoint& pos);
    bool within_vertical_scroll_bar(const QPoint& pos);
    int true_height();
    void on_header_resize(int index, int old_size, int new_size);
    void on_header_move(int logical_index, int old_index, int new_index);
    void on_horizontal_slider_value_changed(int new_value);
    void on_rows_about_to_be_inserted();
};

namespace {
  auto MINIMUM_TABLE_WIDTH = 750;
  auto SCROLL_BAR_FADE_TIME_MS = 500;
}

time_and_sales_table_view::time_and_sales_table_view(
    QAbstractItemModel* model,QWidget* parent)
    : QScrollArea(parent) {
  setStyleSheet(QString(R"(
    QWidget {
      background-color: #FFFFFF;
      border: none;
    }

    QScrollBar::horizontal {
      height: %1px;
    }

    QScrollBar::vertical {
      width: %2px;
    }

    QScrollBar::handle {
      background-color: #C8C8C8;
    }

    QScrollBar::handle:horizontal {
      min-width: %3px;
    }

    QScrollBar::handle:vertical {
      min-height: %4px;
    }

    QScrollBar::add-line, QScrollBar::sub-line,
    QScrollBar::add-page, QScrollBar::sub-page {
      background: none;
      border: none;
      height: 0px;
      width: 0px;
    })").arg(scale_height(12)).arg(scale_height(12))
        .arg(scale_width(30)).arg(scale_height(30)));
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);
  horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &time_and_sales_table_view::on_horizontal_slider_value_changed);
  m_header = new QHeaderView(Qt::Horizontal, this);
  m_header->setMinimumSectionSize(scale_width(35));
  m_header->setStretchLastSection(true);
  m_header->setSectionsClickable(false);
  m_header->setSectionsMovable(true);
  m_header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)));
  connect(m_header, &QHeaderView::sectionResized, this,
    &time_and_sales_table_view::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &time_and_sales_table_view::on_header_move);
  auto main_widget = new QWidget(this);
  main_widget->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  auto layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_header_padding = new QWidget(this); 
  m_header_padding->setFixedHeight(m_header->height());
  layout->addWidget(m_header_padding);
  m_table = new QTableView(this);
  m_table->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  m_table->resize(width(), 0);
  m_table->setFocusPolicy(Qt::NoFocus);
  m_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->hide();
  m_table->verticalHeader()->hide();
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setStyleSheet(R"(
      border: none;
      gridline-color: #C8C8C8;)");
  m_table->setItemDelegate(new item_padding_delegate(scale_width(5),
    new custom_variant_item_delegate(), this));
  connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
    &time_and_sales_table_view::on_rows_about_to_be_inserted);
  layout->addWidget(m_table);
  m_h_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_h_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_table_view::fade_out_horizontal_scroll_bar);
  m_v_scroll_bar_timer.setInterval(SCROLL_BAR_FADE_TIME_MS);
  connect(&m_v_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_table_view::fade_out_vertical_scroll_bar);
  set_model(model);
  setWidget(main_widget);
}

void time_and_sales_table_view::set_model(QAbstractItemModel* model) {
  m_header->setModel(model);
  m_table->setModel(model);
}

void time_and_sales_table_view::set_properties(
    const time_and_sales_properties& properties) {
  m_table->setShowGrid(properties.m_show_grid);
  QFontMetrics metrics(properties.m_font);
  auto row_height = metrics.height() + scale_height(2);
  m_table->verticalHeader()->setDefaultSectionSize(row_height);
  auto header_font = m_table->verticalHeader()->font();
  if(properties.m_font.pointSize() >= 11) {
    header_font.setPointSizeF(properties.m_font.pointSize() * 0.8);
  } else {
    header_font.setPointSize(9);
  }
  m_header->setFont(header_font);
  QFontMetrics header_metrics(header_font);
  m_header->setFixedHeight(header_metrics.height() * 1.8);
  m_header_padding->setFixedHeight(m_header->height());
  //
  // is this still required?
  //
  viewport()->update();
}

bool time_and_sales_table_view::event(QEvent* event) {
  if(event->type() == QEvent::HoverMove) {
    auto e = static_cast<QHoverEvent*>(event);
    if(within_horizontal_scroll_bar(e->pos())) {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else if(within_vertical_scroll_bar(e->pos())) {
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      m_v_scroll_bar_timer.start();
    } else {
      if(!m_v_scroll_bar_timer.isActive() &&
          verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        fade_out_vertical_scroll_bar();
      }
      if(!m_h_scroll_bar_timer.isActive() &&
          horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        fade_out_horizontal_scroll_bar();
      }
    }
  }
  return QScrollArea::event(event);
}

void time_and_sales_table_view::resizeEvent(QResizeEvent* event) {
  m_header->setFixedWidth(m_table->width());
}

void time_and_sales_table_view::wheelEvent(QWheelEvent* event) {
  if(event->modifiers() & Qt::ShiftModifier) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
      (event->delta() / 2));
    m_h_scroll_bar_timer.start();
  } else if(!event->modifiers().testFlag(Qt::ShiftModifier)) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    verticalScrollBar()->setValue(verticalScrollBar()->value() -
      (event->delta() / 2));
    m_v_scroll_bar_timer.start();
  }
}

void time_and_sales_table_view::fade_out_horizontal_scroll_bar() {
  m_h_scroll_bar_timer.stop();
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_table_view::fade_out_vertical_scroll_bar() {
  m_v_scroll_bar_timer.stop();
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

int time_and_sales_table_view::true_height() {
  return visibleRegion().boundingRect().height() +
    m_table->visibleRegion().boundingRect().height() +
    m_header->height();
}

bool time_and_sales_table_view::within_horizontal_scroll_bar(
    const QPoint& pos) {
  return pos.y() > true_height() -  horizontalScrollBar()->height();
}

bool time_and_sales_table_view::within_vertical_scroll_bar(const QPoint& pos) {
  return pos.x() > width() - verticalScrollBar()->width();
}

void time_and_sales_table_view::on_header_resize(int index, int old_size,
    int new_size) {
  m_table->horizontalHeader()->resizeSection(index,
    m_header->sectionSize(index));
}

void time_and_sales_table_view::on_header_move(int logical_index,
    int old_index, int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void time_and_sales_table_view::on_horizontal_slider_value_changed(
    int new_value) {
  if(new_value != 0) {
    m_header->move(widget()->pos().x(), m_header->pos().y());
  } else {
    m_header->move(0, m_header->pos().y());
  }
}

void time_and_sales_table_view::on_rows_about_to_be_inserted() {
  if(m_table->model()->rowCount() > 0) {
    widget()->resize(width(),
      (m_table->model()->rowCount() + 1) * m_table->rowHeight(0) +
      m_header->height());
    if(verticalScrollBar()->value() != 0) {
      verticalScrollBar()->setValue(verticalScrollBar()->value() +
        m_table->rowHeight(0));
    }
  }
  setWindowTitle(QString("%1").arg(m_table->model()->rowCount()));
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto periodic_model = std::make_shared<periodic_time_and_sales_model>(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()));
  periodic_model->set_price(Money(Quantity(44)));
  periodic_model->set_price_range(
    time_and_sales_properties::price_range::AT_ASK);
  periodic_model->set_period(boost::posix_time::milliseconds(1000));
  auto table_model = new time_and_sales_window_model(periodic_model,
      time_and_sales_properties());
  auto filter = new custom_variant_sort_filter_proxy_model();
  filter->setSourceModel(table_model);
  auto table = new time_and_sales_table_view(filter);
  table->set_properties(time_and_sales_properties());
  table->show();
  application->exec();
}
