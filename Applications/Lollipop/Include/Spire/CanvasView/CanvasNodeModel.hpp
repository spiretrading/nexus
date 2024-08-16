#ifndef SPIRE_CANVAS_NODE_MODEL_HPP
#define SPIRE_CANVAS_NODE_MODEL_HPP
#include <functional>
#include <memory>
#include <ostream>
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

  /** Interface for a widget displaying CanvasNodes. */
  class CanvasNodeModel {
    public:

      /** Stores a CanvasNode's identity within a CanvasNodeModel. */
      struct Identity {

        /**
         * Constructs a default Identity with a model id of 0 and canvas node id
         * of 0.
         */
        Identity();

        /**
         * Constructs an Identity.
         */
        Identity(int modelId, int canvasNodeId);

        /** The id of the CanvasNodeModel. */
        int m_modelId;

        /** The id of the CanvasNode. */
        int m_canvasNodeId;
      };

      /** Stores a CanvasNodeModel coordinate. */
      struct Coordinate {

        /** The Coordinate's row. */
        int m_row;

        /** The Coordinate's column. */
        int m_column;

        auto operator <=>(const Coordinate& coordinate) const = default;
      };

      /** Saves a snapshot of a CanvasNodeModel that can be restored later. */
      class Snapshot {
        public:

          /** Constructs an empty Snapshot. */
          Snapshot() = default;

          /**
           * Saves a Snapshot of a CanvasNodeModel.
           * @param model The CanvasNodeModel to save.
           */
          void Save(const CanvasNodeModel& model);

          /**
           * Restores a CanvasNodeModel to this Snapshot.
           * @param model The CanvasNodeModel to restore this Snapshot to.
           */
          void Restore(Beam::Out<CanvasNodeModel> model);

        private:
          std::vector<std::tuple<Coordinate, std::unique_ptr<CanvasNode>>>
            m_roots;
      };

      /** Meta-data used to keep track of changes to a CanvasNode. */
      static std::string GetIdentityKey();

      /**
       * Finds a CanvasNode's Identity.
       * @param node The CanvasNode whose Identity is to be looked up.
       */
      static boost::optional<Identity> FindIdentity(const CanvasNode& node);

      /**
       * Returns the meta-data representing an Identity.
       * @param identity The Identity to convert into meta-data.
       * @return The string representation of the <i>identity</i>.
       */
      static std::string ToMetaData(const Identity& identity);

      /**
       * Meta-data used to determine which CanvasNode a ReferenceNode refers
       * to.
       */
      static std::string GetReferentKey();

      /**
       * Finds a ReferenceNode's referent via it's identity.
       * @param node The ReferenceNode whose referent is to be found.
       */
      static boost::optional<const CanvasNode&>
        FindReferent(const ReferenceNode& node);

      /**
       * Strips all CanvasNodes of their identity meta-data.
       * @param node The root of the CanvasNode whose identity is to be striped.
       * @return A clone of the <i>node</i> with all identity meta-data striped.
       */
      static std::unique_ptr<CanvasNode> StripIdentity(const CanvasNode& node);

      virtual ~CanvasNodeModel() = default;

      /** Returns all root CanvasNodes. */
      virtual std::vector<const CanvasNode*> GetRoots() const = 0;

      /**
       * Returns the CanvasNode at a specified location.
       * @param coordinate The Coordinate to retrieve the CanvasNode from.
       * @return The CanvasNode at the specified <i>coordinate</i>.
       */
      virtual boost::optional<const CanvasNode&>
        GetNode(const Coordinate& coordinate) const = 0;

      /**
       * Returns the Coordinate of a CanvasNode.
       * @param node The CanvasNode whose Coordinate is to be returned.
       * @return The Coordinate of the specified <i>node</i>.
       */
      virtual Coordinate GetCoordinate(const CanvasNode& node) const = 0;

      /** Returns the currently selected CanvasNode. */
      virtual boost::optional<const CanvasNode&> GetCurrentNode() const;

      /** Returns the Coordinate of the currently selected CanvasNode. */
      virtual boost::optional<Coordinate> GetCurrentCoordinate() const;

      /**
       * Sets the current CanvasNode.
       * @param node The CanvasNode to set as the current.
       */
      virtual void SetCurrent(const CanvasNode& node);

      /**
       * Sets the current CanvasNode.
       * @param coordinate The Coordinate to set as current.
       */
      virtual void SetCurrent(const Coordinate& coordinate);

      /**
       * Adds a CanvasNode to this model.
       * @param coordinate The Coordinate to place the CanvasNode.
       * @param node The CanvasNode to place.
       * @return The CanvasNode that was added.
       */
      virtual const CanvasNode&
        Add(const Coordinate& coordinate, const CanvasNode& node) = 0;

      /**
       * Removes a node from this model.
       * @param node The CanvasNode to remove.
       */
      virtual void Remove(const CanvasNode& node);

      /**
       * Removes a node from this model.
       * @param coordinate The Coordinate of the CanvasNode to remove.
       */
      virtual void Remove(const Coordinate& coordinate);
  };

  std::ostream& operator <<(
    std::ostream& out, const CanvasNodeModel::Coordinate& coordinate);
}

namespace Beam::Serialization {
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

namespace std {
  template <>
  struct hash<Spire::CanvasNodeModel::Coordinate> {
    size_t operator()(const Spire::CanvasNodeModel::Coordinate& value) const;
  };
}

#endif
