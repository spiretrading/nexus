#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/AddRegionForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListIndexValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DeletableListItem.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto modifiers = std::array<Qt::KeyboardModifier, 4>{
    Qt::NoModifier, Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier};

  void apply_deletable_list_item_style(StyleSheet& style) {
    style.get((Any() > is_a<DeletableListItem>()) << is_a<ListItem>()).
      set(vertical_padding(0)).
      set(horizontal_padding(0));
    style.get(Any() > (is_a<ListItem>() && Selected()) >
        is_a<DeletableListItem>() > is_a<Button>()).
      set(Visibility::VISIBLE);
    style.get(Any() > (is_a<ListItem>() && Current()) >
        is_a<DeletableListItem>() >
          (is_a<Button>() && (Hover() || Press())) > Body()).
      set(BackgroundColor(QColor(0xD0CEEB)));
  }

  void apply_action_box_style(StyleSheet& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8))).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0)));
  }

  void apply_master_box_style(StyleSheet& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderRightSize(scale_width(1))).
      set(BorderRightColor(QColor(0xE0E0E0)));
  }
}

struct InteractionsPage::RegionInteractionsListModel : ArrayListModel<Region> {
  std::shared_ptr<RegionInteractionsMap> m_source;

  explicit RegionInteractionsListModel(
      std::shared_ptr<RegionInteractionsMap> source)
      : m_source(std::move(source)) {
    ArrayListModel<Region>::insert(Region(), 0);
    for(auto i = m_source->Begin(); i != m_source->End(); ++i) {
      auto& region = std::get<0>(*i);
      ArrayListModel<Region>::insert(region, find_insert_index(region));
    }
  }

  QValidator::State insert(const Region& value, int index) override {
    if(std::get<0>(*m_source->Find(value)) == value) {
      return QValidator::State::Invalid;
    }
    auto result = ArrayListModel<Region>::insert(value,
      find_insert_index(value));
    if(result == QValidator::State::Acceptable) {
      m_source->Set(value, make_key_bindings(value));
    }
    return result;
  }

  QValidator::State remove(int index) override {
    m_source->Erase(get(index));
    return ArrayListModel<Region>::remove(index);
  }

  auto make_key_bindings(const Region& region) const {
    auto key_bindings = std::make_shared<InteractionsKeyBindingsModel>();
    auto base_region = [&] {
      if(!region.GetMarkets().empty()) {
        for(auto i = 2; i < get_size(); ++i) {
          if(get(i) > region && !get(i).GetCountries().empty()) {
            return get(i);
          }
        }
      } else if(!region.GetCountries().empty()) {
        return get(1);
      }
      return get(1);
    }();
    auto base_key_bindings = m_source->Get(base_region);
    key_bindings->get_default_quantity()->set(
      base_key_bindings->get_default_quantity()->get());
    key_bindings->is_cancel_on_fill()->set(
      base_key_bindings->is_cancel_on_fill()->get());
    for(auto modifier : modifiers) {
      key_bindings->get_quantity_increment(modifier)->set(
        base_key_bindings->get_quantity_increment(modifier)->get());
      key_bindings->get_price_increment(modifier)->set(
        base_key_bindings->get_price_increment(modifier)->get());
    }
    return key_bindings;
  }

  int find_insert_index(const Region& region) const {
    for(auto i = 1; i < get_size(); ++i) {
      if(!get(i).GetCountries().empty() && !region.GetCountries().empty()) {
        if(to_text(*get(i).GetCountries().begin()) >
            to_text(*region.GetCountries().begin())) {
          return i;
        }
      } else if(!get(i).GetMarkets().empty() && !region.GetMarkets().empty()) {
        if(to_text(MarketToken(*get(i).GetMarkets().begin())) >
            to_text(MarketToken(*region.GetMarkets().begin()))) {
          return i;
        }
      } else if(!get(i).GetMarkets().empty() &&
          !region.GetCountries().empty()) {
        return i;
      }
    }
    return get_size();
  }
};

struct InteractionsPage::RegionInteractionsKeyBindingsModel :
    InteractionsKeyBindingsModel {
  std::vector<scoped_connection> m_connections;

  void set_key_bindings(
      std::shared_ptr<InteractionsKeyBindingsModel> key_bindings) {
    m_connections.clear();
    get_default_quantity()->set(key_bindings->get_default_quantity()->get());
    m_connections.push_back(get_default_quantity()->connect_update_signal(
      [=] (auto& value) {
        key_bindings->get_default_quantity()->set(value);
      }));
    is_cancel_on_fill()->set(key_bindings->is_cancel_on_fill()->get());
    m_connections.push_back(is_cancel_on_fill()->connect_update_signal(
      [=] (auto& value) {
        key_bindings->is_cancel_on_fill()->set(value);
      }));
    for(auto modifier : modifiers) {
      get_quantity_increment(modifier)->set(
        key_bindings->get_quantity_increment(modifier)->get());
      m_connections.push_back(
        get_quantity_increment(modifier)->connect_update_signal(
          [=] (auto& value) {
            key_bindings->get_quantity_increment(modifier)->set(value);
          }));
      get_price_increment(modifier)->set(
        key_bindings->get_price_increment(modifier)->get());
      m_connections.push_back(
        get_price_increment(modifier)->connect_update_signal(
          [=] (auto& value) {
            key_bindings->get_price_increment(modifier)->set(value);
          }));
    }
  }
};

InteractionsPage::InteractionsPage(std::shared_ptr<RegionListModel> regions,
    std::shared_ptr<RegionInteractionsMap> region_interactions, QWidget* parent)
    : QWidget(parent) {
  m_list_model =
    std::make_shared<RegionInteractionsListModel>(
      std::move(region_interactions));
  m_list_view_current = std::make_shared<LocalRegionModel>();
  m_list_view_current->connect_update_signal(
    std::bind_front(&InteractionsPage::on_current_region, this));
  m_list_view = new ListView(m_list_model,
    std::make_shared<ListIndexValueModel<Region>>(m_list_model,
      m_list_view_current),
    std::make_shared<ArrayListModel<int>>(),
    std::bind_front(&InteractionsPage::make_list_item, this));
  m_list_view->setFocusPolicy(Qt::NoFocus);
  m_list_view->get_list_item(0)->setEnabled(false);
  m_list_view->get_current()->connect_update_signal(
    std::bind_front(&InteractionsPage::on_current_index, this));
  update_style(*m_list_view, apply_deletable_list_item_style);
  auto scrollable_list_box = new ScrollableListBox(*m_list_view);
  scrollable_list_box->setFocusPolicy(Qt::NoFocus);
  scrollable_list_box->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*scrollable_list_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0));
  });
  auto button = make_label_button(tr("Add Region"));
  button->setFixedWidth(scale_width(120));
  button->connect_click_signal(
    std::bind_front(&InteractionsPage::on_add_region_click, this));
  auto action_body = new QWidget();
  auto action_body_layout = make_vbox_layout(action_body);
  action_body_layout->addWidget(button, 0, Qt::AlignLeft);
  auto action_box = new Box(action_body);
  action_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*action_box, apply_action_box_style);
  auto master_body = new QWidget();
  auto master_body_layout = make_vbox_layout(master_body);
  master_body_layout->addWidget(scrollable_list_box);
  master_body_layout->addWidget(action_box);
  auto master_box = new Box(master_body);
  master_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  master_box->setFixedWidth(scale_width(240));
  update_style(*master_box, apply_master_box_style);
  auto scroll_box_body = new QWidget();
  scroll_box_body->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  auto root_region = std::get<0>(*get_region_interactions()->Begin());
  auto center_layout = make_vbox_layout();
  m_interactions_form_model =
    std::make_shared<RegionInteractionsKeyBindingsModel>();
  m_interactions_form =
    new InteractionsKeyBindingsForm(root_region, m_interactions_form_model);
  m_interactions_form->setMinimumWidth(scale_width(384));
  m_interactions_form->setMaximumWidth(scale_width(480));
  center_layout->addWidget(m_interactions_form);
  center_layout->addStretch(1);
  auto scroll_box_body_layout = make_hbox_layout(scroll_box_body);
  scroll_box_body_layout->addStretch(0);
  scroll_box_body_layout->addLayout(center_layout, 1);
  scroll_box_body_layout->addStretch(0);
  auto scroll_box = new ScrollBox(scroll_box_body);
  scroll_box->setFocusPolicy(Qt::NoFocus);
  scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*scroll_box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  auto layout = make_hbox_layout(this);
  layout->addWidget(master_box);
  layout->addWidget(scroll_box);
  m_add_region_form = new AddRegionForm(std::move(regions), *this);
  m_add_region_form->connect_submit_signal(
    std::bind_front(&InteractionsPage::on_add_region, this));
  m_list_view_current->set(root_region);
}

const std::shared_ptr<RegionListModel>& InteractionsPage::get_regions() const {
  return m_add_region_form->get_regions();
}

const std::shared_ptr<RegionInteractionsMap>&
    InteractionsPage::get_region_interactions() const {
  return m_list_model->m_source;
}

connection InteractionsPage::connect_add_signal(
    const AddSignal::slot_type& slot) const {
  return m_add_signal.connect(slot);
}

connection InteractionsPage::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

QWidget* InteractionsPage::make_list_item(
    const std::shared_ptr<RegionListModel>& list, int index) {
  if(index == 0) {
    auto label = make_label(tr("Regions"));
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(10));
      style.get(Any()).set(text_style(font, QColor(0x808080)));
    });
    return label;
  } else if(index == 1) {
    return make_label(to_text(list->get(index)));
  } else {
    auto& region = list->get(index);
    auto label = make_label(to_text(region));
    label->setFocusPolicy(Qt::NoFocus);
    auto item = new DeletableListItem(*label);
    item->connect_delete_signal(
      std::bind_front(&InteractionsPage::on_delete_region, this, region));
    return item;
  }
}

void InteractionsPage::on_add_region_click() {
  m_add_region_form->show();
  auto add_region_window = m_add_region_form->window();
  add_region_window->move(mapToGlobal(QPoint(0, 0)) + rect().center() -
    add_region_window->rect().center());
}

void InteractionsPage::on_current_index(const optional<int>& current) {
  if(current) {
    if(auto item = m_list_view->get_list_item(*current)) {
      item->setFocusPolicy(Qt::NoFocus);
    }
  }
}

void InteractionsPage::on_current_region(const Region& region) {
  m_interactions_form->set_region(region);
  m_interactions_form_model->set_key_bindings(
    get_region_interactions()->Get(region));
}

void InteractionsPage::on_add_region(const Region& region) {
  m_add_region_form->close();
  auto result = m_list_model->push(region);
  m_list_view_current->set(region);
  if(result == QValidator::Acceptable) {
    m_add_signal(region, get_region_interactions()->Get(region));
  }
}

void InteractionsPage::on_delete_region(const Region& region) {
  auto index = [&] {
    for(auto i = 0; i < m_list_model->get_size(); ++i) {
      if(m_list_model->get(i) == region) {
        return i;
      }
    }
    return -1;
  }();
  if(index > 0 && m_list_model->remove(index) == QValidator::Acceptable) {
    m_list_view->get_selection()->push(*m_list_view->get_current()->get());
    m_delete_signal(region);
  }
}
