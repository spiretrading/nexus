#ifndef SPIRE_FILEREADERNODEDIALOG_HPP
#define SPIRE_FILEREADERNODEDIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include <QMetaObject>
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/Spire/Spire.hpp"

class QAbstractButton;
class QComboBox;
class QTableWidgetItem;
class Ui_FileReaderNodeDialog;

namespace Spire {

  /*! \class FileReaderNodeDialog
      \brief Edits a FileReaderNode.
   */
  class FileReaderNodeDialog : public QDialog {
    public:

      //! Constructs a FileReaderNodeDialog for a new FileReaderNode.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      FileReaderNodeDialog(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a FileReaderNodeDialog based on an existing FileReaderNode.
      /*!
        \param userProfile The user's profile.
        \param node The FileReaderNode to edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      FileReaderNodeDialog(const FileReaderNode& node,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~FileReaderNodeDialog();

      //! Returns the new FileReaderNode.
      std::unique_ptr<FileReaderNode> GetNode();

    private:
      friend class FileReaderNodeStructureTable;
      friend class Ui_FileReaderNodeDialog;
      std::unique_ptr<Ui_FileReaderNodeDialog> m_ui;
      UserProfile* m_userProfile;
      std::unique_ptr<FileReaderNode> m_originalNode;
      std::vector<RecordType::Field> m_fields;
      QMetaObject::Connection m_itemChangedConnection;

      void Setup(const FileReaderNode& node);
      void AddField(const RecordType::Field& field, int row);
      void DeleteItem(QTableWidgetItem* item);
      void AddNewField();
      void OnButtonClicked(QAbstractButton* button);
      void OnItemChanged(QTableWidgetItem* item);
      void OnItemActivated(QTableWidgetItem* item);
      void OnCurrentItemChanged(QTableWidgetItem* current,
        QTableWidgetItem* previous);
  };
}

#endif
