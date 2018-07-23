#ifndef SPIRE_TASKSTATETYPE_HPP
#define SPIRE_TASKSTATETYPE_HPP
#include <Beam/Tasks/Task.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class TaskStateType
      \brief Represents a Task's State type.
   */
  class TaskStateType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Beam::Tasks::Task::State Type;

      //! Returns an instance of this type.
      static const TaskStateType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      TaskStateType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TaskStateType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
