#ifndef SPIRE_RISKWIDGET_HPP
#define SPIRE_RISKWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"

class Ui_RiskWidget;

namespace Spire {
  class UserProfile;

  /*! \class RiskWidget
      \brief Shows the risk parameters for an account.
   */
  class RiskWidget : public QWidget {
    public:

      //! Constructs a RiskWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      RiskWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~RiskWidget();

      //! Sets the user's profile.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the model is being shown
               strictly for display purposes.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile, bool isReadOnly);

      //! Returns the model being displayed.
      const RiskModel& GetModel() const;

      //! Returns the model being displayed.
      RiskModel& GetModel();

      //! Sets the model that this widget displays.
      /*!
        \param model The model to display.
      */
      void SetModel(const std::shared_ptr<RiskModel>& model);

      //! Commits changes to the model.
      void Commit();

    private:
      std::unique_ptr<Ui_RiskWidget> m_ui;
      bool m_isReadOnly;
      UserProfile* m_userProfile;
      std::shared_ptr<RiskModel> m_model;

      void OnCurrencyIndexChanged(int index);
  };
}

#endif
