#ifndef SPIRE_ORDERWRAPPERTASKNODE_HPP
#define SPIRE_ORDERWRAPPERTASKNODE_HPP
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class OrderWrapperTaskNode
      \brief Wraps an existing Order into a CanvasNode.
   */
  class OrderWrapperTaskNode : public CanvasNode {
    public:

      //! Constructs an OrderWrapperTaskNode.
      /*!
        \param order The Order to wrap.
        \param userProfile The user's profile.
      */
      OrderWrapperTaskNode(const Nexus::OrderExecutionService::Order& order,
        const UserProfile& userProfile);

      //! Constructs an OrderWrapperTaskNode with a specified display text.
      /*!
        \param order The Order to wrap.
        \param userProfile The user's profile.
        \param text The display text to use.
      */
      OrderWrapperTaskNode(const Nexus::OrderExecutionService::Order& order,
        const UserProfile& userProfile, std::string text);

      //! Returns the Order to be wrapped.
      const Nexus::OrderExecutionService::Order& GetOrder() const;

      //! Renames this CanvasNode.
      /*!
        \param name The name to give to the clone of this CanvasNode.
        \return A clone of this CanvasNode with the display Text set to the
                specified <i>name</i>.
      */
      std::unique_ptr<OrderWrapperTaskNode> Rename(std::string name) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      using CanvasNode::Replace;
    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      const Nexus::OrderExecutionService::Order* m_order;

      void Initialize(std::string text, const UserProfile& userProfile);
  };
}

#endif
