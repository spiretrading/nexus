#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LineInputForm.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_help_text_box() {
    auto help_text_box = make_text_area_label(
      QObject::tr("Allowed keys are: <b>F1–F12</b> and <b>Ctrl, Shift, "
        "Alt  +  F1–F12</b> and <b>Ctrl, Shift, Alt  +  0–9</b>"));
    help_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*help_text_box, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(text_style(font, QColor(0x808080))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(10))).
        set(PaddingBottom(scale_height(5)));
    });
    return help_text_box;
  }

  auto make_button(const QString& path, const QString& tooltip) {
    auto button = make_icon_button(imageFromSvg(path, scale(16, 16)), tooltip);
    button->setFixedSize(scale(26, 26));
    button->setFocusPolicy(Qt::TabFocus);
    return button;
  }

  auto populate_region_box_model() {
    auto securities = std::vector<std::pair<std::string, std::string>>{
      {"MSFT.NSDQ", "Microsoft Corporation"},
      {"MG.TSX", "Magna International Inc."},
      {"MRU.TSX", "Metro Inc."},
      {"MFC.TSX", "Manulife Financial Corporation"},
      {"MX.TSX", "Methanex Corporation"},
      {"TSO.ASX", "Tesoro Resources Limited"}};
    auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
      DefaultMarkets::ISE(), DefaultMarkets::CSE(), DefaultMarkets::TSX(),
      DefaultMarkets::TSXV(), DefaultMarkets::BOSX()};
    auto countries = std::vector<CountryCode>{DefaultCountries::US(),
      DefaultCountries::CA(), DefaultCountries::AU(), DefaultCountries::JP(),
      DefaultCountries::CN()};
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& security_info : securities) {
      auto security = ParseSecurity(security_info.first);
      auto region = Region(security);
      region.SetName(security_info.second);
      model->add(to_text(security).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market_code : markets) {
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      auto region = Region(market);
      region.SetName(market.m_description);
      model->add(to_text(MarketToken(market.m_code)).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& country : countries) {
      auto region = Region(country);
      region.SetName(
        GetDefaultCountryDatabase().FromCode(country).m_name);
      model->add(to_text(country).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return model;
  }
}

TaskKeysPage::TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
    DestinationDatabase destinations, MarketDatabase markets, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_operation_connection(
        m_key_bindings->get_order_task_arguments()->connect_operation_signal(
          std::bind_front(&TaskKeysPage::on_operation, this))) {
  auto toolbar_body = new QWidget();
  auto toolbar_layout = make_hbox_layout(toolbar_body);
  auto search_box = new SearchBox();
  search_box->set_placeholder(tr("Search tasks"));
  search_box->setFixedWidth(scale_width(368));
  toolbar_layout->addWidget(search_box);
  toolbar_layout->addStretch();
  toolbar_layout->addSpacing(scale_width(18));
  auto add_task_button =
    make_button(":/Icons/add.svg", tr("Add Task (Shift + Enter)"));
  add_task_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_new_task_action, this));
  toolbar_layout->addWidget(add_task_button);
  toolbar_layout->addSpacing(scale_width(4));
  m_duplicate_button =
    make_button(":/Icons/duplicate.svg", tr("Duplicate (Ctrl + D)"));
  m_duplicate_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_duplicate_task_action, this));
  toolbar_layout->addWidget(m_duplicate_button);
  toolbar_layout->addSpacing(scale_width(4));
  m_delete_button = make_button(":/Icons/delete3.svg", tr("Delete"));
  m_delete_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_delete_task_action, this));
  update_style(*m_delete_button, [] (auto& style) {
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(Fill(QColor(0xB71C1C)));
  });
  toolbar_layout->addWidget(m_delete_button);
  auto toolbar = new Box(toolbar_body);
  update_style(*toolbar, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingTop(scale_height(5))).
      set(PaddingBottom(scale_height(10))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
  });
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(make_help_text_box());
  layout->addWidget(toolbar);
  m_table_view = make_task_keys_table_view(
    m_key_bindings->get_order_task_arguments(),
    populate_region_box_model(),
    std::move(destinations), std::move(markets));
  m_table_view->installEventFilter(this);
  layout->addWidget(m_table_view);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  enclose(*this, *box);
  update_button_state();
  auto& row_selection = m_table_view->get_selection()->get_row_selection();
  m_selection_connection = row_selection->connect_operation_signal(
    std::bind_front(&TaskKeysPage::on_row_selection, this));
}

const std::shared_ptr<KeyBindingsModel>&
    TaskKeysPage::get_key_bindings() const {
  return m_key_bindings;
}

bool TaskKeysPage::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table_view) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.modifiers() & Qt::ShiftModifier &&
          (key_event.key() == Qt::Key_Return ||
            key_event.key() == Qt::Key_Enter)) {
        on_new_task_action();
        return true;
      } else if(key_event.modifiers() & Qt::ControlModifier &&
          key_event.key() == Qt::Key_D) {
        on_duplicate_task_action();
        return true;
      }
    }
  } else if(watched == m_added_region_item) {
    if(event->type() == QEvent::Show) {
      auto enter_event =
        QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
      QCoreApplication::sendEvent(&m_added_region_item->get_body(),
        &enter_event);
      m_added_region_item->removeEventFilter(this);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TaskKeysPage::update_button_state() {
  auto is_enabled =
    m_table_view->get_selection()->get_row_selection()->get_size() > 0;
  m_duplicate_button->setEnabled(is_enabled);
  m_delete_button->setEnabled(is_enabled);
}

void TaskKeysPage::on_new_task_action() {
  m_new_task_form = new LineInputForm(tr("New Task"), *this);
  m_new_task_form->connect_submit_signal(
    std::bind_front(&TaskKeysPage::on_new_task_submission, this));
  m_new_task_form->show();
  auto window = m_new_task_form->window();
  window->move(
    mapToGlobal(QPoint(0, 0)) + rect().center() - window->rect().center());
}

void TaskKeysPage::on_duplicate_task_action() {
  auto& selection = m_table_view->get_selection()->get_row_selection();
  if(selection->get_size() == 0) {
    return;
  }
  auto last_current_row = m_table_view->get_current()->get()->m_row;
  auto sorted_selection = std::vector<int>();
  sorted_selection.assign(selection->begin(), selection->end());
  std::sort(sorted_selection.begin(), sorted_selection.end(), std::greater<int>());
  auto& table = m_table_view->get_body().get_table();
  for(auto index : sorted_selection) {
    auto order_task = OrderTaskArguments();
    order_task.m_name = table->get<QString>(index, 1);
    order_task.m_region = table->get<Region>(index, 2);
    order_task.m_destination = table->get<Destination>(index, 3);
    order_task.m_order_type = table->get<OrderType>(index, 4);
    order_task.m_side = table->get<Side>(index, 5);
    order_task.m_quantity = table->get<optional<Quantity>>(index, 6);
    order_task.m_time_in_force = table->get<TimeInForce>(index, 7);
    order_task.m_additional_tags =
      table->get<std::vector<Nexus::Tag>>(index, 8);
    m_key_bindings->get_order_task_arguments()->insert(order_task,
      m_table_view->get_current()->get()->m_row);
  }
  QTimer::singleShot(0, this, [=] {
    m_table_view->get_current()->set(TableView::Index(last_current_row, 1));
  });
}

void TaskKeysPage::on_delete_task_action() {
  auto& selection = m_table_view->get_selection()->get_row_selection();
  for(auto i = 0; i < selection->get_size(); ++i) {
    m_key_bindings->get_order_task_arguments()->remove(selection->get(i));
  }
}

void TaskKeysPage::on_new_task_submission(const QString& name) {
  auto order_task = OrderTaskArguments();
  order_task.m_name = name;
  auto current_index = TableView::Index();
  if(auto& current = m_table_view->get_current()->get()) {
    m_key_bindings->get_order_task_arguments()->insert(order_task,
      current->m_row);
    current_index = {current->m_row, 2};
  } else {
    m_key_bindings->get_order_task_arguments()->push(order_task);
    current_index =
      {m_key_bindings->get_order_task_arguments()->get_size() - 1, 2};
  }
  QTimer::singleShot(0, this, [=] {
    m_table_view->get_current()->set(current_index);
    m_added_region_item = m_table_view->get_body().get_item(current_index);
    m_added_region_item->installEventFilter(this);
  });
}

void TaskKeysPage::on_row_selection(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      update_button_state();
    },
    [&] (const ListModel<int>::RemoveOperation& operation) {
      update_button_state();
    });
}

void TaskKeysPage::on_operation(
    const OrderTaskArgumentsListModel::Operation& operation) {
  visit(operation,
    [&] (const OrderTaskArgumentsListModel::RemoveOperation& operation) {
      if(m_key_bindings->get_order_task_arguments()->get_size() == 0) {
        QTimer::singleShot(0, this, [=] {
          m_table_view->setFocus();
        });
      }
    });
}
