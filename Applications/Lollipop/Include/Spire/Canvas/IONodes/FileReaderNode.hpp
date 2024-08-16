#ifndef SPIRE_FILEREADERNODE_HPP
#define SPIRE_FILEREADERNODE_HPP
#include <Beam/Parsers/ParserPublisher.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class FileReaderNode
      \brief A CanvasNode that parses data from a file.
   */
  class FileReaderNode : public CanvasNode {
    public:

      //! Constructs a FileReaderNode.
      FileReaderNode();

      //! Constructs a FileReaderNode.
      /*!
        \param errorPolicy The error policy.
      */
      FileReaderNode(Beam::Parsers::ParserErrorPolicy errorPolicy);

      //! Constructs a FileReaderNode.
      /*!
        \param readType The type of data to read from the file.
      */
      FileReaderNode(const NativeType& readType);

      //! Constructs a FileReaderNode.
      /*!
        \param errorPolicy The Expression's error policy.
        \param readType The type of data to read from the file.
      */
      FileReaderNode(Beam::Parsers::ParserErrorPolicy errorPolicy,
        const NativeType& readType);

      //! Returns the error policy.
      Beam::Parsers::ParserErrorPolicy GetErrorPolicy() const;

      //! Sets error policy.
      /*!
        \param errorPolicy The error policy to use.
        \return A clone of this CanvasNode with the specified
                <i>errorPolicy</i>.
      */
      std::unique_ptr<FileReaderNode> SetErrorPolicy(
        Beam::Parsers::ParserErrorPolicy errorPolicy) const;

      //! Returns the type of data to read from the file.
      boost::optional<const NativeType&> GetReadType() const;

      //! Sets the type of data to read from.
      /*!
        \param type The type of data to read from the file.
        \return A FileReaderNode that reads data of the specified <i>type</i>.
      */
      std::unique_ptr<FileReaderNode> SetReadType(const NativeType& type) const;

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Beam::Parsers::ParserErrorPolicy m_errorPolicy;
      std::shared_ptr<NativeType> m_readType;

      FileReaderNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FileReaderNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("read_type", m_readType);
    shuttle.Shuttle("error_policy", m_errorPolicy);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FileReaderNode> : std::false_type {};
}
}

#endif
