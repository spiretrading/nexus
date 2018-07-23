#ifndef SPIRE_CANVASTYPE_HPP
#define SPIRE_CANVASTYPE_HPP
#include <memory>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/noncopyable.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {
namespace Details {
  BEAM_ENUM(CanvasTypeCompatibilityDefinition,

    //! The type is compatible/substitutable.
    COMPATIBLE,

    //! The two types are equal.
    EQUAL);
}

  /*! \class CanvasType
      \brief Stores information about a CanvasNode's data type.
   */
  class CanvasType : public std::enable_shared_from_this<CanvasType>,
      private boost::noncopyable {
    public:

      //! Enumerates types of compatibilities between CanvasTypes.
      using Compatibility = Details::CanvasTypeCompatibilityDefinition;

      //! Returns the shared_ptr managing this CanvasType.
      operator std::shared_ptr<CanvasType> () const;

      virtual ~CanvasType() = default;

      //! Returns the name of this type.
      virtual std::string GetName() const = 0;

      //! Tests the degree to which another CanvasType can be substituted for
      //! this type.
      /*!
        \param type The type to check for compatibility/substitution.
        \return The Compatibility from <i>type</i> to <i>this</i>.
      */
      virtual Compatibility GetCompatibility(const CanvasType& type) const = 0;

      //! Applies a CanvasTypeVisitor to this instance.
      /*!
        \param visitor The CanvasTypeVisitor to apply.
      */
      virtual void Apply(CanvasTypeVisitor& visitor) const = 0;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CanvasType::Shuttle(Shuttler& shuttle, unsigned int version) {}

  //! Tests if a value represents CanvasType compatibility.
  /*!
    \param compatibility The Compatibility to test.
    \return <code>true</code> iff <i>compatibility</i> is COMPATIBLE or EQUAL.
  */
  bool IsCompatible(CanvasType::Compatibility compatibility);

  //! Tests if two CanvasTypes are compatible.
  /*!
    \param a One of the CanvasTypes to test.
    \param b The CanvasType to substitute in place of <i>a</i>.
    \return <code>true</code> iff <i>b</i> can be substituted for <i>a</i>.
  */
  bool IsCompatible(const CanvasType& a, const CanvasType& b);
}

#endif
