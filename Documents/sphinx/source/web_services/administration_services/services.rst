Nexus Administration Web Services
=================================

.. http:post:: /api/administration_service/load_organization_name

    Loads the name of the organization.

    :statuscode 200: :py:class:`string`
      The name of the organization using this instance.

.. http:post:: /api/administration_service/load_accounts_by_roles

    Loads the list of accounts that match a set of roles.

    :param roles: The roles to match.
    :type roles: :py:class:`nexus.administration_service.AccountRoles`
    :statuscode 200: :py:class:`beam.service_locator.DirectoryEntry[] <beam.service_locator.DirectoryEntry>`
      The list of directory entries of accounts that match the specified roles.

.. http:post:: /api/administration_service/load_trading_group

    Loads the trading group represented by a directory entry.

    :param directory_entry: The directory entry of the trading group to load.
    :type directory_entry: :py:class:`beam.service_locator.DirectoryEntry`
    :statuscode 200: :py:class:`nexus.administration_service.TradingGroup`
      The trading group represented by the directory_entry.

.. http:post:: /api/administration_service/load_account_modification_request

    Loads an account modification request.

    :param id: The id of the request to load.
    :type id: number
    :statuscode 200: :py:class:`nexus.administration_service.AccountModificationRequest`
      The request with the specified id.

.. http:post:: /api/administration_service/load_account_modification_request_ids

    Given an account, loads the ids of requests to modify that account.

    :param account: The account whose requests are to be loaded.
    :type account: :py:class:`beam.service_locator.DirectoryEntry`
    :param start_id: The id of the first request to load (exclusive) or -1 to
                     start with the most recent request.
    :type start_id: number
    :param max_count: The maximum number of ids to load.
    :type max_count: number
    :statuscode 200: :py:class:`number[]`
      The list of ids referencing the specified account modification requests.

.. http:post:: /api/administration_service/load_managed_account_modification_request_ids

    Given an account, loads the ids of requests that the account is authorized
    to manage.

    :param account: The account managing modifications.
    :type account: :py:class:`beam.service_locator.DirectoryEntry`
    :param start_id: The id of the first request to load (exclusive) or -1 to
                     start with the most recent request.
    :type start_id: number
    :param max_count: The maximum number of ids to load.
    :type max_count: number
    :statuscode 200: :py:class:`number[]`
      The list of ids referencing the specified account modification requests.

.. http:post:: /api/administration_service/load_entitlement_modification

    Loads an entitlement modification.

    :param id: The id of the modification to load.
    :type id: number
    :statuscode 200: :py:class:`nexus.administration_service.EntitlementModifcation`
      The entitlement modification with the specified id.

.. http:post:: /api/administration_service/submit_entitlement_modification_request

    Submits a request to modify an account's entitlements.

    :param account: The account to modify.
    :type account: :py:class:`beam.service_locator.DirectoryEntry`
    :param modification: The modification to apply.
    :type modification: :py:class:`nexus.administration_service.EntitlementModification`
    :param comment: The comment to associate with the request.
    :type comment: :py:class:`nexus.administration_service.Message`
    :statuscode 200: :py:class:`nexus.administration_service.AccountModificationRequest`
      An object representing the request.

.. http:post:: /api/administration_service/load_account_modification_request_status

    Loads the status of an account modification request.

    :param id: The id of the request.
    :type id: number
    :statuscode 200: :py:class:`nexus.administration_service.AccountModificationRequest.Update`
      The update representing the current status of the request.

.. http:post:: /api/administration_service/approve_account_modification_request

    Approves an account modification request.

    :param id: The id of the request to approve.
    :type id: number
    :param comment: The comment to associate with the update.
    :type comment: :py:class:`nexus.administration_service.Message`
    :statuscode 200: :py:class:`nexus.administration_service.AccountModificationRequest.Update`
      The object representing the update.

.. http:post:: /api/administration_service/reject_account_modification_request

    Rejects an account modification request.

    :param id: The id of the request to reject.
    :type id: number
    :param comment: The comment to associate with the update.
    :type comment: :py:class:`nexus.administration_service.Message`
    :statuscode 200: :py:class:`nexus.administration_service.AccountModificationRequest.Update`
      The object representing the update.

.. http:post:: /api/administration_service/load_message

    Loads a message.

    :param id: The id of the message.
    :type id: number
    :statuscode 200: :py:class:`nexus.administration_service.Message`
      The message with the specified id.

.. http:post:: /api/administration_service/load_message_ids

    Loads the list of messages associated with an account modification.

    :param id: The id of the request.
    :type id: number
    :statuscode 200: :py:class:`number[]`
      The list of ids referencing the messages associated with the request.

.. http:post:: /api/administration_service/send_account_modification_request_message

    Appends a message to an account modification request.

    :param id: The id of the request to associate the message with.
    :type id: number
    :param message: The message to append.
    :type message: :py:class:`nexus.administration_service.Message`
    :statuscode 200: :py:class:`nexus.administration_service.Message`
      The message that was appended.
