#include "Spire/Ui/ScopeBox.hpp"
#include <boost/iterator/counting_iterator.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScopeListItem.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::iterators;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::vector<Scope> to_tag_list(const Scope& scope, ScopeQueryModel& scopes) {
    auto tags = std::vector<Scope>();
    for(auto& country : scope.get_countries()) {
      tags.push_back(*scopes.parse(to_text(country)));
    }
    for(auto& venue : scope.get_venues()) {
      tags.push_back(*scopes.parse(to_text(venue)));
    }
    for(auto& ticker : scope.get_tickers()) {
      tags.push_back(*scopes.parse(to_text(ticker)));
    }
    return tags;
  }

  void sort(ListModel<Scope>& list) {
    enum class EntryType : std::uint8_t {
      COUNTRY,
      VENUE,
      TICKER,
      NONE
    };
    struct Entry {
      int m_index;
      EntryType m_type;
      QString m_key;

      Entry()
        : m_index(0),
          m_type(EntryType::NONE) {
      }
    };
    auto size = list.get_size();
    if(size <= 1) {
      return;
    }
    auto entries = std::vector<Entry>();
    entries.reserve(size);
    for(auto i = 0; i < size; ++i) {
      auto entry = Entry();
      entry.m_index = i;
      auto& scope = list.get(i);
      auto& countries = scope.get_countries();
      auto& venues = scope.get_venues();
      auto& tickers = scope.get_tickers();
      if(!countries.empty()) {
        entry.m_type = EntryType::COUNTRY;
        entry.m_key = to_text(*countries.begin());
      } else if(!venues.empty()) {
        entry.m_type = EntryType::VENUE;
        entry.m_key = to_text(*venues.begin());
      } else if(!tickers.empty()) {
        entry.m_type = EntryType::TICKER;
        entry.m_key = to_text(*tickers.begin());
      } else {
        entry.m_type = EntryType::NONE;
      }
      entries.emplace_back(std::move(entry));
    }
    std::stable_sort(entries.begin(), entries.end(),
      [] (const auto& a, const auto& b) {
        if(a.m_type != b.m_type) {
          return a.m_type < b.m_type;
        }
        return a.m_key < b.m_key;
      });
    auto current_pos = std::vector<int>(size);
    std::iota(current_pos.begin(), current_pos.end(), 0);
    auto placed = std::vector<bool>(size, false);
    list.transact([&] {
      for(auto i = 0; i < size; ++i) {
        auto original = entries[i].m_index;
        if(placed[original]) {
          continue;
        }
        auto from = current_pos[original];
        auto to = i;
        if(from != to) {
          list.move(from, to);
          for(auto j = 0; j < size; ++j) {
            auto& pos = current_pos[j];
            if(pos >= to && pos < from) {
              ++pos;
            } else if(pos <= to && pos > from) {
              --pos;
            }
          }
          current_pos[original] = to;
        }
        placed[original] = true;
      }
    });
  }
}

ScopeBox::ScopeBox(std::shared_ptr<ScopeQueryModel> scopes, QWidget* parent)
  : ScopeBox(std::move(scopes), std::make_shared<LocalScopeModel>(), parent) {}

ScopeBox::ScopeBox(std::shared_ptr<ScopeQueryModel> scopes,
    std::shared_ptr<ScopeModel> current, QWidget* parent)
    : QWidget(parent),
      m_scopes(std::move(scopes)),
      m_current(std::move(current)),
      m_last_scope(m_current->get()),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&ScopeBox::on_current, this))) {
  auto current_model = std::make_shared<ArrayListModel<Scope>>();
  auto tags = to_tag_list(m_current->get(), *m_scopes);
  current_model->transact([&] {
    for(auto& tag : tags) {
      current_model->push(tag);
    }
    sort(*current_model);
  });
  m_tag_combo_box = new TagComboBox(m_scopes, std::move(current_model),
    [] (const auto& list, auto index) {
      return new ScopeListItem(list->get(index));
    });
  m_tag_combo_box->connect_submit_signal(
    std::bind_front(&ScopeBox::on_submit, this));
  m_tag_operation_connection =
    m_tag_combo_box->get_current()->connect_operation_signal(
      std::bind_front(&ScopeBox::on_tags_operation, this));
  m_tag_combo_box->installEventFilter(this);
  enclose(*this, *m_tag_combo_box);
  proxy_style(*this, *m_tag_combo_box);
  setFocusProxy(m_tag_combo_box);
}

const std::shared_ptr<ScopeQueryModel>& ScopeBox::get_scopes() const {
  return m_scopes;
}

const std::shared_ptr<ScopeModel>& ScopeBox::get_current() const {
  return m_current;
}

void ScopeBox::set_placeholder(const QString& placeholder) {
  m_tag_combo_box->set_placeholder(placeholder);
}

bool ScopeBox::is_read_only() const {
  return m_tag_combo_box->is_read_only();
}

void ScopeBox::set_read_only(bool is_read_only) {
  m_tag_combo_box->set_read_only(is_read_only);
}

connection ScopeBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void ScopeBox::on_current(const Scope& scope) {
  if(scope == m_last_scope) {
    return;
  }
  m_last_scope = scope;
  auto scopes = to_tag_list(scope, *m_scopes);
  auto blocker = shared_connection_block(m_tag_operation_connection);
  auto current = m_tag_combo_box->get_current();
  current->transact([&] {
    while(current->get_size() > scopes.size()) {
      current->remove(current->get_size() - 1);
    }
    auto i = std::size_t(0);
    while(i < current->get_size() && i < scopes.size()) {
      if(current->get(i) != scopes[i]) {
        current->set(i, scopes[i]);
      }
      ++i;
    }
    while(i < scopes.size()) {
      current->push(scopes[i]);
      ++i;
    }
    sort(*current);
  });
  invalidate_descendant_layouts(*this);
  adjustSize();
}

void ScopeBox::on_submit(const std::shared_ptr<ListModel<Scope>>& submission) {
  sort(*m_tag_combo_box->get_current());
  sort(*submission);
  auto scope = Nexus::Scope();
  for(auto r : *submission) {
    scope += r;
  }
  m_submit_signal(scope);
}

void ScopeBox::on_tags_operation(const AnyListModel::Operation& operation) {
  auto update_current = [&] {
    auto scope = Nexus::Scope();
    for(auto r : *m_tag_combo_box->get_current()) {
      scope += r;
    }
    auto blocker = shared_connection_block(m_current_connection);
    if(m_current->set(scope) != QValidator::Invalid) {
      m_last_scope = std::move(scope);
    }
  };
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      update_current();
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      update_current();
    });
}
