#ifndef SPIRE_BOOKVIEWHIGHLIGHTPROPERTIESWIDGET_HPP
#define SPIRE_BOOKVIEWHIGHLIGHTPROPERTIESWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/UI/UI.hpp"

class QListWidgetItem;
class Ui_BookViewHighlightPropertiesWidget;

namespace Spire {

  /*! \class BookViewHighlightPropertiesWidget
      \brief Displays a book view's venue highlight properties.
   */
  class BookViewHighlightPropertiesWidget : public QWidget {
    public:

      //! Constructs a BookViewHighlightPropertiesWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      BookViewHighlightPropertiesWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~BookViewHighlightPropertiesWidget();

      //! Initializes the BookViewHighlightPropertiesWidget.
      /*!
        \param userProfile The user's profile.
        \param properties The BookViewProperties to represent.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BookViewProperties> properties);

      void Redisplay();

    private:
      std::unique_ptr<Ui_BookViewHighlightPropertiesWidget> m_ui;
      UserProfile* m_userProfile;
      BookViewProperties* m_properties;
      QColor m_venueHighlightColor;
      QColor m_orderHighlightColor;

      const Nexus::VenueDatabase::Entry&
        GetCurrentVenueHighlightEntry() const;
      void UpdateVenueHighlightColor(const QColor& color);
      void UpdateOrderHighlightColor(const QColor& color);
      void OnCurrentItemChanged(QListWidgetItem* current,
        QListWidgetItem* previous);
      void OnNoneClicked(bool checked);
      void OnTopLevelClicked(bool checked);
      void OnAllLevelsClicked(bool checked);
      void OnColorClicked();
      void OnHideOrdersClicked(bool checked);
      void OnDisplayOrdersClicked(bool checked);
      void OnHighlightOrdersClicked(bool checked);
      void OnOrderHighlightColorClicked();
  };
}

#endif
