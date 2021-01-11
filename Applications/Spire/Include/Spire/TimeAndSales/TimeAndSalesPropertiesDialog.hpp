#ifndef SPIRE_TIMEANDSALESPROPERTIESDIALOG_HPP
#define SPIRE_TIMEANDSALESPROPERTIESDIALOG_HPP
#include <boost/signals2/connection.hpp>
#include <QDialog>
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

class QAbstractButton;
class QListWidgetItem;
class Ui_TimeAndSalesPropertiesDialog;

namespace Spire {

  /*! \class TimeAndSalesPropertiesDialog
      \brief Displays the properties for a TimeAndSalesWindow.
   */
  class TimeAndSalesPropertiesDialog : public QDialog {
    public:

      //! Constructs a TimeAndSalesPropertiesDialog.
      /*!
        \param userProfile The user's profile.
        \param properties The TimeAndSalesProperties to modify.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeAndSalesPropertiesDialog(Beam::Ref<UserProfile> userProfile,
        const TimeAndSalesProperties& properties, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TimeAndSalesPropertiesDialog();

      //! Returns the TimeAndSaleProperties represented by this dialog.
      const TimeAndSalesProperties& GetProperties() const;

    private:
      std::unique_ptr<Ui_TimeAndSalesPropertiesDialog> m_ui;
      UserProfile* m_userProfile;
      TimeAndSalesProperties m_properties;
      QListWidgetItem* m_priceRangeItems[
        TimeAndSalesProperties::PRICE_RANGE_COUNT];
      boost::signals2::scoped_connection m_fontChangedConnection;

      void Redisplay();
      void OnFontChanged(const QFont& oldFont, const QFont& newFont);
      void OnPriceRangeItemChanged(QListWidgetItem* current,
        QListWidgetItem* previous);
      void OnForegroundClicked();
      void OnBackgroundClicked();
      void OnColumnCheckBoxClicked(QAbstractButton* button);
      void OnShowGridClicked(int state);
      void OnHorizontalScrollBoxClicked(int state);
      void OnVerticalScrollBoxClicked(int state);
      void OnLoadDefault();
      void OnSaveAsDefault();
      void OnResetDefault();
      void OnApply();
      void OnApplyToAll();
  };
}

#endif
