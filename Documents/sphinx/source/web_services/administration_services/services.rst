Nexus Administration Web Services
=================================

.. http:post:: /api/administration_service/load_trading_group

   Loads the trading group represented by a directory entry.

  :param directory_entry: The directory entry of the trading group to load.
  :type directory_entry: beam.service_locator.DirectoryEntry
  :statuscode 200: :json:object:`nexus.administration_service.TradingGroup`
    The trading group represented by the directory_entry.
