#ifndef SPIRE_LUASCRIPTNODE_HPP
#define SPIRE_LUASCRIPTNODE_HPP
#include <filesystem>
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleFileSystemPath.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class LuaScriptNode
      \brief A CanvasNode that executes a Lua script.
   */
  class LuaScriptNode : public CanvasNode {
    public:

      /*! \struct Parameter
          \brief Stores information about the script's parameters.
       */
      struct Parameter {

        //! The name of the parameter.
        std::string m_name;

        //! The parameter's type.
        std::shared_ptr<NativeType> m_type;

        //! Constructs an empty Parameter.
        Parameter();

        //! Constructs a Parameter.
        /*!
          \param name The name of the parameter.
          \param type The parameter's type.
        */
        Parameter(std::string name, const NativeType& type);

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      //! Constructs an uninitialized LuaScriptNode.
      LuaScriptNode();

      //! Constructs a LuaScriptNode.
      /*!
        \param name The name of this node.
        \param type The NativeType that the script evaluates to.
        \param path The path to the script to execute.
        \param parameters The parameters to pass to the script.
      */
      LuaScriptNode(std::string name, const NativeType& type,
        const std::filesystem::path& path, std::vector<Parameter> parameters);

      //! Returns the name.
      const std::string& GetName() const;

      //! Returns the path.
      const std::filesystem::path& GetPath() const;

      //! Returns the parameters.
      const std::vector<Parameter>& GetParameters() const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_name;
      std::filesystem::path m_path;
      std::vector<Parameter> m_parameters;

      LuaScriptNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LuaScriptNode::Parameter::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("type", m_type);
  }

  template<typename Shuttler>
  void LuaScriptNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("path", m_path);
    shuttle.Shuttle("parameters", m_parameters);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::LuaScriptNode> : std::false_type {};
}
}

#endif
