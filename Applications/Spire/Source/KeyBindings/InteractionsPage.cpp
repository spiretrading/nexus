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
  std::shared_ptr<RegionInteractionsMap> m_region_interactions;

  explicit RegionInteractionsListModel(
      std::shared_ptr<RegionInteractionsMap> region_interactions)
      : m_region_interactions(std::move(region_interactions)) {
    push(Region());
    for(auto i = m_region_interactions->Begin();
        i != m_region_interactions->End(); ++i) {
      push(std::get<0>(*i));
    }
  }

  void push_region(const Region& region) {
    m_region_interactions->Set(region,
      std::make_shared<InteractionsKeyBindingsModel>());
    push(region);
  }

  void remove_region(const Region& region) {
    m_region_interactions->Erase(region);
    auto index = [&] {
      for(auto i = 0; i < get_size(); ++i) {
        if(get(i) == region) {
          return i;
        }
      }
      return -1;
    }();
    remove(index);
  }
};

InteractionsPage::InteractionsPage(std::shared_ptr<RegionListModel> regions,
    std::shared_ptr<RegionInteractionsMap> region_interactions, QWidget* parent)
    : QWidget(parent),
      m_regions(std::move(regions)) {
  m_list_model = std::make_shared<RegionInteractionsListModel>(
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
  m_center_layout = make_vbox_layout();
  m_center_layout->addStretch(1);
  auto scroll_box_body_layout = make_hbox_layout(scroll_box_body);
  scroll_box_body_layout->addStretch(0);
  scroll_box_body_layout->addLayout(m_center_layout, 1);
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
  m_add_region_from = new AddRegionForm(m_regions, *this);
  m_add_region_from->connect_submit_signal(
    std::bind_front(&InteractionsPage::on_add_region, this));
  m_list_view_current->set(std::get<0>(*get_region_interactions()->Begin()));
}

const std::shared_ptr<RegionListModel>& InteractionsPage::get_regions() const {
  return m_regions;
}

const std::shared_ptr<RegionInteractionsMap>&
    InteractionsPage::get_region_interactions() const {
  return m_list_model->m_region_interactions;
}

connection InteractionsPage::connect_add_signal(
    const AddSignal::slot_type& slot) const {
  return m_add_signal.connect(slot);
}

connection InteractionsPage::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

QWidget* InteractionsPage::make_interactions_form(const Region& region) const {
  auto interactions_form = new InteractionsKeyBindingsForm(region,
    std::get<1>(*get_region_interactions()->Find(region)));
  interactions_form->setMinimumWidth(scale_width(384));
  interactions_form->setMaximumWidth(scale_width(480));
  return interactions_form;
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
  m_add_region_from->show();
}

void InteractionsPage::on_current_index(const optional<int>& current) {
  if(current) {
    auto item = m_list_view->get_list_item(*current);
    item->set_selected(true);
    item->setFocusPolicy(Qt::NoFocus);
  }
  m_list_view->setFocusPolicy(Qt::NoFocus);
}

void InteractionsPage::on_current_region(const Region& region) {
  if(m_current_region == region) {
    return;
  }
  if(auto widget = m_center_layout->itemAt(0)->widget(); widget) {
    auto item = m_center_layout->replaceWidget(
      m_center_layout->itemAt(0)->widget(), make_interactions_form(region));
    if(item) {
      delete item->widget();
      delete item;
    }
  } else {
    m_center_layout->insertWidget(0, make_interactions_form(region));
  }
  m_current_region = region;
}

void InteractionsPage::on_add_region(const Region& region) {
  if(std::get<0>(*get_region_interactions()->Find(region)) == region) {
    m_list_view_current->set(region);
  } else {
    m_list_model->push_region(region);
    m_list_view_current->set(region);
    m_list_view->get_list_item(
      m_list_model->get_size() - 1)->setFocusPolicy(Qt::NoFocus);
    m_add_signal(region, get_region_interactions()->Get(region));
  }
  m_add_region_from->close();
}

void InteractionsPage::on_delete_region(const Region& region) {
  m_list_model->remove_region(region);
  m_delete_signal(region);
}
