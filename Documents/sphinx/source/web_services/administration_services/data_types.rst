Nexus Administration Web Services Data Types
============================================

.. py:currentmodule:: nexus.administration_service

.. py:class:: AccountRole

    Enumerates an account's roles.

    .. py:data:: NONE=0

      The account has no role.

    .. py:data:: TRADER=1

      The account has permission to trade.

    .. py:data:: MANAGER=2

      The account is a trading group manager.

    .. py:data:: SERVICE=3

      The account provides an API service.

    .. py:data:: ADMINISTRATOR=4

      The account is an administrator.

.. py:class:: AccountRoles

    Stores a bitset containing all of an account's roles.

    Refer to: :py:class:`AccountRole`

.. py:class:: AccountModificationRequest

    Stores a request to modify an account.

  .. py:attribute:: id

    `number`
    The id that uniquely identifies this request.

  .. py:attribute:: type

    :py:class:`AccountModificationRequest.Type`
    The type of modification requested.

  .. py:attribute:: account

    :py:class:`beam.service_locator.DirectoryEntry`
    The account to modify.

  .. py:attribute:: submission_account

    :py:class:`beam.service_locator.DirectoryEntry`
    The account that submitted the request.

  .. py:attribute:: timestamp

    :py:class:`beam.DateTime`
    The timestamp when the request was received.

  .. py:class:: Type

      Enumerates the types of modifications that can be made to an account.

    .. py:data:: ENTITLEMENTS=0

      Modify an account's market data entitlements.

  .. py:class:: Status

      Lists the status of a request.

    .. py:data:: NONE=0

      No or invalid status.

    .. py:data:: PENDING=1

      The request is pending.

    .. py:data:: REVIEWED=2

      The request has been reviewed by a manager.

    .. py:data:: SCHEDULED=3

      The request has been scheduled.

    .. py:data:: GRANTED=4

      The request has been granted.

    .. py:data:: REJECTED=5

      The request has been rejected.

  .. py:class:: Update

      Stores a request status update.

    .. py:attribute:: status

      :py:class:`AccountModificationRequest.Status`
      The updated status.

    .. py:attribute:: account

      :py:class:`beam.service_locator.DirectoryEntry`
      The account that updated the status.

    .. py:attribute:: sequence_number

      `number`
      The update sequence number.

    .. py:attribute:: timestamp

      :py:class:`beam.DateTime`
      The timestamp when the update occurred.

.. py:class:: EntitlementModification

    Stores a request to update an account's market data entitlements.

  .. py:attribute:: entitlements

      :py:class:`beam.service_locator.DirectoryEntry[] <beam.service_locator.DirectoryEntry>`
      The list of entitlements to grant to the account.

.. py:class:: Message

    Stores the details of a single message.

  .. py:attribute:: id

      `number`
      The message's unique id.

  .. py:attribute:: account

      :py:class:`beam.service_locator.DirectoryEntry`
      The account that sent the message.

  .. py:attribute:: timestamp

      :py:class:`beam.DateTime`
      The timestamp when the message was received.

  .. py:attribute:: bodies

      :py:class:`Message.Body[] <Message.Body>`
      The list of message bodies.

  .. py:class:: Body

      Stores the body of a message.

    .. py:attribute:: content_type

      `string`
      The MIME-type of the message.

    .. py:attribute:: message

      `string`
      The contents of the message.

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
