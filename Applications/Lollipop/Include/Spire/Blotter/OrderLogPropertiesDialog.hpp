#ifndef SPIRE_ORDERLOGPROPERTIESDIALOG_HPP
#define SPIRE_ORDERLOGPROPERTIESDIALOG_HPP
#include <memory>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OrderLogProperties.hpp"
#include "Spire/Spire/Spire.hpp"

class QCheckBox;
class Ui_OrderLogPropertiesDialog;

namespace Spire {

  /*! \class OrderLogPropertiesDialog
      \brief Allows editing of the blotter's Order log display properties.
   */
  class OrderLogPropertiesDialog : public QDialog {
    public:

      //! Constructs a OrderLogPropertiesDialog.
      /*!
        \param userProfile The user's profile.
        \param blotterModel The BlotterModel to represent.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      OrderLogPropertiesDialog(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BlotterModel> blotterModel, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~OrderLogPropertiesDialog();

    private:
      std::unique_ptr<Ui_OrderLogPropertiesDialog> m_ui;
      UserProfile* m_userProfile;
      BlotterModel* m_blotterModel;
      OrderLogProperties m_properties;
      std::unordered_map<Nexus::OrderStatus, QCheckBox*>
        m_orderStatusCheckBoxes;

      void OnAllOrdersClicked(bool checked);
      void OnLiveOrdersClicked(bool checked);
      void OnTerminalOrdersClicked(bool checked);
      void OnCustomOrdersClicked(bool checked);
      void OnOkButton();
      void OnApplyAllButton();
  };
}

#endif
