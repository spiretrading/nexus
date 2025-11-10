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

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
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
      friend struct Beam::DataShuttle;
      std::string m_name;
      std::filesystem::path m_path;
      std::vector<Parameter> m_parameters;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LuaScriptNode::Parameter::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("type", m_type);
  }

  template<Beam::IsShuttle S>
  void LuaScriptNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("path", m_path);
    shuttle.shuttle("parameters", m_parameters);
  }
}

#endif
