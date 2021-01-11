#ifndef SPIRE_BOOKVIEWLEVELPROPERTIESWIDGET_HPP
#define SPIRE_BOOKVIEWLEVELPROPERTIESWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/BookView/BookView.hpp"

class QListWidgetItem;
class Ui_BookViewLevelPropertiesWidget;

namespace Spire {

  /*! \class BookViewLevelPropertiesWidget
      \brief Displays a book view's level properties.
   */
  class BookViewLevelPropertiesWidget : public QWidget {
    public:

      //! Constructs a BookViewLevelPropertiesWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      BookViewLevelPropertiesWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~BookViewLevelPropertiesWidget();

      //! Initializes the BookViewLevelPropertiesWidget.
      /*!
        \param properties The BookViewProperties to represent.
      */
      void Initialize(Beam::Ref<BookViewProperties> properties);

      void Redisplay();

    private:
      std::unique_ptr<Ui_BookViewLevelPropertiesWidget> m_ui;
      BookViewProperties* m_properties;
      std::vector<QListWidgetItem*> m_colorBandItems;
      QColor m_topGradient;
      QColor m_bottomGradient;
      boost::signals2::scoped_connection m_fontChangedConnection;

      void OnFontChanged(const QFont& oldFont, const QFont& newFont);
      void OnPriceRangeItemChanged(QListWidgetItem* current,
        QListWidgetItem* previous);
      void OnTopGradientButtonClicked();
      void OnBottomGradientButtonClicked();
      void OnApplyGradientButtonClicked();
      void OnBandColorButtonClicked();
      void OnBandCountValueChanged(int value);
      void OnShowGridClicked(int state);
      void OnShowBboClicked(int state);
  };
}

#endif
