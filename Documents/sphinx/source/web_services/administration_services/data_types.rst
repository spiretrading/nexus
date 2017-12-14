Nexus Administration Web Services Data Types
============================================

.. py:currentmodule:: nexus.administration_service

.. py:class:: TradingGroup

   Represents a group of traders and their managers.

  .. py:attribute:: entry

    :py:class:`beam.service_locator.DirectoryEntry`
    The trading group's directory entry.

  .. py:attribute:: managers_directory

    :py:class:`beam.service_locator.DirectoryEntry`
    The directory containing this group's managers.

  .. py:attribute:: managers

    :py:class:`beam.service_locator.DirectoryEntry[]<beam.service_locator.DirectoryEntry>`
    The list of managers belonging to this group.

  .. py:attribute:: traders_directory

    :py:class:`beam.service_locator.DirectoryEntry`
    The directory containing this group's traders.

  .. py:attribute:: traders

    :py:class:`beam.service_locator.DirectoryEntry[]<beam.service_locator.DirectoryEntry>`
    The list of traders belonging to this group.
