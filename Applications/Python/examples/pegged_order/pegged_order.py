import beam
import nexus

class PeggedOrder:
  """Trades an order passively at a price pegged to the touch, honoring the
  order's limit price. The order's type is superseded: every submission is a
  limit order.
  """

  def __init__(self, clients, order_fields, peg_difference):
    """Constructs a PeggedOrder.
    @param clients The clients used to access Nexus services.
    @param order_fields The specification of the order to execute.
    @param peg_difference The difference applied to the pegged price.
    """
    self._clients = clients
    self._order_fields = nexus.OrderFields(order_fields)
    self._peg_difference = peg_difference
    self._state = None
    self._failure = None
    self._orders = beam.QueueWriterPublisher()
    self._tasks = beam.RoutineTaskQueue()
    self._tasks.push(self._s0)

  @property
  def order_fields(self):
    """Returns a copy of the specification of the order being executed."""
    return nexus.OrderFields(self._order_fields)

  @property
  def peg_difference(self):
    """Returns the difference applied to the pegged price."""
    return self._peg_difference

  @property
  def orders(self):
    """Returns the publisher of the orders submitted."""
    return self._orders

  @property
  def filled_quantity(self):
    """Returns the quantity filled so far."""
    return self._filled_quantity

  def cancel(self):
    try:
      self._tasks.push(self._on_cancel)
    except beam.PipeBrokenException:
      pass

  def wait(self):
    self._tasks.wait()

  def _s0(self):
    self._state = 0
    self._filled_quantity = 0
    self._clients.market_data_client.query_bbo_quotes(
      beam.make_current_query(self._order_fields.ticker),
      self._tasks.get_slot(self._on_bbo_quote))

  def _s1(self):
    self._state = 1
    if self._filled_quantity == self._order_fields.quantity:

      # C0
      return self._s6()

    # epsilon
    return self._s2()

  def _s2(self):
    self._state = 2
    self._submission_price = self._peg_price
    fields = nexus.OrderFields(self._order_fields)
    fields.type = nexus.OrderType.LIMIT
    fields.price = self._submission_price
    fields.quantity = self._order_fields.quantity - self._filled_quantity
    self._order = self._clients.order_execution_client.submit(fields)
    self._order.publisher.monitor(
      self._tasks.get_slot(self._on_execution_report))
    self._orders.push(self._order)

  def _s3(self):
    self._state = 3
    self._orders.close(self._failure)
    self._tasks.close()

  def _s4(self):
    self._state = 4
    if nexus.pick(self._order_fields.side,
        self._submission_price > self._peg_price,
        self._submission_price < self._peg_price):

      # C1
      return self._s5()

  def _s5(self):
    self._state = 5
    self._clients.order_execution_client.cancel(self._order)

  def _s6(self):
    self._state = 6
    self._orders.close()
    self._tasks.close()

  def _s7(self):
    self._state = 7

  def _s8(self):
    self._state = 8
    self._clients.order_execution_client.cancel(self._order)

  def _on_bbo_quote(self, quote):

    # E0
    self._bbo = quote
    self._peg_price = nexus.pick(self._order_fields.side,
      self._bbo.ask.price, self._bbo.bid.price) - \
      nexus.direction(self._order_fields.side) * self._peg_difference
    if self._order_fields.price != nexus.Money.ZERO:
      self._peg_price = nexus.pick(self._order_fields.side,
        max(self._peg_price, self._order_fields.price),
        min(self._peg_price, self._order_fields.price))
    if self._state == 0:

      # E0
      return self._s1()
    elif self._state == 4:

      # E0
      return self._s4()

  def _on_execution_report(self, report):

    # E1
    self._filled_quantity += report.last_quantity
    if self._state == 2:
      if report.status == nexus.OrderStatus.REJECTED:

        # E2
        self._failure = RuntimeError(report.text)
        return self._s3()
      elif report.status == nexus.OrderStatus.NEW:

        # E3
        return self._s4()
      elif nexus.is_terminal(report.status):

        # E4
        return self._s1()
    elif self._state == 4:
      if nexus.is_terminal(report.status):

        # E4
        return self._s1()
    elif self._state == 5:
      if nexus.is_terminal(report.status):

        # E4
        return self._s1()
    elif self._state == 7:
      if report.status == nexus.OrderStatus.NEW:

        # E3
        return self._s8()
      elif nexus.is_terminal(report.status):

        # E4
        return self._s6()
    elif self._state == 8:
      if nexus.is_terminal(report.status):

        # E4
        return self._s6()

  def _on_cancel(self):
    if self._state == 0:

      # E5
      return self._s6()
    elif self._state == 2:

      # E5
      return self._s7()
    elif self._state == 4:

      # E5
      return self._s8()
    elif self._state == 5:

      # E5
      return self._s8()
