#ifndef SPIRE_VALUENODE_HPP
#define SPIRE_VALUENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {
namespace Details {
  template<typename Shuttler>
  void Shuttle(Shuttler& shuttle, Nexus::Quantity& value) {
    if(Beam::IsReceiver<Shuttler>::value) {
      std::int64_t v;
      shuttle.Shuttle("value", v);
      value = v;
    } else {
      std::int64_t v = static_cast<std::int64_t>(value);
      shuttle.Shuttle("value", v);
    }
  }

  template<typename Shuttler>
  void Shuttle(Shuttler& shuttle, Nexus::Money& value) {
    if(Beam::IsReceiver<Shuttler>::value) {
      std::int64_t v;
      shuttle.Shuttle("value", v);
      value = Nexus::Money{Nexus::Quantity{v} / Nexus::Quantity::MULTIPLIER};
    } else {
      std::int64_t v = static_cast<std::int64_t>(
        static_cast<Nexus::Quantity>(value) * Nexus::Quantity::MULTIPLIER);
      shuttle.Shuttle("value", v);
    }
  }

  template<typename Shuttler, typename T>
  void Shuttle(Shuttler& shuttle, T& value) {
    shuttle.Shuttle("value", value);
  }
}

  /*! \class BaseValueNode
      \brief Base class for a ValueNode.
   */
  class BaseValueNode : public CanvasNode {};

  /*! \class ValueNode
      \brief Simple implementation of a CanvasNode for wrapping a value.
   */
  template<typename T>
  class ValueNode : public BaseValueNode {
    public:

      //! Defines the CanvasType this node stores.
      using Type = T;

      //! Constructs a ValueNode.
      ValueNode();

      //! Constructs a ValueNode.
      /*!
        \param value The initial value.
      */
      ValueNode(const typename Type::Type& value);

      //! Returns the stored value.
      const typename Type::Type& GetValue() const;

    protected:
      friend struct Beam::DataShuttle;

      //! Copes a ValueNode.
      /*!
        \param node The ValueNode to copy.
      */
      ValueNode(const ValueNode& node) = default;

      //! Sets the value.
      /*!
        \param value The value to store.
      */
      void SetInternalValue(const typename Type::Type& value);

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      typename Type::Type m_value;
  };

  template<typename T>
  ValueNode<T>::ValueNode()
      : m_value(typename Type::Type()) {
    SetType(Type::GetInstance());
  }

  template<typename T>
  ValueNode<T>::ValueNode(const typename Type::Type& value)
      : m_value(value) {
    SetType(Type::GetInstance());
  }

  template<typename T>
  const typename ValueNode<T>::Type::Type& ValueNode<T>::GetValue() const {
    return m_value;
  }

  template<typename T>
  void ValueNode<T>::SetInternalValue(const typename Type::Type& value) {
    m_value = value;
  }

  template<typename T>
  template<typename Shuttler>
  void ValueNode<T>::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    Details::Shuttle(shuttle, m_value);
  }
}

#endif
