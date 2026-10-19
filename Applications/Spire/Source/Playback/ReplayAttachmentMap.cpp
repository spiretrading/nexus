#include "Spire/Playback/ReplayAttachmentMap.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/PropertyHubMember.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  class AttachedHubListModel : public PropertyHubListModel {
    public:
      AttachedHubListModel(std::shared_ptr<ReplayAttachmentMap> attachment_map,
          std::shared_ptr<ReplaySession> session)
          : m_attachment_map(std::move(attachment_map)),
            m_session(std::move(session)),
            m_hubs(m_attachment_map->get_hubs(*m_session)) {
        m_connection = m_attachment_map->connect_update_signal(
          std::bind_front(&AttachedHubListModel::on_update, this));
      }

      int get_size() const override {
        return static_cast<int>(m_hubs.size());
      }

      const Type& get(int index) const override {
        if(index < 0 || index >= get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        return m_hubs[index];
      }

      QValidator::State insert(const Type& value, int index) override {
        if(index < 0 || index > get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        if(!value) {
          return QValidator::State::Invalid;
        }
        if(std::ranges::find(m_hubs, value) != m_hubs.end()) {
          return QValidator::State::Acceptable;
        }
        m_pending = std::pair(value, index);
        m_attachment_map->attach(value, m_session);
        m_pending = none;
        return QValidator::State::Acceptable;
      }

      QValidator::State remove(int index) override {
        if(index < 0 || index >= get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        m_attachment_map->detach(m_hubs[index]);
        return QValidator::State::Acceptable;
      }

      connection connect_operation_signal(
          const OperationSignal::slot_type& slot) const override {
        return m_transaction.connect_operation_signal(slot);
      }

      using PropertyHubListModel::insert;
      using PropertyHubListModel::remove;
      using PropertyHubListModel::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override {
        m_transaction.transact(transaction);
      }

    private:
      std::shared_ptr<ReplayAttachmentMap> m_attachment_map;
      std::shared_ptr<ReplaySession> m_session;
      std::vector<std::shared_ptr<PropertyHub>> m_hubs;
      optional<std::pair<std::shared_ptr<PropertyHub>, int>> m_pending;
      ListModelTransactionLog<std::shared_ptr<PropertyHub>> m_transaction;
      scoped_connection m_connection;

      void on_update(const std::shared_ptr<PropertyHub>& hub,
          const std::shared_ptr<ReplaySession>& session) {
        auto i = std::ranges::find(m_hubs, hub);
        if(session == m_session) {
          if(i != m_hubs.end()) {
            return;
          }
          auto destination = [&] {
            if(m_pending && m_pending->first == hub) {
              return std::min(m_pending->second, get_size());
            }
            return get_size();
          }();
          m_hubs.insert(std::next(m_hubs.begin(), destination), hub);
          m_transaction.push(AddOperation(destination));
        } else if(i != m_hubs.end()) {
          auto index = static_cast<int>(i - m_hubs.begin());
          m_transaction.transact([&] {
            m_transaction.push(PreRemoveOperation(index));
            m_hubs.erase(std::next(m_hubs.begin(), index));
            m_transaction.push(RemoveOperation(index));
          });
        }
      }
  };

  class AttachedSessionModel : public ReplaySessionModel {
    public:
      AttachedSessionModel(std::shared_ptr<PropertyHubMember::HubModel> hub,
          std::shared_ptr<ReplayAttachmentMap> attachment_map)
          : m_hub(std::move(hub)),
            m_attachment_map(std::move(attachment_map)),
            m_value(evaluate()) {
        m_hub_connection = m_hub->connect_update_signal(
          std::bind_front(&AttachedSessionModel::on_hub, this));
        m_attachment_map_connection = m_attachment_map->connect_update_signal(
          std::bind_front(&AttachedSessionModel::on_attachment, this));
      }

      const std::shared_ptr<ReplaySession>& get() const override {
        return m_value;
      }

      connection connect_update_signal(
          const UpdateSignal::slot_type& slot) const override {
        return m_update_signal.connect(slot);
      }

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<PropertyHubMember::HubModel> m_hub;
      std::shared_ptr<ReplayAttachmentMap> m_attachment_map;
      std::shared_ptr<ReplaySession> m_value;
      scoped_connection m_hub_connection;
      scoped_connection m_attachment_map_connection;

      std::shared_ptr<ReplaySession> evaluate() const {
        return m_attachment_map->find(m_hub->get());
      }

      void update() {
        if(auto value = evaluate(); value != m_value) {
          m_value = std::move(value);
          m_update_signal(m_value);
        }
      }

      void on_hub(const std::shared_ptr<PropertyHub>& hub) {
        update();
      }

      void on_attachment(const std::shared_ptr<PropertyHub>& hub,
          const std::shared_ptr<ReplaySession>& session) {
        update();
      }
  };
}

std::shared_ptr<ReplaySession> ReplayAttachmentMap::find(
    const std::shared_ptr<PropertyHub>& hub) const {
  auto i = m_sessions.find(hub);
  if(i == m_sessions.end()) {
    return nullptr;
  }
  return i->second;
}

std::vector<std::shared_ptr<PropertyHub>> ReplayAttachmentMap::get_hubs(
    const ReplaySession& session) const {
  auto hubs = std::vector<std::shared_ptr<PropertyHub>>();
  for(auto& entry : m_sessions) {
    if(entry.second.get() == &session) {
      hubs.push_back(entry.first);
    }
  }
  return hubs;
}

void ReplayAttachmentMap::attach(std::shared_ptr<PropertyHub> hub,
    std::shared_ptr<ReplaySession> session) {
  if(!session) {
    detach(hub);
    return;
  }
  if(find(hub) == session) {
    return;
  }
  m_sessions[hub] = session;
  m_update_signal(hub, session);
}

void ReplayAttachmentMap::detach(const std::shared_ptr<PropertyHub>& hub) {
  auto i = m_sessions.find(hub);
  if(i == m_sessions.end()) {
    return;
  }
  auto detached = i->first;
  m_sessions.erase(i);
  m_update_signal(detached, nullptr);
}

connection ReplayAttachmentMap::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

std::shared_ptr<PropertyHubListModel> Spire::make_attached_hub_list(
    std::shared_ptr<ReplayAttachmentMap> attachment_map,
    std::shared_ptr<ReplaySession> session) {
  return std::make_shared<AttachedHubListModel>(
    std::move(attachment_map), std::move(session));
}

std::shared_ptr<ReplaySessionModel> Spire::make_attached_session_model(
    const PropertyHubMember& member,
    std::shared_ptr<ReplayAttachmentMap> attachment_map) {
  return std::make_shared<AttachedSessionModel>(
    member.get_hub(), std::move(attachment_map));
}
