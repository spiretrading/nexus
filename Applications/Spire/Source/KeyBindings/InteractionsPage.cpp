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

InteractionsPage::InteractionsPage(
    std::shared_ptr<KeyBindingsModel> key_bindings, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)) {
  m_regions = std::make_shared<ArrayListModel<Region>>(
    m_key_bindings->make_interactions_key_bindings_regions());
  m_regions->insert(Region::Global(), 0);
  m_list_view = new ListView(
    m_regions, std::bind_front(&InteractionsPage::make_region_list_item, this));
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
  auto center_layout = make_vbox_layout();
  m_interactions_form = new InteractionsKeyBindingsForm(Region::Global(),
    m_key_bindings->get_interactions_key_bindings(Region::Global()));
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
  m_add_region_form = new AddRegionForm(m_regions, *this);
  m_add_region_form->connect_submit_signal(
    std::bind_front(&InteractionsPage::on_add_region, this));
//  m_list_view_current->set(root_region);
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
/*
  m_interactions_form->set_region(region);
  m_interactions_form_model->set_key_bindings(
    get_region_interactions()->Get(region));
*/
}

void InteractionsPage::on_add_region(const Region& region) {
/*
  m_add_region_form->close();
  auto result = m_list_model->push(region);
  m_list_view_current->set(region);
  if(result == QValidator::Acceptable) {
    m_add_signal(region, get_region_interactions()->Get(region));
  }
*/
}

void InteractionsPage::on_delete_region(const Region& region) {
  m_key_bindings->get_interactions_key_bindings(region)->reset();
/*
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
*/
}
