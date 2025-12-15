#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/AddRegionForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Spire/Utility.hpp"
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
  void apply_deletable_list_item_style(StyleSheet& style) {
    style.get((Any() > is_a<ListItem>() > is_a<DeletableListItem>()) <<
        is_a<ListItem>()).
      set(vertical_padding(0)).
      set(horizontal_padding(0));
    style.get(Any() > (is_a<ListItem>() && Selected()) >
        is_a<DeletableListItem>() > is_a<Button>()).set(Visibility::VISIBLE);
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

  bool region_comparator(const Region& left, const Region& right) {
    if(left.is_global()) {
      return !right.is_global();
    } else if(left.get_countries().size() != 0) {
      if(right.get_countries().size() == 0) {
        return !right.is_global();
      }
      return to_text(*left.get_countries().begin()) <
        to_text(*right.get_countries().begin());
    } else if(left.get_venues().size() != 0) {
      if(right.get_venues().size() == 0) {
        return !right.is_global() && right.get_countries().size() == 0;
      }
      return to_text(*left.get_venues().begin()) <
        to_text(*right.get_venues().begin());
    }
    if(left.get_securities().size() != 0) {
      if(right.get_securities().size() == 0) {
        return !right.is_global() && right.get_countries().size() == 0 &&
          right.get_venues().size() == 0;
      }
      return to_text(*left.get_securities().begin()) <
        to_text(*right.get_securities().begin());
    }
    return false;
  }

  auto make_region_list(const KeyBindingsModel& key_bindings) {
    auto regions = std::make_shared<ArrayListModel<Region>>(
      key_bindings.make_interactions_key_bindings_regions());
    regions->insert(Region::make_global("Global"), 0);
    std::sort(regions->begin() + 1, regions->end(), &region_comparator);
    return regions;
  }

  auto make_available_region_list(const KeyBindingsModel& key_bindings) {
    auto regions = std::make_shared<ArrayListModel<Region>>();
    for(auto& country : DEFAULT_COUNTRIES.get_entries()) {
      auto region = Region(country.m_name);
      region += country.m_code;
      regions->push(region);
    }
    for(auto& venue : DEFAULT_VENUES.get_entries()) {
      auto region = Region(venue.m_description);
      region += venue.m_venue;
      regions->push(region);
    }
    for(auto& region : key_bindings.make_interactions_key_bindings_regions()) {
      auto i = std::find(regions->begin(), regions->end(), region);
      if(i != regions->end()) {
        regions->remove(i);
      }
    }
    std::sort(regions->begin(), regions->end(), &region_comparator);
    return regions;
  }
}

InteractionsPage::InteractionsPage(
    std::shared_ptr<KeyBindingsModel> key_bindings, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_add_region_form(nullptr) {
  m_regions = make_region_list(*m_key_bindings);
  m_available_regions = make_available_region_list(*m_key_bindings);
  auto list_view = new ListView(
    m_regions, std::bind_front(&InteractionsPage::make_region_list_item, this));
  list_view->setFocusPolicy(Qt::NoFocus);
  list_view->get_list_item(0)->setEnabled(false);
  m_current_region = make_transform_value_model(list_view->get_current(),
    [=] (const auto& index) {
      return m_regions->get(index.value_or(0));
    },
    [=] (const auto& region) {
      auto i = std::find(m_regions->rbegin(), m_regions->rend() - 1, region);
      if(i != m_regions->rend()) {
        return std::distance(m_regions->begin(), i.base()) - 1;
      }
      throw std::invalid_argument("Region not found.");
    });
  update_style(*list_view, apply_deletable_list_item_style);
  m_list_box = new ScrollableListBox(*list_view);
  m_list_box->setFocusPolicy(Qt::NoFocus);
  m_list_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_list_box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0));
  });
  m_add_region_button = make_label_button(tr("Add Region"));
  m_add_region_button->setFixedWidth(scale_width(120));
  m_add_region_button->connect_click_signal(
    std::bind_front(&InteractionsPage::on_add_region_click, this));
  auto action_body = new QWidget();
  auto action_body_layout = make_vbox_layout(action_body);
  action_body_layout->addWidget(m_add_region_button, 0, Qt::AlignLeft);
  auto action_box = new Box(action_body);
  action_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*action_box, apply_action_box_style);
  auto master_body = new QWidget();
  auto master_body_layout = make_vbox_layout(master_body);
  master_body_layout->addWidget(m_list_box);
  master_body_layout->addWidget(action_box);
  auto master_box = new Box(master_body);
  master_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  master_box->setFixedWidth(scale_width(240));
  update_style(*master_box, apply_master_box_style);
  auto scroll_box_body = new QWidget();
  scroll_box_body->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto center_layout = make_vbox_layout();
  m_interactions_form =
    new InteractionsKeyBindingsForm(m_key_bindings, m_current_region);
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
  navigate_to_index(*m_list_box, 1);
}

QWidget* InteractionsPage::make_region_list_item(
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
  m_add_region_form = new AddRegionForm(m_available_regions, *this);
  m_add_region_form->connect_submit_signal(
    std::bind_front(&InteractionsPage::on_add_region, this));
  m_add_region_form->show();
  auto window = m_add_region_form->window();
  window->move(
    mapToGlobal(QPoint(0, 0)) + rect().center() - window->rect().center());
}

void InteractionsPage::on_current_index(const optional<int>& current) {
  if(current) {
    if(auto item = m_list_box->get_list_view().get_list_item(*current)) {
      item->setFocusPolicy(Qt::NoFocus);
    }
  }
}

void InteractionsPage::on_add_region(const Region& region) {
  if(!m_add_region_form) {
    return;
  }
  m_add_region_form->close();
  delete_later(m_add_region_form);
  auto i = std::lower_bound(
    m_regions->begin(), m_regions->end(), region, &region_comparator);
  if(m_regions->insert(region, i) == QValidator::Acceptable) {
    auto i = std::find(
      m_available_regions->begin(), m_available_regions->end(), region);
    if(i != m_available_regions->end()) {
      m_available_regions->remove(i);
      m_add_region_button->setDisabled(m_available_regions->get_size() == 0);
    }
    navigate_to_value(*m_list_box, region);
    m_list_box->get_list_view().setFocusProxy(nullptr);
    m_list_box->get_list_view().setFocusPolicy(Qt::StrongFocus);
  }
}

void InteractionsPage::on_delete_region(const Region& region) {
  QTimer::singleShot(0, this, [=] {
    auto current = m_list_box->get_list_view().get_current()->get();
    auto i = std::find(m_regions->begin() + 1, m_regions->end(), region);
    if(i != m_regions->end() &&
        m_regions->remove(i) == QValidator::Acceptable) {
      m_key_bindings->get_interactions_key_bindings(region)->reset();
      if(region.get_securities().empty()) {
        auto i = std::lower_bound(m_available_regions->begin(),
          m_available_regions->end(), region, &region_comparator);
        m_available_regions->insert(region, i);
        m_add_region_button->setDisabled(m_available_regions->get_size() == 0);
      }
      if(current) {
        auto size = m_list_box->get_list_view().get_list()->get_size();
        if(size > 0) {
          if(*current >= size) {
            --*current;
          }
          navigate_to_index(m_list_box->get_list_view(), *current);
          m_list_box->get_list_view().setFocus();
        }
      }
    }
  });
}
