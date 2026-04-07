#ifndef SPIRE_FUNCTION_NODE_HPP
#define SPIRE_FUNCTION_NODE_HPP
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /** A CanvasNode that applies a function to its children. */
  class FunctionNode : public SignatureNode {
    protected:

      /** Constructs a FunctionNode. */
      FunctionNode() = default;

      /**
       * Defines a FunctionNode.
       * @param name The name of the function.
       * @param parameterNames The name of each parameter.
       * @param signatures The available signatures for this function.
       */
      void DefineFunction(std::string name,
        std::vector<std::string> parameterNames,
        std::vector<Signature> signatures);

      const std::vector<Signature>& GetSignatures() const override;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

      using SignatureNode::Replace;
    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      std::vector<Signature> m_signatures;
  };

  template<Beam::IsShuttle S>
  void FunctionNode::shuttle(S& shuttle, unsigned int version) {
    SignatureNode::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("signatures", m_signatures);
  }
}

#endif
