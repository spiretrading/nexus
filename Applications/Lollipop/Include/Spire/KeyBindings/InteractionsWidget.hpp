#ifndef SPIRE_INTERACTIONSWIDGET_HPP
#define SPIRE_INTERACTIONSWIDGET_HPP
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_InteractionsWidget;

namespace Spire {

  /*! \class InteractionsWidget
      \brief Displays and allows modification of the InteractionsProperties.
   */
  class InteractionsWidget : public QWidget {
    public:

      //! Constructs an InteractionsWidget, call Initialize after construction.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      InteractionsWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~InteractionsWidget();

      //! Initializes the InteractionsWidget, must be called after construction.
      /*!
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      //! Initializes the InteractionsWidget, must be called after construction.
      /*!
        \param userProfile The user's profile.
        \param security The Security to display the InteractionsProperties for.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const Nexus::Security& security);

      //! Returns the InteractionsProperties represented by this widget.
      const Nexus::RegionMap<InteractionsProperties>& GetProperties();

    private:
      struct RegionEntry {
        Nexus::Region m_region;
        bool m_isActive;
      };
      std::unique_ptr<Ui_InteractionsWidget> m_ui;
      UserProfile* m_userProfile;
      Nexus::RegionMap<InteractionsProperties> m_properties;
      std::unordered_map<std::string, RegionEntry> m_regions;
      std::string m_regionIndex;
      int m_quantityModifierIndex;
      int m_priceModifierIndex;

      void AddRegion(RegionEntry region);
      void StyleRegion(const RegionEntry& region);
      void Update();
      void Store();
      void OnRegionIndexChanged(int index);
      void OnKeyboardModifierIndexChanged(int index);
      void OnActivateRegionClicked();
      void OnResetRegionClicked();
  };
}

#endif
