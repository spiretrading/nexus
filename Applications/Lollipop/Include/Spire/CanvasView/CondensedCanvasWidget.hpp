#ifndef SPIRE_CONDENSEDCANVASWIDGET_HPP
#define SPIRE_CONDENSEDCANVASWIDGET_HPP
#include <map>
#include <set>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/UI/UI.hpp"

class QGroupBox;
class QGridLayout;

namespace Spire {

  /*! \class CondensedCanvasWidget
      \brief Displays a single CanvasNode in condensed form.
   */
  class CondensedCanvasWidget : public QWidget, public CanvasNodeModel {
    public:

      //! Constructs a CondensedCanvasWidget.
      /*!
        \param name The name to display.
        \param userProfile The user's profile.
        \param parent The parent widget.
      */
      CondensedCanvasWidget(const std::string& name,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      virtual ~CondensedCanvasWidget();

      //! Grants focus to the first visible CanvasNode.
      void Focus();

      //! Navigates the current node forward.
      void NavigateForward();

      //! Navigates the current node backward.
      void NavigateBackward();

      virtual std::vector<const CanvasNode*> GetRoots() const;

      virtual boost::optional<const CanvasNode&> GetNode(
        const Coordinate& coordinate) const;

      virtual Coordinate GetCoordinate(const CanvasNode& node) const;

      virtual boost::optional<const CanvasNode&> GetCurrentNode() const;

      virtual void SetCurrent(const Coordinate& coordinate);

      virtual const CanvasNode& Add(const Coordinate& coordinate,
        const CanvasNode& node);

      virtual void Remove(const Coordinate& coordinate);

      using CanvasNodeModel::SetCurrent;

    protected:
      virtual void focusInEvent(QFocusEvent* event);

    private:
      std::string m_name;
      UserProfile* m_userProfile;
      QGroupBox* m_group;
      QGridLayout* m_layout;
      std::unique_ptr<CanvasNode> m_node;
      std::set<std::unique_ptr<QWidget>> m_deletedCells;
      const CanvasNode* m_topLeaf;
      const CanvasNode* m_currentNode;
      std::map<const CanvasNode*, Coordinate> m_nodeToModelCoordinates;
      std::map<Coordinate, const CanvasNode*> m_modelCoordinatesToNode;
      std::map<const CanvasNode*, Coordinate> m_nodeToViewCoordinates;
      std::map<Coordinate, const CanvasNode*> m_viewCoordinatesToNode;
  };
}

#endif
