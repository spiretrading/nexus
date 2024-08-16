#ifndef SPIRE_CANVASTABLEWIDGET_HPP
#define SPIRE_CANVASTABLEWIDGET_HPP
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QTableWidget>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/CanvasView/CanvasNodeEditor.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/Spire/Spire.hpp"

class QUndoCommand;

namespace Spire {
namespace Details {
  struct VetoCombiner {
    typedef bool result_type;

    template<typename InputIterator>
    bool operator()(InputIterator first, InputIterator last) const {
      for(auto i = first; i != last; ++i) {
        if(!*i) {
          return false;
        }
      }
      return true;
    }
  };
}

  /*! \class CanvasTableWidget
      \brief Displays a CanvasNodeModel inside of a QTableWidget.
   */
  class CanvasTableWidget : public QTableWidget, public CanvasNodeModel {
    public:

      //! Signals a CanvasNode is about to be edited.
      /*!
        \param node The CanvasNode about to be edited.
      */
      typedef boost::signals2::signal<bool (const CanvasNode& node),
        Details::VetoCombiner> BeginEditSignal;

      //! Signals a QUndoCommand was instantiated by this widget.
      /*!
        \param command The QUndoCommand that was instantiated.
      */
      typedef boost::signals2::signal<void (QUndoCommand* command)>
        CommandSignal;

      //! The number of columns initially displayed.
      static const int INITIAL_COLUMN_COUNT = 100;

      //! The number of rows initially displayed.
      static const int INITIAL_ROW_COUNT = 100;

      //! Constructs a CanvasTableWidget.
      /*!
        \param parent The parent widget.
      */
      CanvasTableWidget(QWidget* parent = nullptr);

      //! Sets the user's profile.
      /*!
        \param userProfile The user's profile.
      */
      void SetUserProfile(Beam::Ref<UserProfile> userProfile);

      //! Navigates the current node forward.
      void NavigateForward();

      //! Navigates the current node backward.
      void NavigateBackward();

      //! Returns the currently selected CanvasNodes.
      std::vector<const CanvasNode*> GetSelectedNodes() const;

      //! Edits a CanvasNode.
      /*!
        \param node The CanvasNode to edit.
        \param event The event that triggered the edit.
      */
      void Edit(const CanvasNode& node, QEvent* event = nullptr);

      //! Connects the signal indicating an edit is being performed.
      /*!
        \param slot The slot to connect to the signal.
        \return The connection to the signal.
      */
      boost::signals2::connection ConnectBeginEditSignal(
        const BeginEditSignal::slot_type& slot) const;

      //! Connects a slot to the CommandSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return The connection to the signal.
      */
      boost::signals2::connection ConnectCommandSignal(
        const CommandSignal::slot_type& slot) const;

      virtual std::vector<const CanvasNode*> GetRoots() const;

      virtual boost::optional<const CanvasNode&> GetNode(
        const Coordinate& coordinate) const;

      virtual Coordinate GetCoordinate(const CanvasNode& node) const;

      virtual boost::optional<const CanvasNode&> GetCurrentNode() const;

      virtual boost::optional<Coordinate> GetCurrentCoordinate() const;

      virtual void SetCurrent(const CanvasNode& node);

      virtual const CanvasNode& Add(const Coordinate& coordinate,
        const CanvasNode& node);

      virtual void Remove(const CanvasNode& node);

      using CanvasNodeModel::Remove;

    protected:
      virtual bool event(QEvent* event);

      virtual bool eventFilter(QObject* object, QEvent* event);

      virtual void keyPressEvent(QKeyEvent* event);

      virtual void dropEvent(QDropEvent* event);

      virtual void startDrag(Qt::DropActions supportedActions);

    private:
      struct NodeEntry {
        std::unique_ptr<CanvasNode> m_node;
        Coordinate m_coordinate;

        NodeEntry(const CanvasNode& node, const Coordinate& coordinate);
        NodeEntry(const NodeEntry& entry);
        NodeEntry& operator =(const NodeEntry& entry);
      };
      struct NodeCells {
        ValueCell* m_valueCell;
        LabelCell* m_labelCell;
        std::vector<LinkCell*> m_linkCells;

        NodeCells();
      };
      UserProfile* m_userProfile;
      int m_modelId;
      int m_nextCanvasNodeId;
      std::vector<std::unique_ptr<CanvasNode>> m_roots;
      std::unordered_map<const CanvasNode*, NodeCells> m_nodeCells;
      BreadthFirstCanvasNodeIterator m_currentNodeIterator;
      bool m_handlingKeyEvent;
      std::unique_ptr<CanvasNodeEditor> m_editor;
      CanvasCell* m_editedCell;
      QWidget* m_cellEditor;
      mutable BeginEditSignal m_beginEditSignal;
      mutable CommandSignal m_commandSignal;

      BreadthFirstCanvasNodeIterator& GetCurrentNodeIterator();
      void SetNodeSelected(const CanvasNode& node, bool selected);
      void InternalAdd(Coordinate coordinate, const CanvasNode& node);
      int InternalInsert(Coordinate* coordinate, const CanvasNode& node,
        std::vector<const CanvasNode*>* fixedNodes,
        std::vector<NodeEntry>* displacedNodes, const std::string& name);
      void InternalRemove(const CanvasNode& node);
      void CommitEditor();
      void OnContextMenu(const QPoint& position);
  };
}

#endif
