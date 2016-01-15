#ifndef SPIRE_CANVASNODEMODEL_HPP
#define SPIRE_CANVASNODEMODEL_HPP
#include <memory>
#include <tuple>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class CanvasNodeModel
      \brief Interface for a widget displaying CanvasNodes.
   */
  class CanvasNodeModel {
    public:

      /*! \struct Identity
          \brief Stores a CanvasNode's identity within a CanvasNodeModel.
       */
      struct Identity {

        //! The id of the CanvasNodeModel.
        int m_modelId;

        //! The id of the CanvasNode.
        int m_canvasNodeId;

        //! Constructs an uninitialized Identity.
        Identity();

        //! Constructs an Identity.
        /*!
          \param modelId The id of the CanvasNodeModel.
          \param canvasNodeId The id of the CanvasNode.
        */
        Identity(int modelId, int canvasNodeId);
      };

      /*! \struct Coordinate
          \brief Stores a CanvasNodeModel coordinate.
       */
      struct Coordinate {

        //! The Coordinate's row.
        int m_row;

        //! The Coordinate's column.
        int m_column;

        //! Constructs a Coordinate.
        Coordinate() = default;

        //! Constructs a Coordinate.
        /*!
          \param row The Coordinate's row.
          \param column The Coordinate's column.
        */
        Coordinate(int row, int column);

        //! Compares two Coordinates based on row first, then column.
        bool operator <(const Coordinate& coordinate) const;

        //! Tests if two Coordinates are equal.
        /*!
          \param coordinate The Coordinate to test for equality.
          \return <code>true</code> iff the Coordinates have equal rows and
                  columns.
        */
        bool operator ==(const Coordinate& coordinate) const;

        //! Tests if two Coordinates are not equal.
        /*!
          \param coordinate The Coordinate to test for inequality.
          \return <code>true</code> iff the Coordinates have different rows or
                  columns.
        */
        bool operator !=(const Coordinate& coordinate) const;
      };

      /*! \class Snapshot
          \brief Saves a snapshot of a CanvasNodeModel that can be restored
                 later.
       */
      class Snapshot {
        public:

          //! Constructs a Snapshot.
          Snapshot() = default;

          //! Saves a Snapshot of a CanvasNodeModel.
          /*!
            \param model The CanvasNodeModel to save.
          */
          void Save(const CanvasNodeModel& model);

          //! Restores a CanvasNodeModel to this Snapshot.
          /*!
            \param model The CanvasNodeModel to restore this Snapshot to.
          */
          void Restore(Beam::Out<CanvasNodeModel> model);

        private:
          std::vector<std::tuple<Coordinate, std::unique_ptr<CanvasNode>>>
            m_roots;
      };

      //! Meta-data used to keep track of changes to a CanvasNode.
      static std::string GetIdentityKey();

      //! Finds a CanvasNode's Identity.
      /*!
        \param node The CanvasNode whose Identity is to be looked up.
      */
      static boost::optional<Identity> FindIdentity(const CanvasNode& node);

      //! Returns the meta-data representing an Identity.
      /*!
        \param identity The Identity to convert into meta-data.
        \return The string representation of the <i>identity</i>.
      */
      static std::string ToMetaData(const Identity& identity);

      //! Meta-data used to determine which CanvasNode a ReferenceNode refers
      //! to.
      static std::string GetReferentKey();

      //! Finds a ReferenceNode's referent via it's identity.
      /*!
        \param node The ReferenceNode whose referent is to be found.
      */
      static boost::optional<const CanvasNode&> FindReferent(
        const ReferenceNode& node);

      //! Strips all CanvasNodes of their identity meta-data.
      /*!
        \param node The root of the CanvasNode whose identity is to be striped.
        \return A clone of the <i>node</i> with all identity meta-data striped.
      */
      static std::unique_ptr<CanvasNode> StripIdentity(const CanvasNode& node);

      virtual ~CanvasNodeModel() = default;

      //! Returns all root CanvasNodes.
      virtual std::vector<const CanvasNode*> GetRoots() const = 0;

      //! Returns the CanvasNode at a specified location.
      /*!
        \param coordinate The Coordinate to retrieve the CanvasNode from.
        \return The CanvasNode at the specified <i>coordinate</i>.
      */
      virtual boost::optional<const CanvasNode&> GetNode(
        const Coordinate& coordinate) const = 0;

      //! Returns the Coordinate of a CanvasNode.
      /*!
        \param node The CanvasNode whose Coordinate is to be returned.
        \return The Coordinate of the specified <i>node</i>.
      */
      virtual Coordinate GetCoordinate(const CanvasNode& node) const = 0;

      //! Returns the currently selected CanvasNode.
      virtual boost::optional<const CanvasNode&> GetCurrentNode() const;

      //! Returns the Coordinate of the currently selected CanvasNode.
      virtual boost::optional<Coordinate> GetCurrentCoordinate() const;

      //! Sets the current CanvasNode.
      /*!
        \param node The CanvasNode to set as the current.
      */
      virtual void SetCurrent(const CanvasNode& node);

      //! Sets the current CanvasNode.
      /*!
        \param coordinate The Coordinate to set as current.
      */
      virtual void SetCurrent(const Coordinate& coordinate);

      //! Adds a CanvasNode to this model.
      /*!
        \param coordinate The Coordinate to place the CanvasNode.
        \param node The CanvasNode to place.
        \return The CanvasNode that was added.
      */
      virtual const CanvasNode& Add(const Coordinate& coordinate,
        const CanvasNode& node) = 0;

      //! Removes a node from this model.
      /*!
        \param node The CanvasNode to remove.
      */
      virtual void Remove(const CanvasNode& node);

      //! Removes a node from this model.
      /*!
        \param coordinate The Coordinate of the CanvasNode to remove.
      */
      virtual void Remove(const Coordinate& coordinate);
  };
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Spire::CanvasNodeModel::Coordinate> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Spire::CanvasNodeModel::Coordinate& value, unsigned int version) const {
      shuttle.Shuttle("row", value.m_row);
      shuttle.Shuttle("column", value.m_column);
    }
  };
}
}

#endif
