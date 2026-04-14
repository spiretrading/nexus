#include "Spire/KeyBindings/InteractionsPage.hpp"
#include "Spire/KeyBindings/AddScopeForm.hpp"
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

  bool scope_comparator(const Scope& left, const Scope& right) {
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
    if(left.get_tickers().size() != 0) {
      if(right.get_tickers().size() == 0) {
        return !right.is_global() && right.get_countries().size() == 0 &&
          right.get_venues().size() == 0;
      }
      return to_text(*left.get_tickers().begin()) <
        to_text(*right.get_tickers().begin());
    }
    return false;
  }

  auto make_scope_list(const KeyBindingsModel& key_bindings) {
    auto scopes = std::make_shared<ArrayListModel<Scope>>(
      key_bindings.make_interactions_key_bindings_scopes());
    scopes->insert(Scope::make_global("Global"), 0);
    std::sort(scopes->begin() + 1, scopes->end(), &scope_comparator);
    return scopes;
  }

  auto make_available_scope_list(const KeyBindingsModel& key_bindings) {
    auto scopes = std::make_shared<ArrayListModel<Scope>>();
    for(auto& country : DEFAULT_COUNTRIES.get_entries()) {
      auto scope = Scope(country.m_name);
      scope += country.m_code;
      scopes->push(scope);
    }
    for(auto& venue : DEFAULT_VENUES.get_entries()) {
      auto scope = Scope(venue.m_description);
      scope += venue.m_venue;
      scopes->push(scope);
    }
    for(auto& scope : key_bindings.make_interactions_key_bindings_scopes()) {
      auto i = std::find(scopes->begin(), scopes->end(), scope);
      if(i != scopes->end()) {
        scopes->remove(i);
      }
    }
    std::sort(scopes->begin(), scopes->end(), &scope_comparator);
    return scopes;
  }
}

InteractionsPage::InteractionsPage(
    std::shared_ptr<KeyBindingsModel> key_bindings, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_add_scope_form(nullptr) {
  m_scopes = make_scope_list(*m_key_bindings);
  m_available_scopes = make_available_scope_list(*m_key_bindings);
  auto list_view = new ListView(
    m_scopes, std::bind_front(&InteractionsPage::make_scope_list_item, this));
  list_view->setFocusPolicy(Qt::NoFocus);
  list_view->get_list_item(0)->setEnabled(false);
  m_current_scope = make_transform_value_model(list_view->get_current(),
    [=] (const auto& index) {
      return m_scopes->get(index.value_or(0));
    },
    [=] (const auto& scope) {
      auto i = std::find(m_scopes->rbegin(), m_scopes->rend() - 1, scope);
      if(i != m_scopes->rend()) {
        return std::distance(m_scopes->begin(), i.base()) - 1;
      }
      throw std::invalid_argument("Scope not found.");
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
  m_add_scope_button = make_label_button(tr("Add Scope"));
  m_add_scope_button->setFixedWidth(scale_width(120));
  m_add_scope_button->connect_click_signal(
    std::bind_front(&InteractionsPage::on_add_scope_click, this));
  auto action_body = new QWidget();
  auto action_body_layout = make_vbox_layout(action_body);
  action_body_layout->addWidget(m_add_scope_button, 0, Qt::AlignLeft);
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
    new InteractionsKeyBindingsForm(m_key_bindings, m_current_scope);
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

QWidget* InteractionsPage::make_scope_list_item(
    const std::shared_ptr<ScopeListModel>& list, int index) {
  if(index == 0) {
    auto label = make_label(tr("Scopes"));
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
    auto& scope = list->get(index);
    auto label = make_label(to_text(scope));
    label->setFocusPolicy(Qt::NoFocus);
    auto item = new DeletableListItem(*label);
    item->connect_delete_signal(
      std::bind_front(&InteractionsPage::on_delete_scope, this, scope));
    return item;
  }
}

void InteractionsPage::on_add_scope_click() {
  m_add_scope_form = new AddScopeForm(m_available_scopes, *this);
  m_add_scope_form->connect_submit_signal(
    std::bind_front(&InteractionsPage::on_add_scope, this));
  m_add_scope_form->show();
  auto window = m_add_scope_form->window();
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

void InteractionsPage::on_add_scope(const Scope& scope) {
  if(!m_add_scope_form) {
    return;
  }
  m_add_scope_form->close();
  delete_later(m_add_scope_form);
  auto i = std::lower_bound(
    m_scopes->begin(), m_scopes->end(), scope, &scope_comparator);
  if(m_scopes->insert(scope, i) == QValidator::Acceptable) {
    auto i = std::find(
      m_available_scopes->begin(), m_available_scopes->end(), scope);
    if(i != m_available_scopes->end()) {
      m_available_scopes->remove(i);
      m_add_scope_button->setDisabled(m_available_scopes->get_size() == 0);
    }
    navigate_to_value(*m_list_box, scope);
    m_list_box->get_list_view().setFocusProxy(nullptr);
    m_list_box->get_list_view().setFocusPolicy(Qt::StrongFocus);
  }
}

void InteractionsPage::on_delete_scope(const Scope& scope) {
  QTimer::singleShot(0, this, [=] {
    auto current = m_list_box->get_list_view().get_current()->get();
    auto i = std::find(m_scopes->begin() + 1, m_scopes->end(), scope);
    if(i != m_scopes->end() &&
        m_scopes->remove(i) == QValidator::Acceptable) {
      m_key_bindings->get_interactions_key_bindings(scope)->reset();
      if(scope.get_tickers().empty()) {
        auto i = std::lower_bound(m_available_scopes->begin(),
          m_available_scopes->end(), scope, &scope_comparator);
        m_available_scopes->insert(scope, i);
        m_add_scope_button->setDisabled(m_available_scopes->get_size() == 0);
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
