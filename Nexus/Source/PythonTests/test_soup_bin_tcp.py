import datetime
import gc
import struct
import unittest
import beam
import nexus


def frame(packet_type, payload=b''):
    return struct.pack('!HB', len(payload) + 1, ord(packet_type)) + payload


class TestSoupBinTcp(unittest.TestCase):
    def test_packet(self):
        source = frame('S', b'X\x00\xff')
        packet = nexus.SoupBinTcpPacket.parse(source)
        del source
        gc.collect()
        self.assertEqual(packet.length, 4)
        self.assertEqual(packet.type, ord('S'))
        self.assertEqual(packet.payload, b'X\x00\xff')
        payload = packet.payload
        del packet
        gc.collect()
        self.assertEqual(payload, b'X\x00\xff')
        self.assertEqual(nexus.SoupBinTcpPacket.parse(frame('H')).payload, b'')

    def test_malformed_packet(self):
        source = frame('S', b'XYZ')
        for size in range(len(source)):
            with self.subTest(size=size):
                with self.assertRaises(nexus.SoupBinTcpParserException):
                    nexus.SoupBinTcpPacket.parse(source[:size])
        for source in (b'\x00\x00', frame('H') + b'x'):
            with self.subTest(source=source):
                with self.assertRaises(nexus.SoupBinTcpParserException):
                    nexus.SoupBinTcpPacket.parse(source)
        with self.assertRaises(TypeError):
            nexus.SoupBinTcpPacket.parse('text')

    def test_login_packets(self):
        accepted = nexus.SoupBinTcpPacket.parse(
            frame('A', b'SESSION123'.rjust(10) + b'42'.rjust(20)))
        login = nexus.parse_login_accepted_packet(accepted)
        self.assertEqual(login.session, 'SESSION123')
        self.assertEqual(login.sequence_number, 42)
        rejected = nexus.SoupBinTcpPacket.parse(frame('J', b'A'))
        self.assertEqual(nexus.parse_login_rejected_packet(rejected).reason,
            'A')
        for packet in (rejected, nexus.SoupBinTcpPacket.parse(frame('A'))):
            with self.assertRaises(nexus.SoupBinTcpParserException):
                nexus.parse_login_accepted_packet(packet)
        with self.assertRaises(nexus.SoupBinTcpParserException):
            nexus.parse_login_rejected_packet(accepted)
        self.assertEqual(nexus.make_login_request_packet(
            'user', 'pass', 'SESSION', 123), frame('L',
                b'user  pass      SESSION   123                 '))
        with self.assertRaises(nexus.SoupBinTcpParserException):
            nexus.make_login_request_packet('TOO_LONG', 'pass', '', 1)
        self.assertEqual(nexus.make_client_heartbeat_packet(), frame('R'))


class TestSoupBinTcpClient(unittest.TestCase):
    def setUp(self):
        self.routines = beam.RoutineHandlerGroup()
        self.addCleanup(self.routines.wait)
        self.server = beam.LocalServerConnection()
        self.addCleanup(self.server.close)
        self.clock = beam.tests.TimeServiceTestEnvironment()
        self.addCleanup(self.clock.close)
        self.addCleanup(self.close_client)

        def connect():
            self.channel = beam.LocalClientChannel('soup_bin_tcp', self.server)

        self.routines.spawn(connect)
        self.sender = beam.ServerConnection(self.server).accept()
        self.addCleanup(self.sender.connection.close)
        self.routines.wait()
        self.timer = beam.tests.TestTimer(datetime.timedelta(seconds=1),
            self.clock)

    def close_client(self):
        if hasattr(self, 'client'):
            self.client.close()
        elif hasattr(self, 'channel'):
            self.channel.connection.close()

    def log_in(self, *args):
        self.sender.writer.write(
            frame('A', b'SESSION123'.rjust(10) + b'42'.rjust(20)))
        self.client = nexus.SoupBinTcpClient(
            'user', 'pass', *args, self.channel, self.timer)
        request = beam.SharedBuffer()
        self.sender.reader.read(request)
        return request.get_data()

    def test_read(self):
        self.assertEqual(self.log_in(),
            nexus.make_login_request_packet('user', 'pass', '', 1))
        self.assertEqual(self.client.session, 'SESSION123')
        self.assertEqual(self.client.sequence_number, 42)
        del self.channel, self.timer
        gc.collect()
        self.sender.writer.write(frame('S', b'X\x00\xff') + frame('H'))
        first = self.client.read()
        self.assertEqual(self.client.sequence_number, 43)
        second = self.client.read()
        self.assertEqual(self.client.sequence_number, 43)
        self.client.close()
        del self.client
        gc.collect()
        self.assertEqual(first.type, ord('S'))
        self.assertEqual(first.payload, b'X\x00\xff')
        self.assertEqual(second.type, ord('H'))
        self.assertEqual(second.payload, b'')

    def test_resume(self):
        self.assertEqual(self.log_in('SESSION123', 42),
            nexus.make_login_request_packet('user', 'pass', 'SESSION123', 42))
        self.assertEqual(self.client.session, 'SESSION123')
        self.assertEqual(self.client.sequence_number, 42)
        self.clock.advance(datetime.timedelta(seconds=1))
        beam.flush_pending_routines()
        heartbeat = beam.SharedBuffer()
        self.sender.reader.read(heartbeat)
        self.assertEqual(heartbeat.get_data(), frame('R'))

    def test_close_pending_read(self):
        self.log_in()
        result = beam.Queue()

        def read():
            try:
                result.push(self.client.read())
            except Exception as error:
                result.close(error)

        self.routines.spawn(read)
        beam.flush_pending_routines()
        self.assertIsNone(result.try_pop())
        self.client.close()
        self.routines.wait()
        with self.assertRaises(beam.IOException):
            result.pop()
        self.client.close()

    def test_login_rejection(self):
        self.sender.writer.write(frame('J', b'A'))
        with self.assertRaises(beam.ConnectException):
            nexus.SoupBinTcpClient('user', 'pass', self.channel, self.timer)

    def test_login_timeout(self):
        result = beam.Queue()

        def connect():
            try:
                self.client = nexus.SoupBinTcpClient(
                    'user', 'pass', self.channel, self.timer)
                result.push(self.client)
            except Exception as error:
                result.close(error)

        self.routines.spawn(connect)
        beam.flush_pending_routines()
        self.assertIsNone(result.try_pop())
        request = beam.SharedBuffer()
        self.sender.reader.read(request)
        for _ in range(nexus.SoupBinTcpClient.TIMEOUT_TICKS):
            self.sender.writer.write(frame('+', b'debug'))
            self.clock.advance(datetime.timedelta(seconds=1))
        self.routines.wait()
        with self.assertRaises(beam.ConnectException):
            result.pop()

    def test_receive_timeout(self):
        self.log_in()
        for _ in range(nexus.SoupBinTcpClient.TIMEOUT_TICKS + 1):
            self.clock.advance(datetime.timedelta(seconds=1))
        with self.assertRaises(beam.IOException):
            self.client.read()


if __name__ == '__main__':
    unittest.main()
