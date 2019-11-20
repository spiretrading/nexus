#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include <boost/range/adaptor/map.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::adaptors;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

CanvasNodeTranslationContext::CanvasNodeTranslationContext(
    Ref<UserProfile> userProfile, Ref<Executor> executor,
    DirectoryEntry executingAccount)
    : m_parent(nullptr),
      m_userProfile(userProfile.Get()),
      m_executor(executor.Get()),
      m_executingAccount(std::move(executingAccount)),
      m_marketDataPublisher(std::make_unique<
        RealTimeMarketDataPublisher<VirtualMarketDataClient*>>(
        &m_userProfile->GetServiceClients().GetMarketDataClient())) {}

CanvasNodeTranslationContext::CanvasNodeTranslationContext(
    Ref<CanvasNodeTranslationContext> parent)
    : m_parent(parent.Get()),
      m_userProfile(m_parent->m_userProfile),
      m_executingAccount(m_parent->m_executingAccount),
      m_executor(m_parent->m_executor) {}

const UserProfile& CanvasNodeTranslationContext::GetUserProfile() const {
  return *m_userProfile;
}

UserProfile& CanvasNodeTranslationContext::GetUserProfile() {
  return *m_userProfile;
}

const Executor& CanvasNodeTranslationContext::GetExecutor() const {
  return *m_executor;
}

Executor& CanvasNodeTranslationContext::GetExecutor() {
  return *m_executor;
}

const DirectoryEntry& CanvasNodeTranslationContext::
    GetExecutingAccount() const {
  return m_executingAccount;
}

void CanvasNodeTranslationContext::Add(Ref<const CanvasNode> node,
    const Translation& translation) {
  boost::lock_guard<boost::mutex> lock(m_mutex);
  m_translations.insert(std::make_pair(node.Get(), translation));
#if 0 // TODO
  auto taskTranslation = get<const TaskTranslation>(&translation);
  if(taskTranslation != nullptr) {
    m_orderExecutionPublishers.insert(std::make_pair(node.Get(),
      taskTranslation->m_publisher));
  }
#endif
  if(m_parent != nullptr) {
    m_parent->AddSubtranslation(Ref(node), translation);
  }
}

boost::optional<Translation> CanvasNodeTranslationContext::FindTranslation(
    const CanvasNode& node) const {
  boost::lock_guard<boost::mutex> lock(m_mutex);
  auto translationIterator = m_translations.find(&node);
  if(translationIterator == m_translations.end()) {
    if(m_parent == nullptr) {
      return none;
    }
    return m_parent->FindTranslation(node);
  }
  return translationIterator->second;
}

boost::optional<Translation> CanvasNodeTranslationContext::FindSubTranslation(
    const CanvasNode& node) const {
  boost::lock_guard<boost::mutex> lock(m_mutex);
  auto translationIterator = m_translations.find(&node);
  if(translationIterator == m_translations.end()) {
    auto subTranslationIterator = m_subTranslations.find(&node);
    if(subTranslationIterator == m_subTranslations.end()) {
      if(m_parent == nullptr) {
        return none;
      }
      return m_parent->FindSubTranslation(node);
    }
    return subTranslationIterator->second;
  }
  return translationIterator->second;
}

std::shared_ptr<OrderExecutionPublisher>
    CanvasNodeTranslationContext::FindOrderExecutionPublisher(
    const CanvasNode& node) const {
  boost::lock_guard<boost::mutex> lock(m_mutex);
  auto publisherIterator = m_orderExecutionPublishers.find(&node);
  if(publisherIterator == m_orderExecutionPublishers.end()) {
    if(m_parent == nullptr) {
      return nullptr;
    }
    return m_parent->FindOrderExecutionPublisher(node);
  }
  return publisherIterator->second;
}

RealTimeMarketDataPublisher<VirtualMarketDataClient*>&
    CanvasNodeTranslationContext::GetRealTimeMarketDataPublisher() const {
  if(m_parent == nullptr) {
    return *m_marketDataPublisher;
  }
  return m_parent->GetRealTimeMarketDataPublisher();
}

void CanvasNodeTranslationContext::AddSubtranslation(
    Beam::Ref<const CanvasNode> node, const Translation& translation) {
  boost::lock_guard<boost::mutex> lock(m_mutex);
  m_subTranslations.insert(std::make_pair(node.Get(), translation));
  if(m_parent != nullptr) {
    m_parent->AddSubtranslation(Ref(node), translation);
  }
}

void Spire::Mirror(const CanvasNode& sourceNode,
    const CanvasNodeTranslationContext& sourceContext,
    const CanvasNode& mirrorNode,
    Out<CanvasNodeTranslationContext> mirrorContext) {
  auto translation = sourceContext.FindSubTranslation(sourceNode);
  if(translation.is_initialized()) {
    mirrorContext->Add(Ref(mirrorNode), *translation);
  }
}
