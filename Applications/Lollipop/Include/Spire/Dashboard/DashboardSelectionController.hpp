#ifndef SPIRE_DASHBOARDSELECTIONCONTROLLER_HPP
#define SPIRE_DASHBOARDSELECTIONCONTROLLER_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Dashboard/Dashboard.hpp"

class QKeyEvent;
class QMouseEvent;

namespace Spire {

  /*! \class DashboardSelectionController
      \brief Updates a DashboardSelectionModel based on user input events.
   */
  class DashboardSelectionController : private boost::noncopyable {
    public:

      DashboardSelectionController(
        Beam::Ref<DashboardSelectionModel> model);

      bool HandleKeyEvent(const QKeyEvent& event);

      bool HandleMouseEvent(const QMouseEvent& event, int rowIndex);

    private:
      enum class MouseState {
        NONE,
        PRESSED
      };
      enum class SelectState {
        NONE,
        SINGLE,
        MULTIPLE
      };
      DashboardSelectionModel* m_model;
      MouseState m_mouseState;
      SelectState m_selectState;
      std::vector<int> m_multiSelectedRows;
      std::tuple<int, int> m_multiSelectRange;

      void SetActiveRow(const boost::optional<int>& row);
  };
}

#endif
