#ifndef SPIRE_CANVAS_NODE_TRANSLATION_CONTEXT_HPP
#define SPIRE_CANVAS_NODE_TRANSLATION_CONTEXT_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Stores all the context needed to translate a CanvasNode. */
  class CanvasNodeTranslationContext : private boost::noncopyable {
    public:

      /**
       * Constructs a CanvasNodeTranslationContext.
       * @param userProfile The user's profile.
       * @param executor The executor used on the translated reactor.
       * @param executingAccount The account used to execute Orders.
       */
      CanvasNodeTranslationContext(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<Executor> executor,
        Beam::DirectoryEntry executingAccount);

      /**
       * Constructs a CanvasNodeTranslationContext from a parent context.
       * @param parent The parent CanvasNodeTranslationContext.
       */
      CanvasNodeTranslationContext(
        Beam::Ref<CanvasNodeTranslationContext> parent);

      /** Returns the UserProfile. */
      const UserProfile& GetUserProfile() const;

      /** Returns the UserProfile. */
      UserProfile& GetUserProfile();

      /** Returns the executor. */
      const Executor& GetExecutor() const;

      /** Returns the executor. */
      Executor& GetExecutor();

      /** Returns the executing account. */
      const Beam::DirectoryEntry& GetExecutingAccount() const;

      /**
       * Returns the publisher for all orders submitted by the translated
       * node.
       */
      const Beam::Publisher<std::shared_ptr<Nexus::Order>>&
        GetOrderPublisher() const;

      /**
       * Returns the publisher for all orders submitted by the translated
       * node.
       */
      Beam::SequencePublisher<std::shared_ptr<Nexus::Order>>&
        GetOrderPublisher();

      /**
       * Associates a Translation with a CanvasNode.
       * @param node The CanvasNode that was translated.
       * @param translation The Translation to associate with the <i>node</i>.
       */
      void Add(Beam::Ref<const CanvasNode> node,
        const Translation& translation);

      /**
       * Finds a Translation.
       * @param node The CanvasNode to find the Translation for.
       * @return The specified <i>node</i>'s Translation.
       */
      boost::optional<Translation> FindTranslation(
        const CanvasNode& node) const;

      /**
       * Finds a Translation that was either added directly or to a child node.
       * @param node The CanvasNode to find the Translation for.
       * @return The specified <i>node</i>'s Translation.
       */
      boost::optional<Translation> FindSubTranslation(
        const CanvasNode& node) const;

    private:
      mutable boost::mutex m_mutex;
      CanvasNodeTranslationContext* m_parent;
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_executingAccount;
      Executor* m_executor;
      std::shared_ptr<Beam::SequencePublisher<std::shared_ptr<Nexus::Order>>>
        m_orderPublisher;
      std::unordered_map<const CanvasNode*, Translation> m_translations;
      std::unordered_map<const CanvasNode*, Translation> m_subTranslations;

      void AddSubtranslation(Beam::Ref<const CanvasNode> node,
        const Translation& translation);
  };

  /**
   * Mirrors the translation of a node in one context, into a node of another
   * context.
   * @param sourceNode The node previously translated and to be mirrored.
   * @param sourceContext The context containing the <i>sourceNode</i>'s
   *        translations.
   * @param mirrorNode The node to duplicate the translation into.
   * @param mirrorContext The context to store the mirrored translation.
   */
  void Mirror(const CanvasNode& sourceNode,
    const CanvasNodeTranslationContext& sourceContext,
    const CanvasNode& mirrorNode,
    Beam::Out<CanvasNodeTranslationContext> mirrorContext);
}

#endif
