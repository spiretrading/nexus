#ifndef SPIRE_SINGLEORDERTASKNODE_HPP
#define SPIRE_SINGLEORDERTASKNODE_HPP
#include <string>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class SingleOrderTaskNode
      \brief The CanvasNode representation of a SingleOrderTask.
   */
  class SingleOrderTaskNode : public CanvasNode {
    public:

      /** The name of the property used to specify the Ticker. */
      static const std::string TICKER_PROPERTY;

      /** The name of the property used to specify the OrderType. */
      static const std::string ORDER_TYPE_PROPERTY;

      /** The name of the property used to specify the Side. */
      static const std::string SIDE_PROPERTY;

      /** The name of the property used to specify the destination. */
      static const std::string DESTINATION_PROPERTY;

      /** The name of the property used to specify the price. */
      static const std::string PRICE_PROPERTY;

      /** The name of the property used to specify the Quantity. */
      static const std::string QUANTITY_PROPERTY;

      /** The name of the property used to specify the currency. */
      static const std::string CURRENCY_PROPERTY;

      /** The name of the property used to specify the TimeInForce. */
      static const std::string TIME_IN_FORCE_PROPERTY;

      /*! \struct FieldEntry
          \brief Stores the details of an additional field used to submit an
                 Order.
       */
      struct FieldEntry {

        //! The name of the field.
        std::string m_name;

        //! The field's type.
        std::shared_ptr<NativeType> m_type;

        //! The field's key.
        int m_key;

        //! Constructs an uninitialized FieldEntry.
        FieldEntry();

        //! Constructs a FieldEntry.
        /*!
          \param name The name of the field.
          \param type The field's type.
          \param key The field's key.
        */
        FieldEntry(std::string name, const NativeType& type, int key);
      };

      //! Constructs a SingleOrderTaskNode.
      SingleOrderTaskNode();

      //! Constructs a SingleOrderTaskNode with a specified display text.
      /*!
        \param text The display text to use.
      */
      SingleOrderTaskNode(std::string text);

      //! Returns the list of additional fields.
      const std::vector<FieldEntry>& GetFields() const;

      //! Adds an additional field to the Order submitted by this Task.
      /*!
        \param name The name of the field to add.
        \param key The field's key.
        \param value The field's default value.
      */
      std::unique_ptr<SingleOrderTaskNode> AddField(std::string name, int key,
        std::unique_ptr<CanvasNode> value) const;

      //! Renames this CanvasNode.
      /*!
        \param name The name to give to the clone of this CanvasNode.
        \return A clone of this CanvasNode with the display Text set to the
                specified <i>name</i>.
      */
      std::unique_ptr<SingleOrderTaskNode> Rename(std::string name) const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      using CanvasNode::Replace;
    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;
      std::vector<FieldEntry> m_fields;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SingleOrderTaskNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
    shuttle.shuttle("fields", m_fields);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Spire::SingleOrderTaskNode::FieldEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::SingleOrderTaskNode::FieldEntry& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("key", value.m_key);
    }
  };
}

#endif
