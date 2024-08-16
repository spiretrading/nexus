#ifndef SPIRE_FILEPATHINPUTWIDGET_HPP
#define SPIRE_FILEPATHINPUTWIDGET_HPP
#include <filesystem>
#include <QFileDialog>
#include <QWidget>
#include "Spire/UI/UI.hpp"

class Ui_FilePathInputWidget;

namespace Spire {
namespace UI {

  /*! \class FilePathInputWidget
      \brief Allows a user to select a file.
   */
  class FilePathInputWidget : public QWidget {
    public:

      //! Constructs a FilePathInputWidget.
      /*!
        \param parent The parent widget.
        \param flags The flags passed to the <i>parent</i> widget.
      */
      FilePathInputWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~FilePathInputWidget();

      //! Returns the path.
      std::filesystem::path GetPath() const;

      //! Sets the path.
      /*!
        \param path The font to represent.
      */
      void SetPath(const std::filesystem::path& path);

      //! Sets the caption.
      /*!
        \param caption The caption to display when searching for a file.
      */
      void SetCaption(const std::string& caption);

      //! Sets the file filter.
      /*!
        \param filter The file filter used when searching for a file.
      */
      void SetFilter(const std::string& filter);

      //! Returns the AcceptMode.
      QFileDialog::AcceptMode GetAcceptMode() const;

      //! Sets the AcceptMode.
      void SetAcceptMode(QFileDialog::AcceptMode acceptMode);

    private:
      friend class Ui_FilePathInputWidget;
      std::unique_ptr<Ui_FilePathInputWidget> m_ui;
      std::string m_caption;
      std::string m_filter;
      QFileDialog::AcceptMode m_acceptMode;

      void OnOpenFileButtonClicked();
  };
}
}

#endif
