#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

CanvasNodeTranslationContext::CanvasNodeTranslationContext(
  Ref<UserProfile> userProfile, Ref<Executor> executor,
  DirectoryEntry executingAccount)
  : m_parent(nullptr),
    m_userProfile(userProfile.Get()),
    m_executingAccount(std::move(executingAccount)),
    m_executor(executor.Get()),
    m_orderPublisher(std::make_shared<SequencePublisher<const Order*>>()) {}

CanvasNodeTranslationContext::CanvasNodeTranslationContext(
  Ref<CanvasNodeTranslationContext> parent)
  : m_parent(parent.Get()),
    m_userProfile(m_parent->m_userProfile),
    m_executingAccount(m_parent->m_executingAccount),
    m_executor(m_parent->m_executor),
    m_orderPublisher(m_parent->m_orderPublisher) {}

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

const DirectoryEntry&
    CanvasNodeTranslationContext::GetExecutingAccount() const {
  return m_executingAccount;
}

const Publisher<const Order*>&
    CanvasNodeTranslationContext::GetOrderPublisher() const {
  return *m_orderPublisher;
}

SequencePublisher<const Order*>&
    CanvasNodeTranslationContext::GetOrderPublisher() {
  return *m_orderPublisher;
}

void CanvasNodeTranslationContext::Add(
    Ref<const CanvasNode> node, const Translation& translation) {
  auto lock = lock_guard(m_mutex);
  m_translations.insert(std::make_pair(node.Get(), translation));
  if(m_parent) {
    m_parent->AddSubtranslation(Ref(node), translation);
  }
}

optional<Translation> CanvasNodeTranslationContext::FindTranslation(
    const CanvasNode& node) const {
  auto lock = lock_guard(m_mutex);
  auto translationIterator = m_translations.find(&node);
  if(translationIterator == m_translations.end()) {
    if(!m_parent) {
      return none;
    }
    return m_parent->FindTranslation(node);
  }
  return translationIterator->second;
}

optional<Translation> CanvasNodeTranslationContext::FindSubTranslation(
    const CanvasNode& node) const {
  auto lock = lock_guard(m_mutex);
  auto translationIterator = m_translations.find(&node);
  if(translationIterator == m_translations.end()) {
    auto subTranslationIterator = m_subTranslations.find(&node);
    if(subTranslationIterator == m_subTranslations.end()) {
      if(!m_parent) {
        return none;
      }
      return m_parent->FindSubTranslation(node);
    }
    return subTranslationIterator->second;
  }
  return translationIterator->second;
}

void CanvasNodeTranslationContext::AddSubtranslation(
    Beam::Ref<const CanvasNode> node, const Translation& translation) {
  auto lock = lock_guard(m_mutex);
  m_subTranslations.insert(std::pair(node.Get(), translation));
  if(m_parent) {
    m_parent->AddSubtranslation(Ref(node), translation);
  }
}

void Spire::Mirror(const CanvasNode& sourceNode,
    const CanvasNodeTranslationContext& sourceContext,
    const CanvasNode& mirrorNode,
    Out<CanvasNodeTranslationContext> mirrorContext) {
  if(auto translation = sourceContext.FindSubTranslation(sourceNode)) {
    mirrorContext->Add(Ref(mirrorNode), *translation);
  }
}
