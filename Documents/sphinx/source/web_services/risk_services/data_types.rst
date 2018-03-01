Nexus Risk Web Services Data Types
============================================

.. py:currentmodule:: nexus.risk_service

.. py:class:: RiskParameters

    Stores the set of parameters used to control an account's trading risk.

  .. py:attribute:: currency

      :py:class:`nexus.CurrencyId`
      The currency used for risk calculations.

  .. py:attribute:: buying_power

      :py:class:`nexus.Money`
      The maximum amount of buying power.

  .. py:attribute:: allowed_state

      :py:class:`nexus.risk_service.RiskState`
      The default risk state of the account.

  .. py:attribute:: net_loss

      :py:class:`nexus.Money`
      The maximum net loss before entering closed orders mode.

  .. py:attribute:: loss_from_top

      :py:class:`number`
      The percentage lost from the top before entering closed orders mode.

  .. py:attribute:: transition_time

      :py:class:`beam.Duration`
      The amount of time allowed to transition from closed orders mode to
      disabled mode.
