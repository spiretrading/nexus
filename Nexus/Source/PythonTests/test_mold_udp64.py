import gc
import struct
import unittest
import beam
import nexus


class TestMoldUdp64(unittest.TestCase):
    def test_request(self):
        for sequence, count in ((0x0102030405060708, 0x0910),
                (2**64 - 1, 65535)):
            with self.subTest(sequence=sequence, count=count):
                request = nexus.MoldUdp64Request(b'SESSION123', sequence, count)
                self.assertEqual(request.session, b'SESSION123')
                self.assertEqual(request.sequence_number, sequence)
                self.assertEqual(request.count, count)
                self.assertEqual(request.encode(),
                    struct.pack('!10sQH', b'SESSION123', sequence, count))
        for session in (b'', b'SHORT', b'SESSION1234'):
            with self.subTest(session=session):
                with self.assertRaises(ValueError):
                    nexus.MoldUdp64Request(session, 1, 1)
        with self.assertRaises(TypeError):
            nexus.MoldUdp64Request('SESSION123', 1, 1)

    def test_parse(self):
        source = struct.pack('!10sQH', b'SESSION123', 0x0102030405060708, 3)
        source += b'\x00\x03X\x00Y\x00\x00\x00\x01\xff'
        packet = nexus.MoldUdp64Packet.parse(source)
        self.assertEqual(packet.session, b'SESSION123')
        self.assertEqual(packet.sequence_number, 0x0102030405060708)
        self.assertEqual(packet.count, 3)
        self.assertFalse(packet.is_heartbeat)
        self.assertFalse(packet.is_end_of_session)
        self.assertEqual(list(packet), [b'X\x00Y', b'', b'\xff'])
        self.assertEqual(list(packet), [b'X\x00Y', b'', b'\xff'])

    def test_lifetime(self):
        payload = bytes(range(256)) * 16
        source = struct.pack('!10sQHH', b'SESSION123', 1, 1, len(payload))
        source += payload
        packet = nexus.MoldUdp64Packet.parse(source)
        del source
        gc.collect()
        self.assertEqual(list(packet), [payload])
        first = iter(packet)
        second = iter(packet)
        del packet
        gc.collect()
        result = next(first)
        self.assertEqual(next(second), payload)
        del first, second
        gc.collect()
        self.assertIsInstance(result, bytes)
        self.assertEqual(result, payload)

    def test_control_packets(self):
        for count in (0, 65535):
            with self.subTest(count=count):
                source = struct.pack('!10sQH', b'SESSION123', 2**64 - 1, count)
                packet = nexus.MoldUdp64Packet.parse(source)
                self.assertEqual(packet.sequence_number, 2**64 - 1)
                self.assertEqual(packet.count, count)
                self.assertEqual(packet.is_heartbeat, count == 0)
                self.assertEqual(packet.is_end_of_session, count == 65535)
                self.assertEqual(list(packet), [])

    def test_malformed_packet(self):
        source = struct.pack('!10sQH', b'SESSION123', 1, 1) + b'\x00\x01X'
        for size in range(len(source)):
            with self.subTest(size=size):
                with self.assertRaises(nexus.MoldUdp64ParserException):
                    nexus.MoldUdp64Packet.parse(source[:size])
        for count in (0, 2, 65535):
            with self.subTest(count=count):
                source = struct.pack('!10sQH', b'SESSION123', 1, count)
                source += b'\x00\x01X'
                with self.assertRaises(nexus.MoldUdp64ParserException):
                    nexus.MoldUdp64Packet.parse(source)
        source = struct.pack('!10sQH', b'SESSION123', 1, 1) + b'\x00\x01XY'
        with self.assertRaises(nexus.MoldUdp64ParserException):
            nexus.MoldUdp64Packet.parse(source)

    def test_immutable_packet(self):
        source = struct.pack('!10sQH', b'SESSION123', 1, 0)
        for value in (source.decode('ascii'), bytearray(source),
                memoryview(source)):
            with self.subTest(type=type(value)):
                with self.assertRaises(TypeError):
                    nexus.MoldUdp64Packet.parse(value)
        packet = nexus.MoldUdp64Packet.parse(source)
        for field in ('session', 'sequence_number', 'count', 'is_heartbeat',
                'is_end_of_session'):
            with self.subTest(field=field):
                with self.assertRaises(AttributeError):
                    setattr(packet, field, getattr(packet, field))


class TestMoldUdp64Client(unittest.TestCase):
    def setUp(self):
        self.routines = beam.RoutineHandlerGroup()
        self.addCleanup(self.routines.wait)
        self.server = beam.LocalServerConnection()
        self.addCleanup(self.server.close)
        self.addCleanup(self.close_client)

        def connect():
            self.channel = beam.LocalClientChannel('mold_udp64', self.server)

        self.routines.spawn(connect)
        self.sender = beam.ServerConnection(self.server).accept()
        self.addCleanup(self.sender.connection.close)
        self.routines.wait()
        self.client = nexus.MoldUdp64Client(self.channel)

    def close_client(self):
        if hasattr(self, 'client'):
            self.client.close()
        elif hasattr(self, 'channel'):
            self.channel.connection.close()

    def test_read(self):
        del self.channel
        gc.collect()
        source = struct.pack('!10sQH', b'SESSION123', 42, 2)
        source += b'\x00\x03ONE\x00\x03TWO'
        self.sender.writer.write(source)
        first = self.client.read()
        source = struct.pack('!10sQH', b'SESSION456', 1, 1) + b'\x00\x01X'
        self.sender.writer.write(source)
        second = self.client.read()
        self.client.close()
        del self.client
        gc.collect()
        self.assertEqual(first.session, b'SESSION123')
        self.assertEqual(first.sequence_number, 42)
        self.assertEqual(first.count, 2)
        self.assertEqual(list(first), [b'ONE', b'TWO'])
        self.assertEqual(second.session, b'SESSION456')
        self.assertEqual(second.sequence_number, 1)
        self.assertEqual(list(second), [b'X'])

    def test_request(self):
        self.client.request(nexus.MoldUdp64Request(b'SESSION123', 42, 100))
        source = beam.SharedBuffer()
        self.sender.reader.read(source)
        self.assertEqual(source.get_data(),
            struct.pack('!10sQH', b'SESSION123', 42, 100))
        response = struct.pack('!10sQH', b'SESSION123', 42, 2)
        response += b'\x00\x03ONE\x00\x03TWO'
        self.sender.writer.write(response)
        packet = self.client.read()
        self.assertEqual(packet.sequence_number, 42)
        self.assertEqual(list(packet), [b'ONE', b'TWO'])
        self.client.request(nexus.MoldUdp64Request(b'SESSION123', 44, 98))
        source.reset()
        self.sender.reader.read(source)
        self.assertEqual(source.get_data(),
            struct.pack('!10sQH', b'SESSION123', 44, 98))
        self.assertEqual(list(packet), [b'ONE', b'TWO'])
        self.client.close()
        with self.assertRaises(beam.IOException):
            self.client.request(nexus.MoldUdp64Request(b'SESSION123', 44, 98))

    def test_close_pending_read(self):
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

    def test_control_packets(self):
        for count in (0, 65535):
            with self.subTest(count=count):
                source = struct.pack('!10sQH', b'SESSION123', 42, count)
                self.sender.writer.write(source)
                packet = self.client.read()
                self.assertEqual(packet.session, b'SESSION123')
                self.assertEqual(packet.sequence_number, 42)
                self.assertEqual(packet.count, count)
                self.assertEqual(packet.is_heartbeat, count == 0)
                self.assertEqual(packet.is_end_of_session, count == 65535)
                self.assertEqual(list(packet), [])

    def test_malformed_packet(self):
        self.sender.writer.write(b'SHORT')
        with self.assertRaises(nexus.MoldUdp64ParserException):
            self.client.read()
        source = struct.pack('!10sQH', b'SESSION123', 42, 1) + b'\x00\x01X'
        self.sender.writer.write(source)
        packet = self.client.read()
        self.assertEqual(packet.sequence_number, 42)
        self.assertEqual(list(packet), [b'X'])


if __name__ == '__main__':
    unittest.main()
