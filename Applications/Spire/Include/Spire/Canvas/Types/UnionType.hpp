#ifndef SPIRE_UNIONTYPE_HPP
#define SPIRE_UNIONTYPE_HPP
#include <memory>
#include <vector>
#include <Beam/Collections/View.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

namespace Spire {

  /*! \class UnionType
      \brief Represents a union of compatible types.
   */
  class UnionType : public CanvasType {
    public:

      //! Returns an empty UnionType.
      static const UnionType& GetEmptyType();

      //! Returns a UnionType representing any type.
      static const UnionType& GetAnyType();

      //! Returns a UnionType representing any value type.
      static const UnionType& GetAnyValueType();

      //! Constructs a type compatible with a list of NativeTypes.
      /*!
        \param compatibleTypes The list of NativeTypes to be compatible with.
        \return A CanvasType compatible with the list of specified types.
      */
      static std::shared_ptr<CanvasType> Create(
        const Beam::View<NativeType>& compatibleTypes);

      //! Constructs a named type compatible with a list of NativeTypes.
      /*!
        \param compatibleTypes The list of NativeTypes to be compatible with.
        \param name The name of the UnionType.
        \return A CanvasType compatible with the list of specified types.
      */
      static std::shared_ptr<CanvasType> Create(
        const Beam::View<NativeType>& compatibleTypes, std::string name);

      //! Returns the compatible types.
      Beam::View<NativeType> GetCompatibleTypes() const;

      virtual std::string GetName() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;
      std::vector<std::shared_ptr<NativeType>> m_compatibleTypes;
      std::string m_name;

      UnionType() = default;
      UnionType(std::vector<std::shared_ptr<NativeType>> compatibleTypes,
        std::string name);
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void UnionType::shuttle(S& shuttle, unsigned int version) {
    CanvasType::shuttle(shuttle, version);
    shuttle.shuttle("compatible_types", m_compatibleTypes);
    shuttle.shuttle("name", m_name);
  }
}

#endif
