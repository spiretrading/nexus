#ifndef SPIRE_TASK_TYPE_HPP
#define SPIRE_TASK_TYPE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /** Represents a Task type. */
  class TaskType : public NativeType {
    public:

      //! Defines the native type being represented.
      using Type = Task;

      //! Returns an instance of this type.
      static const TaskType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      TaskType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TaskType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
