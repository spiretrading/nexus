#include "WebPortal/ReportAccess.hpp"
#include <algorithm>

using namespace Beam;
using namespace Nexus;

ReportAccess::ReportAccess(
  const DirectoryEntry& account, ServiceLocatorClient client)
  : m_account(account),
    m_client(std::move(client)) {}

bool ReportAccess::is_accessible(const ReportJob& job) {
  if(m_account.m_type != DirectoryEntry::Type::ACCOUNT ||
      job.m_status != ReportJob::Status::COMPLETED) {
    return false;
  }
  if(job.m_account == m_account || std::ranges::find(
      job.m_recipients, m_account) != job.m_recipients.end()) {
    return true;
  }
  auto has_groups = std::ranges::any_of(job.m_recipients,
    [] (const auto& entry) {
      return entry.m_type == DirectoryEntry::Type::DIRECTORY &&
        entry != DirectoryEntry::STAR_DIRECTORY;
    });
  if(!has_groups) {
    return false;
  }
  if(!m_groups) {
    auto groups = std::unordered_set{m_account};
    auto pending = std::vector{m_account};
    while(!pending.empty()) {
      auto entry = pending.back();
      pending.pop_back();
      for(auto& parent : m_client.load_parents(entry)) {
        if(groups.insert(parent).second) {
          pending.push_back(parent);
        }
      }
    }
    m_groups = std::move(groups);
  }
  return std::ranges::any_of(job.m_recipients, [&] (const auto& entry) {
    return m_groups->contains(entry);
  });
}
