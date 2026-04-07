import aspen
import beam
import nexus

class PeggedOrder:
  def __init__(self, clients, order_fields, offset):
    self.clients = clients
    self.order_fields = nexus.OrderFields(order_fields)
    self.submission_price = None
    self.offset = offset
    self.order = None
    self.filled_quantity = 0
    self.bbo_quote = None
    self.state = None
    self.tasks = beam.RoutineTaskQueue()

  def start(self):
    self.tasks.push(self.s0)

  def wait(self):
    self.tasks.wait()

  def calculate_expected_price(self):
    return nexus.pick(self.order_fields.side,
      self.bbo_quote.ask.price, self.bbo_quote.bid.price) - \
        nexus.direction(self.order_fields.side) * self.offset

  def s0(self):
    self.state = 0
    query = beam.make_current_query(self.order_fields.security)
    self.clients.market_data_client.query_bbo_quotes(
      query, self.tasks.get_slot(self.on_bbo_quote))

  def s1(self):
    self.state = 1
    self.submission_price = self.calculate_expected_price()
    order_fields = nexus.OrderFields(self.order_fields)
    order_fields.price = self.submission_price
    order_fields.quantity = self.order_fields.quantity - self.filled_quantity
    self.order = self.clients.order_execution_client.submit(order_fields)
    self.order.publisher.monitor(self.tasks.get_slot(self.on_execution_report))

  def s2(self):
    self.state = 2
    self.tasks.close()

  def s3(self):
    self.state = 3
    if self.submission_price != self.calculate_expected_price():

      # C0
      return self.s5()

  def s4(self):
    self.state = 4
    self.tasks.close()

  def s5(self):
    self.state = 5
    self.clients.order_execution_client.cancel(self.order)

  def on_bbo_quote(self, bbo_quote):
    self.bbo_quote = bbo_quote
    if self.state == 0:

      # E0
      return self.s1()
    elif self.state == 3:

      # E0
      return self.s3()

  def on_execution_report(self, execution_report):
    self.filled_quantity += execution_report.last_quantity
    if self.state == 1:
      if execution_report.status == nexus.OrderStatus.REJECTED:

        # E1
        return self.s2()
      elif execution_report.status == nexus.OrderStatus.NEW:

        # E2
        return self.s3()
    elif self.state == 3:
      if execution_report.status == nexus.OrderStatus.FILLED:

        # E3
        return self.s4()
    elif self.state == 5:
      if execution_report.status == nexus.OrderStatus.FILLED:

        # E3
        return self.s4()
      elif nexus.is_terminal(execution_report.status):

        # E4
        return self.s1()
