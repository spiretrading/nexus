Nexus Administration Web Services Data Types
============================================

.. json:object:: nexus.administration_service.TradingGroup

   Represents a group of traders and their managers.

  :property entry: The trading group's directory entry.
  :proptype entry: :json:object:`beam.service_locator.DirectoryEntry`
  :property managers_directory: The directory containing this group's managers.
  :proptype managers_directory: :json:object:`beam.service_locator.DirectoryEntry`
  :property managers: The list of managers belonging to this group.
  :proptype managers: :json:object:`Array\<beam.service_locator.DirectoryEntry\> <beam.service_locator.DirectoryEntry>`
  :property traders_directory: The directory containing this group's traders.
  :proptype traders_directory: :json:object:`beam.service_locator.DirectoryEntry`
  :property traders: The list of traders belonging to this group.
  :proptype traders: :json:object:`Array\<beam.service_locator.DirectoryEntry\> <beam.service_locator.DirectoryEntry>`
