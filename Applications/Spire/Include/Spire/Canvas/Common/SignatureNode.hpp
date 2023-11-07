#ifndef SPIRE_SIGNATURE_NODE_HPP
#define SPIRE_SIGNATURE_NODE_HPP
#include <vector>
#include <boost/mpl/for_each.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {
namespace Details {
  struct SignatureInnerLoop {
    const CanvasTypeRegistry* m_typeRegistry;
    std::vector<std::shared_ptr<NativeType>>* m_signature;

    SignatureInnerLoop(const CanvasTypeRegistry& typeRegistry,
      std::vector<std::shared_ptr<NativeType>>& signature)
      : m_typeRegistry(&typeRegistry),
        m_signature(&signature) {}

    template<typename T>
    void operator ()(T) {
      m_signature->push_back(*m_typeRegistry->Find(typeid(T)));
    }
  };

  struct SignatureOuterLoop {
    CanvasTypeRegistry m_typeRegistry;
    std::vector<std::vector<std::shared_ptr<NativeType>>>* m_signatures;

    SignatureOuterLoop(
      std::vector<std::vector<std::shared_ptr<NativeType>>>& signatures)
      : m_signatures(&signatures) {}

    template<typename T>
    void operator ()(T) {
      auto signature = std::vector<std::shared_ptr<NativeType>>();
      boost::mpl::for_each<T>(
        Details::SignatureInnerLoop(m_typeRegistry, signature));
      m_signatures->push_back(std::move(signature));
    }
  };
}

  /**
   * Base class for a CanvasNode with a finite set of parameter/return type
   * signatures.
   */
  class SignatureNode : public CanvasNode {
    public:

      /**
       * Defines the type of a single signature.  The vector must be non-empty
       * and contain as its last element the signature's return type.
       */
      using Signature = std::vector<std::shared_ptr<NativeType>>;

      std::unique_ptr<CanvasNode>
        Convert(const CanvasType& type) const override;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      using CanvasNode::Replace;
    protected:

      /** Constructs an empty SignatureNode. */
      SignatureNode();

      /** Copies a SignatureNode. */
      SignatureNode(const SignatureNode& node) = default;

      /** Returns the signatures supported by this CanvasNode. */
      virtual const std::vector<Signature>& GetSignatures() const = 0;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      std::shared_ptr<CanvasType> m_type;
  };

  /** Returns a list of signatures from a compile time list of types. */
  template<typename Signatures>
  std::vector<SignatureNode::Signature> MakeSignatures() {
    auto signatures = std::vector<SignatureNode::Signature>();
    boost::mpl::for_each<typename Signatures::type>(
      Details::SignatureOuterLoop(signatures));
    return signatures;
  }

  template<typename Shuttler>
  void SignatureNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("type", m_type);
  }
}

#endif
