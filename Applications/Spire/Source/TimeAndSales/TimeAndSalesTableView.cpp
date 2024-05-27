#include "Spire/TimeAndSales/TimeAndSalesTableView.hpp"
#include <QMovie>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/SaleConditionBox.hpp"
#include "Spire/Ui/SaleConditionListItem.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using PullIndicator = StateSelector<void, struct PullIndicatorSelectorTag>;
  using PullDelayed = StateSelector<void, struct PullDelayedSelectorTag>;
  using TimeAndSalesBody = StateSelector<void, struct TimeAndSalesBodyTag>;
  using NoAdditionalEntries =
    StateSelector<void, struct NoAdditionalEntriesSelectorTag>;

  struct HeaderItemProperties {
    bool m_is_visible;
    Qt::Alignment m_text_align;
  };

  struct Status {
    bool m_is_loading;
    bool m_has_additional_entries;
    int m_last_scroll_y;
  };

  auto TABLE_VIEW_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    auto body_selector = Any() > is_a<TableBody>();
    style.get(Any() >> is_a<TextBox>()).
      set(Font(font));
    //style.get(Any() > is_a<TableHeader>() >> is_a<TextBox>()).
    //  set(PaddingLeft(scale_width(4))).
    //  set(PaddingTop(scale_height(5))).
    //  set(PaddingBottom(scale_height(4)));
    style.get(body_selector).
      set(grid_color(Qt::transparent)).
      set(horizontal_padding(0)).
      set(vertical_padding(0)).
      set(HorizontalSpacing(0)).
      set(VerticalSpacing(0));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent)).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > CurrentRow()).set(BackgroundColor(Qt::transparent));
    style.get(body_selector > CurrentColumn()).set(BackgroundColor(Qt::transparent));
    //style.get(Any() > TableHeaderItem::Label()).
    //  set(Font(font));
    //style.get(body_selector >> is_a<TextBox>()).
    //  //set(TextStyle(font, QColor(Qt::black))).
    //  set(border_size(0)).
    //  set(horizontal_padding(scale_width(2))).
    //  set(vertical_padding(scale_height(1.5)));
    style.get(NoAdditionalEntries() > is_a<TableBody>()).
      set(PaddingBottom(scale_height(44)));
    style.get(PullDelayed() > is_a<TableBody>()).
      set(PaddingBottom(0));
    style.get(Any() >> PullIndicator()).
      set(Visibility::NONE).
      set(BodyAlign(Qt::AlignHCenter)).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(20))).
      set(PaddingTop(scale_height(8)));
    style.get(PullDelayed() >> PullIndicator()).
      set(Visibility::VISIBLE);
    return style;
  }

  auto TABLE_HADER_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
    return style;
  }

  auto TABLE_CELL_STYLE(StyleSheet style) {
    style.get(Any()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(1.5)));
    return style;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto push = [&] (const QString& name, const QString& short_name) {
      model->push({name, short_name, TableHeaderItem::Order::UNORDERED,
        TableFilter::Filter::NONE});
      };
    push(QObject::tr("Time"), QObject::tr("Time"));
    push(QObject::tr("Price"), QObject::tr("Px"));
    push(QObject::tr("Size"), QObject::tr("Sz"));
    push(QObject::tr("Market"), QObject::tr("Mkt"));
    push(QObject::tr("Condition"), QObject::tr("Cond"));
    push("", "");
    return model;
  }

  auto make_header_item_properties() {
    auto properties = std::vector<HeaderItemProperties>();
    properties.emplace_back(false, Qt::AlignLeft);
    properties.emplace_back(true, Qt::AlignRight);
    properties.emplace_back(true, Qt::AlignRight);
    properties.emplace_back(true, Qt::AlignLeft);
    properties.emplace_back(false, Qt::AlignLeft);
    return properties;
  }

  auto make_pull_indicator() {
    auto spinner = new QMovie(":/Icons/spinner.gif", QByteArray());
    spinner->setScaledSize(scale(16, 16));
    spinner->start();
    auto spinner_widget = new QLabel();
    spinner_widget->setMovie(spinner);
    auto box = new Box(spinner_widget);
    //set_style(*box, PULL_INDICATOR_STYLE());
    return box;
  }

  auto make_time_cell(const ptime& time) {
    auto ts = to_text(time);
    ts = ts.left(ts.lastIndexOf('.'));
    auto time_box = make_time_box(duration_from_string(ts.toStdString()));
    time_box->set_read_only(true);
    update_style(*time_box, [] (auto& style) {
      //style.get(ReadOnly() >> is_a<TextBox>()).
      //  set(border_size(0)).
      //  set(horizontal_padding(0)).
      //  set(vertical_padding(0));
      style.get(Any() > is_a<DecimalBox>()).
        set(TrailingZeros(0));
      style.get(Any()).
        set(PaddingLeft(scale_width(2))).
        set(PaddingRight(0));
      });
    //auto& box = *static_cast<Box*>(time_box->layout()->itemAt(0)->widget());
    //auto box_body = box.get_body();
    //for(auto i = 0; i < box_body->layout()->count(); ++i) {
    //  auto field = box_body->layout()->itemAt(i)->widget();
    //  field->setMinimumWidth(0);
    //  field->setMaximumWidth(QWIDGETSIZE_MAX);
    //  static_cast<QHBoxLayout*>(box_body->layout())->setStretchFactor(field, 0);
    //}
    time_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return time_box;
  }

  template<typename B, typename T =
    std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar>
  auto make_decimal_cell(const T& value) {
    auto cell = new B();
    cell->get_current()->set(value);
    cell->set_read_only(true);
    update_style(*cell, [] (auto& style) {
      style.get(Any() > is_a<TextBox>()).
        set(TextAlign(Qt::AlignRight | Qt::AlignVCenter));
      });
    return cell;
  }

  auto make_market_cell(const std::string& market_code) {
    auto query_model = std::make_shared<LocalComboBoxQueryModel>();
    auto market = GetDefaultMarketDatabase().FromCode(market_code);
    query_model->add(to_text(MarketToken(market.m_code)).toLower(), market);
    query_model->add(QString(market.m_code.GetData()).toLower(), market);
    auto market_box = new MarketBox(std::move(query_model),
      std::make_shared<LocalValueModel<MarketCode>>(market_code));
    market_box->set_read_only(true);
    return market_box;
  }

  auto make_condition_cell(const TimeAndSale::Condition& condition) {
    auto query_model = std::make_shared<LocalComboBoxQueryModel>();
    query_model->add(QString::fromStdString(condition.m_code).toLower(),
      SaleConditionInfo(condition, ""));
    auto condition_box = new SaleConditionBox(std::move(query_model),
      std::make_shared<LocalValueModel<TimeAndSale::Condition>>(condition));
    condition_box->set_read_only(true);
    return condition_box;
  }

  void customize_empty_header_cell(TableHeader& header, int width) {
    auto& empty_header_cell = *static_cast<TableHeaderItem*>(
      header.layout()->itemAt(TimeAndSalesTableModel::COLUMN_SIZE)->widget());
    auto layout = empty_header_cell.layout();
    //layout->setContentsMargins({width, scale_height(5), 0, 0});
    layout->setContentsMargins({0, 0, 0, 0});
    auto contents_layout =
      layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
    contents_layout->setContentsMargins({});
    contents_layout->itemAt(2)->widget()->setFixedWidth(0);
    contents_layout->itemAt(3)->widget()->setFixedWidth(0);
    auto bottom_layout = layout->itemAt(1)->layout();
    bottom_layout->setContentsMargins({0, scale_height(2), 0, 0});
    bottom_layout->itemAt(0)->widget()->setFixedWidth(0);
  }

  //ScrollBox* customize_table_body(TableView& table_view) {
  //  auto& table_body = table_view.get_body();
  //  auto pull_indicator = make_pull_indicator();
  //  pull_indicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  //  match(*pull_indicator, PullIndicator());
  //  auto body_scroll_box =
  //    static_cast<ScrollBox*>(table_view.layout()->itemAt(1)->widget());
  //  body_scroll_box->get_body().layout()->addWidget(pull_indicator);

  //  //auto& old_scroll_box =
  //  //  *static_cast<ScrollBox*>(table_view.layout()->itemAt(1)->widget());
  //  //auto time_and_sales_body = new Box(&table_body);
  //  //match(*time_and_sales_body, TimeAndSalesBody());
  //  //auto body = new QWidget();
  //  //body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //  //auto body_layout = make_vbox_layout(body);
  //  //body_layout->addWidget(time_and_sales_body);
  //  //auto pull_indicator = make_pull_indicator();
  //  //pull_indicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  //  //match(*pull_indicator, PullIndicator());
  //  //body_layout->addWidget(pull_indicator);
  //  //body_layout->addStretch(1);
  //  //auto body_scroll_box = new ScrollBox(body);
  //  //body_scroll_box->setSizePolicy(QSizePolicy::Expanding,
  //  //  QSizePolicy::Expanding);
  //  //body_scroll_box->setFocusPolicy(Qt::NoFocus);
  //  //body_scroll_box->set(ScrollBox::DisplayPolicy::ON_ENGAGE);
  //  //auto layout_item =
  //  //  table_view.layout()->replaceWidget(&old_scroll_box, body_scroll_box);
  //  //delete layout_item->widget();
  //  //delete layout_item;
  //  //auto stretch_item = table_view.layout()->itemAt(2);
  //  //table_view.layout()->removeItem(stretch_item);
  //  //delete stretch_item;
  //  //table_view.setFocusProxy(&table_body);
  //  return body_scroll_box;
  //}

  QWidget* table_view_builder(const std::shared_ptr<TableModel>& table, int row,
      int column) {
    auto column_id = static_cast<TimeAndSalesTableModel::Column>(column);
    auto cell = [&] () -> QWidget* {
      if(column_id == TimeAndSalesTableModel::Column::TIME) {
        return make_time_cell(table->get<ptime>(row, column));
      } else if(column_id == TimeAndSalesTableModel::Column::PRICE) {
        return make_decimal_cell<MoneyBox>(table->get<Money>(row, column));
      } else if(column_id == TimeAndSalesTableModel::Column::SIZE) {
        return make_decimal_cell<QuantityBox>(table->get<Quantity>(row, column));
      } else if(column_id == TimeAndSalesTableModel::Column::MARKET) {
        return make_market_cell(table->get<std::string>(row, column));
      } else if(column_id == TimeAndSalesTableModel::Column::CONDITION) {
        return
          make_condition_cell(table->get<TimeAndSale::Condition>(row, column));
      }
      return make_label("");
      }();
    if(column_id != TimeAndSalesTableModel::Column::TIME) {
      update_style(*cell, [] (auto& style) {
        style = TABLE_CELL_STYLE(style);
      });
    }
    //cell->setEnabled(false);
    return cell;
  }
}

TableView* Spire::make_time_and_sales_table_view(std::shared_ptr<TimeAndSalesTableModel> table,
    QWidget* parent) {
  auto table_view = TableViewBuilder(table).
    set_header(make_header_model()).
    set_view_builder(table_view_builder).make();
  update_style(*table_view, [] (auto& style) {
    style = TABLE_VIEW_STYLE(style);
  });
  auto& header_box =
    *static_cast<Box*>(table_view->layout()->itemAt(0)->widget());
  auto& header = table_view->get_header();
  auto header_scroll_box = new ScrollBox(&header);
  header_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  header_scroll_box->setFocusPolicy(Qt::NoFocus);
  header_scroll_box->set_horizontal(ScrollBox::DisplayPolicy::NEVER);
  header_scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
  update_style(*header_scroll_box, [] (auto& style) {
    style = TABLE_HADER_STYLE(style);
  });
  auto old_header_box =
    table_view->layout()->replaceWidget(&header_box, header_scroll_box);
  delete old_header_box->widget();
  delete old_header_box;
  customize_empty_header_cell(header, header_scroll_box->get_vertical_scroll_bar().sizeHint().width());
  auto properties = make_header_item_properties();
  for(auto i = 0; i < std::ssize(properties); ++i) {
    auto item = table_view->get_header().get_item(i);
    auto item_layout = item->layout();
    item_layout->setContentsMargins({scale_width(4), scale_height(5), 0, scale_height(2)});
    auto contents_layout =
      item_layout->itemAt(0)->layout()->itemAt(0)->widget()->layout();
    contents_layout->setContentsMargins({});
    //auto bottom_layout = item_layout->itemAt(1)->layout();
    //bottom_layout->setContentsMargins({scale_width(4), scale_height(4), 0, 0});
    //item->setVisible(properties[i].m_is_visible);
    //if(!properties[i].m_is_visible) {
    //  table_view->get_header().get_widths()->set(i, 0);
    //}
    if(properties[i].m_text_align == Qt::AlignRight) {
      auto contents_layout =
        item->layout()->itemAt(0)->layout()->itemAt(0)->widget()->layout();
      static_cast<QSpacerItem*>(contents_layout->itemAt(1))->changeSize(0, 0);
      contents_layout->itemAt(2)->widget()->setFixedWidth(0);
      contents_layout->itemAt(3)->widget()->setFixedWidth(0);
      update_style(*item, [] (auto& style) {
        style.get(Any() > TableHeaderItem::Label()).
          set(TextAlign(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter)));
        });
    }
  }
  auto& table_body = table_view->get_body();
  auto pull_indicator = make_pull_indicator();
  pull_indicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto body_scroll_box =
    static_cast<ScrollBox*>(table_view->layout()->itemAt(1)->widget());
  body_scroll_box->get_body().layout()->addWidget(pull_indicator);
  //auto body_scroll_box = customize_table_body(*table_view);
  body_scroll_box->get_horizontal_scroll_bar().connect_position_signal(
    [=] (int position) {
      header_scroll_box->get_horizontal_scroll_bar().set_position(position);
    });
  match(*pull_indicator, PullIndicator());
  auto status = std::make_shared<Status>(false, false, 0);
  auto timer = new QTimer(table_view);
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, [=] {
    match(*table_view, PullDelayed());
    body_scroll_box->get_body().adjustSize();
    scroll_to_end(body_scroll_box->get_vertical_scroll_bar());
  });
  body_scroll_box->get_vertical_scroll_bar().connect_position_signal(
    [=] (int position) {
      if(!status->m_is_loading && position > status->m_last_scroll_y) {
        auto& scroll_bar = body_scroll_box->get_vertical_scroll_bar();
        if(scroll_bar.get_range().m_end - position <
          scroll_bar.get_page_size() / 2) {
          table->load_history(
            body_scroll_box->height() / table_view->get_body().get_item(TableBody::Index{0,0})->height());
        }
      }
      status->m_last_scroll_y = position;
    });

  table->connect_begin_loading_signal([=] {
    if(status->m_is_loading) {
      return;
    }
    status->m_is_loading = true;
    status->m_has_additional_entries = false;
    timer->start(1000);
  });
  table->connect_end_loading_signal([=] {
    status->m_is_loading = false;
    if(status->m_has_additional_entries) {
      unmatch(*table_view, NoAdditionalEntries());
    } else {
      match(*table_view, NoAdditionalEntries());
    }
    timer->stop();
    unmatch(*table_view, PullDelayed());
  });
  table->connect_operation_signal([=] (const TableModel::Operation& operation) {
    visit(operation,
      [&] (const TableModel::AddOperation& operation) {
      status->m_has_additional_entries = true;
      //get_item({operation.m_index, 0})->parentWidget()->show();
      body_scroll_box->get_body().adjustSize();
    });
  });

  return table_view;
}
