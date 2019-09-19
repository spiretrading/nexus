#ifndef SPIRE_CANVASNODETRANSLATIONCONTEXT_HPP
#define SPIRE_CANVASNODETRANSLATIONCONTEXT_HPP
#include <unordered_map>
#include <Aspen/Aspen.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/Reactors.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/MarketDataService/RealTimeMarketDataPublisher.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/TaskNodes/Task.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \struct CanvasNodeTranslationContext
      \brief Stores all the context needed to translate a CanvasNode.
   */
  class CanvasNodeTranslationContext : private boost::noncopyable {
    public:

      //! Constructs a CanvasNodeTranslationContext.
      /*!
        \param userProfile The user's profile.
        \param reactorMonitor The ReactorMonitor.
        \param executingAccount The account used to execute Orders.
      */
      CanvasNodeTranslationContext(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<ReactorMonitor> reactorMonitor,
        const Beam::ServiceLocator::DirectoryEntry& executingAccount);

      //! Constructs a CanvasNodeTranslationContext from a parent context.
      /*!
        \param parent The parent CanvasNodeTranslationContext.
      */
      CanvasNodeTranslationContext(
        Beam::Ref<CanvasNodeTranslationContext> parent);

      //! Returns the UserProfile.
      const UserProfile& GetUserProfile() const;

      //! Returns the UserProfile.
      UserProfile& GetUserProfile();

      //! Returns the ReactorMonitor.
      const ReactorMonitor& GetReactorMonitor() const;

      //! Returns the ReactorMonitor.
      ReactorMonitor& GetReactorMonitor();

      //! Returns the executing account.
      const Beam::ServiceLocator::DirectoryEntry& GetExecutingAccount() const;

      //! Associates a Translation with a CanvasNode.
      /*!
        \param node The CanvasNode that was translated.
        \param translation The Translation to associate with the <i>node</i>.
      */
      void Add(Beam::Ref<const CanvasNode> node,
        const Translation& translation);

      //! Finds a Translation.
      /*!
        \param node The CanvasNode to find the Translation for.
        \return The specified <i>node</i>'s Translation.
      */
      boost::optional<Translation> FindTranslation(
        const CanvasNode& node) const;

      //! Finds a Translation that was either added directly or to a child node.
      /*!
        \param node The CanvasNode to find the Translation for.
        \return The specified <i>node</i>'s Translation.
      */
      boost::optional<Translation> FindSubTranslation(
        const CanvasNode& node) const;

      //! Finds an OrderExecutionPublisher.
      /*!
        \param node The CanvasNode to find the OrderExecutionPublisher for.
        \return The publisher for the <i>node</i> or <code>nullptr</code> iff no
                such publisher exists.
      */
      std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
        FindOrderExecutionPublisher(const CanvasNode& node) const;

      //! Returns the MarketDataPublisher used for real time data.
      Nexus::MarketDataService::RealTimeMarketDataPublisher<
        Nexus::MarketDataService::VirtualMarketDataClient*>&
        GetRealTimeMarketDataPublisher() const;

    private:
      mutable boost::mutex m_mutex;
      CanvasNodeTranslationContext* m_parent;
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_executingAccount;
      ReactorMonitor* m_reactorMonitor;
      std::unordered_map<const CanvasNode*, Translation> m_translations;
      std::unordered_map<const CanvasNode*, Translation> m_subTranslations;
      std::unordered_map<const CanvasNode*,
        std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>>
        m_orderExecutionPublishers;
      std::unique_ptr<Nexus::MarketDataService::RealTimeMarketDataPublisher<
        Nexus::MarketDataService::VirtualMarketDataClient*>>
        m_marketDataPublisher;

      void AddSubtranslation(Beam::Ref<const CanvasNode> node,
        const Translation& translation);
  };

  //! Mirrors the translation of a node in one context, into a node of another
  //! context.
  /*!
    \param sourceNode The node previously translated and to be mirrored.
    \param sourceContext The context containing the <i>sourceNode</i>'s
           translations.
    \param mirrorNode The node to duplicate the translation into.
    \param mirrorContext The context to store the mirrored translation.
  */
  void Mirror(const CanvasNode& sourceNode,
    const CanvasNodeTranslationContext& sourceContext,
    const CanvasNode& mirrorNode,
    Beam::Out<CanvasNodeTranslationContext> mirrorContext);
}

#endif
