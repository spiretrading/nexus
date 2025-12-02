#ifndef SPIRE_ACTIVITYLOGWIDGET_HPP
#define SPIRE_ACTIVITYLOGWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

class Ui_ActivityLogWidget;

namespace Spire {

  /*! \class ActivityLogWidget
      \brief Displays a blotter's activity log.
   */
  class ActivityLogWidget : public QWidget {
    public:

      /*! \struct UIState
          \brief Stores the user interface state of this widget.
       */
      struct UIState {

        //! The geometry of the open positions table header.
        QByteArray m_tableGeometry;

        //! The state of the open positions table header.
        QByteArray m_tableState;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };

      //! Constructs an ActivityLogWidget.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ActivityLogWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ActivityLogWidget();

      //! Returns the current UI state.
      UIState GetUIState() const;

      //! Sets the UI state.
      /*!
        \param state The UIState to set.
      */
      void SetUIState(const UIState& state);

      //! Sets the BlotterModel this widget is representing.
      /*!
        \param userProfile The user's profile.
        \param model The model to represent.
      */
      void SetModel(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BlotterModel> model);

    private:
      std::unique_ptr<Ui_ActivityLogWidget> m_ui;
      UserProfile* m_userProfile;
      BlotterModel* m_model;

      void OnContextMenu(const QPoint& position);
  };

  template<Beam::IsShuttle S>
  void ActivityLogWidget::UIState::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("table_geometry", m_tableGeometry);
    shuttle.shuttle("table_state", m_tableState);
  }
}

#endif
