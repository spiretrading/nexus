#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/KeyBindings/SearchBarOrderTaskArgumentsListModel.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LineInputForm.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace boost;
using namespace boost::signals2;
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

  Region make_region(const SecurityInfo& security_info) {
    auto region = Region(security_info.m_security);
    region.SetName(security_info.m_name);
    return region;
  }

  auto populate_region_query_model(const CountryDatabase& countries,
      const MarketDatabase& markets) {
    auto regions = std::make_shared<LocalQueryModel<Region>>();
    for(auto& country : countries.GetEntries()) {
      auto region = Region(country.m_code);
      region.SetName(country.m_name);
      regions->add(to_text(country.m_code).toLower(), region);
      regions->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market : markets.GetEntries()) {
      auto region = Region(market);
      region.SetName(market.m_description);
      regions->add(to_text(MarketToken(market.m_code)).toLower(), region);
      regions->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return regions;
  }

  struct ConsolidatedRegionQueryModel : RegionQueryModel {
    std::shared_ptr<SecurityInfoQueryModel> m_securities;
    std::shared_ptr<RegionQueryModel> m_regions;

    ConsolidatedRegionQueryModel(
      std::shared_ptr<SecurityInfoQueryModel> securities,
      std::shared_ptr<RegionQueryModel> regions)
      : m_securities(std::move(securities)),
        m_regions(std::move(regions)) {}

    optional<Region> parse(const QString& query) override {
      if(auto security = m_securities->parse(query)) {
        return make_region(*security);
      }
      return m_regions->parse(query);
    }

    QtPromise<std::vector<Region>> submit(const QString& query) override {
      return m_securities->submit(query).then([=] (auto&& security_result) {
        auto security_matches = [&] {
          try {
            return security_result.Get();
          } catch(const std::exception&) {
            return std::vector<SecurityInfo>();
          }
        }();
        auto regions = std::unordered_set<Region>();
        for(auto& security : security_matches) {
          regions.insert(make_region(security));
        }
        return m_regions->submit(query).then(
          [regions = std::move(regions)] (auto&& region_result) mutable {
            auto region_matches = [&] {
              try {
                return region_result.Get();
              } catch(const std::exception&) {
                return std::vector<Region>();
              }
            }();
            for(auto& region : region_matches) {
              regions.insert(region);
            }
            return std::vector<Region>(std::make_move_iterator(regions.begin()),
              std::make_move_iterator(regions.end()));
          });
      });
    }
  };
}

TaskKeysPage::TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<SecurityInfoQueryModel> securities,
    CountryDatabase countries, MarketDatabase markets,
    DestinationDatabase destinations, AdditionalTagDatabase additional_tags,
    QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)) {
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
    style.get((Hover() || Press()) > is_a<Icon>()).set(Fill(QColor(0xB71C1C)));
  });
  toolbar_layout->addWidget(m_delete_button);
  m_reset_button = make_label_button(tr("Reset"));
  m_reset_button->connect_click_signal(
    std::bind_front(&TaskKeysPage::on_reset, this));
  m_reset_button->setFixedWidth(scale_width(80));
  toolbar_layout->addSpacing(scale_width(18));
  toolbar_layout->addWidget(m_reset_button);
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
  auto filtered_tasks = std::make_shared<SearchBarOrderTaskArgumentsListModel>(
    m_key_bindings->get_order_task_arguments(), search_box->get_current(),
      countries, m_markets, m_destinations);
  m_table_view = make_task_keys_table_view(
    std::move(filtered_tasks), std::make_shared<ConsolidatedRegionQueryModel>(
      std::move(securities), populate_region_query_model(countries, m_markets)),
    m_destinations, m_markets, additional_tags);
  layout->addWidget(m_table_view);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  enclose(*this, *box);
  update_button_state();
  auto& row_selection = m_table_view->get_selection()->get_row_selection();
  row_selection->connect_operation_signal(
    std::bind_front(&TaskKeysPage::on_row_selection, this));
}

const std::shared_ptr<KeyBindingsModel>&
    TaskKeysPage::get_key_bindings() const {
  return m_key_bindings;
}

void TaskKeysPage::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() & Qt::ShiftModifier &&
      (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
    on_new_task_action();
  } else if(
      event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_D) {
    on_duplicate_task_action();
  } else {
    QWidget::keyPressEvent(event);
  }
}

void TaskKeysPage::update_button_state() {
  auto is_enabled =
    m_table_view->get_selection()->get_row_selection()->get_size() > 0;
  m_duplicate_button->setEnabled(is_enabled);
  m_delete_button->setEnabled(is_enabled);
}

void TaskKeysPage::on_new_task_action() {
  auto new_task_form = new LineInputForm(tr("New Task"), *this);
  new_task_form->setAttribute(Qt::WA_DeleteOnClose);
  new_task_form->connect_submit_signal(
    std::bind_front(&TaskKeysPage::on_new_task_submission, this));
  new_task_form->show();
  auto window = new_task_form->window();
  window->move(
    mapToGlobal(QPoint(0, 0)) + rect().center() - window->rect().center());
}

void TaskKeysPage::on_duplicate_task_action() {
  auto& selection = m_table_view->get_selection()->get_row_selection();
  if(selection->get_size() == 0) {
    return;
  }
  auto last_current_row = m_table_view->get_body().get_current()->get()->m_row;
  auto sorted_selection =
    std::vector<int>(selection->begin(), selection->end());
  std::sort(sorted_selection.begin(), sorted_selection.end(),
    std::greater<int>());
  for(auto index : sorted_selection) {
    auto order_task = m_key_bindings->get_order_task_arguments()->get(
      any_cast<int>(m_table_view->get_body().get_table()->at(index, 0)));
    order_task.m_key = QKeySequence();
    m_key_bindings->get_order_task_arguments()->insert(order_task,
      m_table_view->get_body().get_current()->get()->m_row);
  }
  QTimer::singleShot(0, this, [=] {
    m_table_view->get_body().get_current()->set(
      TableView::Index(last_current_row, 1));
  });
}

void TaskKeysPage::on_delete_task_action() {
  m_key_bindings->get_order_task_arguments()->transact([&] {
    for(auto i : *m_table_view->get_selection()->get_row_selection()) {
      m_key_bindings->get_order_task_arguments()->remove(
        any_cast<int>(m_table_view->get_body().get_table()->at(i, 0)));
    }
  });
}

void TaskKeysPage::on_reset() {
  reset_order_task_arguments(
    *m_key_bindings->get_order_task_arguments(), m_markets, m_destinations);
}

void TaskKeysPage::on_new_task_submission(const QString& name) {
  auto order_task = OrderTaskArguments();
  order_task.m_name = name;
  if(auto current = m_table_view->get_current()->get()) {
    m_key_bindings->get_order_task_arguments()->insert(
      order_task, current->m_row);
    m_table_view->get_current()->set(TableView::Index(current->m_row, 1));
  } else {
    m_key_bindings->get_order_task_arguments()->push(order_task);
    m_table_view->get_current()->set(TableView::Index(
      m_key_bindings->get_order_task_arguments()->get_size() - 1, 1));
  }
  auto view_current = m_table_view->get_body().get_current()->get();
  if(auto view_current = m_table_view->get_body().get_current()->get()) {
    if(auto item = m_table_view->get_body().find_item(*view_current)) {
      item->get_body().setFocus(Qt::ActiveWindowFocusReason);
    }
  }
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
