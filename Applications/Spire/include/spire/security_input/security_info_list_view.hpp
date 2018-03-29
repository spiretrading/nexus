#ifndef SPIRE_SECURITY_INFO_LIST_VIEW
#define SPIRE_SECURITY_INFO_LIST_VIEW
#include <vector>
#include <QScrollArea>
#include "Nexus/Definitions/SecurityInfo.hpp"

namespace spire {

  //! \brief Displays a list of securities with symbol, exchange, and company
  //! name.
  class security_info_list_view : public QScrollArea {
    public:

      //! Constructs a security_info_list_view with an empty list.
      /*!
        \param parent The parent to the security_info_list_view.
      */
      security_info_list_view(QWidget* parent = nullptr);

      //! Sets the displayed list.
      /*!
        \param list The list of securities to display.
      */
      void set_list(const std::vector<Nexus::SecurityInfo>& list);

    private:
      QWidget* m_list_widget;
  };
}

#endif
