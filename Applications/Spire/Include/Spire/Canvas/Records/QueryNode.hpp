#ifndef SPIRE_QUERYNODE_HPP
#define SPIRE_QUERYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class QueryNode
      \brief Queries a field in a Record.
   */
  class QueryNode : public CanvasNode {
    public:

      //! Constructs a QueryNode.
      QueryNode();

      //! Returns the field being queried.
      const std::string& GetField() const;

      //! Returns a new QueryNode querying a specified field.
      /*!
        \param field The field to query.
        \return A clone of this node querying the specified <i>field</i>.
      */
      std::unique_ptr<QueryNode> SetField(const std::string& field) const;

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_field;
      std::shared_ptr<CanvasType> m_type;

      QueryNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void QueryNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("field", m_field);
    shuttle.Shuttle("type", m_type);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::QueryNode> : std::false_type {};
}
}

#endif
