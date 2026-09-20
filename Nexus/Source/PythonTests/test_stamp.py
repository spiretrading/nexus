import gc
import unittest
import nexus


class TestStamp(unittest.TestCase):
    def test_field_reader(self):
        source = (b'\x01\x1e1=HEADER\x1c\x1e70.1=002\x1e55=ABX'
            b'\x1e70=001\x1e64=100\x1e41=\x1e40.9=ORDER')
        message = nexus.StampMessage.parse(source)
        section = message.business_content
        reader = nexus.StampFieldReader(section)
        del source, message, section
        gc.collect()
        self.assertEqual(reader.read(55, str), 'ABX')
        self.assertEqual(reader.read(64, int), 100)
        self.assertEqual(reader.read(70, 0, int), 1)
        self.assertEqual(reader.read(70, 1, int), 2)
        self.assertEqual(reader.read_optional(55, str), 'ABX')
        self.assertEqual(reader.read_optional(70, 1, int), 2)
        calls = []

        def parser(value):
            calls.append(value)
            return 'default'

        self.assertIsNone(reader.read_optional(9999, parser))
        self.assertIsNone(reader.read_optional(70, 2, parser))
        self.assertEqual(calls, [])
        self.assertEqual(reader.read_optional(41, parser), 'default')
        self.assertEqual(calls, [''])
        self.assertIsNone(reader.read(55, lambda value: None))
        self.assertEqual(reader.get_count([]), 0)
        self.assertEqual(reader.get_count([9999]), 0)
        self.assertEqual(reader.get_count([55]), 1)
        self.assertEqual(reader.get_count([55, 70]), 2)
        self.assertEqual(reader.get_count([40, 70]), 10)
        for read, arguments in (
                (reader.read, (9999, str)),
                (reader.read, (70, 2, str)),
                (reader.read, (70, str)),
                (reader.read_optional, (70, str))):
            with self.subTest(arguments=arguments):
                with self.assertRaises(nexus.StampParserException):
                    read(*arguments)

        def invalid(value):
            raise LookupError(value)

        for read in (reader.read, reader.read_optional):
            with self.subTest(read=read):
                with self.assertRaises(LookupError):
                    read(55, invalid)
        for duplicate in (b'55=ABX\x1e55=XYZ', b'55=ABX\x1e55.0=XYZ',
                b'70.1=001\x1e70.1=002'):
            with self.subTest(duplicate=duplicate):
                message = nexus.StampMessage.parse(
                    b'\x01\x1e1=HEADER\x1c\x1e' + duplicate)
                with self.assertRaises(nexus.StampParserException):
                    nexus.StampFieldReader(message.business_content)

    def test_field(self):
        for source, identifier, index, value in (
                (b'\x1e55=ABX', 55, 0, 'ABX'),
                (b'\x1e55.2=ABX', 55, 2, 'ABX'),
                (b'\x1e9999.9999=', 9999, 9999, ''),
                (b'\x1e1= a=b ', 1, 0, ' a=b ')):
            with self.subTest(source=source):
                field = nexus.StampField.parse(source)
                self.assertEqual(field.identifier, identifier)
                self.assertEqual(field.index, index)
                self.assertEqual(field.value, value)
                self.assertIsInstance(field.value, str)
        for name in ('identifier', 'index', 'value'):
            with self.subTest(name=name):
                with self.assertRaises(AttributeError):
                    setattr(field, name, getattr(field, name))

    def test_message(self):
        source = (b'\x01\x1e8=20260919070000\x1c\x1e6=OpeningAuction'
            b'\x1e55=ABX\x1e55.1=XYZ\x1e191=25.50\x1e9999=')
        for trailer in (b'', b'\x1d'):
            with self.subTest(trailer=trailer):
                message = nexus.StampMessage.parse(source + trailer)
                header = message.control_header
                content = message.business_content
                self.assertIsInstance(content, nexus.StampMessage.Section)
                self.assertEqual(header.find(8).value, '20260919070000')
                self.assertIsNone(header.find(55))
                self.assertIsNone(content.find(8))
                self.assertEqual(content.find(55).value, 'ABX')
                self.assertEqual(content.find(55, 0).value, 'ABX')
                self.assertEqual(content.find(55, 1).value, 'XYZ')
                self.assertIsNone(content.find(55, 2))
                self.assertEqual(content.find(191).value, '25.50')
                self.assertEqual(content.find(9999).value, '')
                self.assertIsNone(content.find(9998))
                for _ in range(2):
                    self.assertEqual([(field.identifier, field.index,
                        field.value) for field in content], [
                            (6, 0, 'OpeningAuction'), (55, 0, 'ABX'),
                            (55, 1, 'XYZ'), (191, 0, '25.50'), (9999, 0, '')])
                for name in ('control_header', 'business_content'):
                    with self.assertRaises(AttributeError):
                        setattr(message, name, getattr(message, name))

    def test_lifetime(self):
        value = 'x' * 4096
        source = b'\x1e55=' + value.encode('ascii')
        field = nexus.StampField.parse(source)
        del source
        gc.collect()
        self.assertEqual(field.value, value)
        source = b'\x01\x1e1=HEADER\x1c\x1e55=' + value.encode('ascii')
        source += b'\x1e55.1=SECOND'
        message = nexus.StampMessage.parse(source)
        header = message.control_header
        content = message.business_content
        del source, message
        gc.collect()
        self.assertEqual(header.find(1).value, 'HEADER')
        found = content.find(55)
        first = iter(content)
        second = iter(content)
        del header, content
        gc.collect()
        self.assertEqual(found.value, value)
        result = next(first)
        self.assertEqual(next(second).value, value)
        self.assertEqual(next(first).value, 'SECOND')
        del first, second
        gc.collect()
        self.assertEqual(result.value, value)
        text = result.value
        del result
        gc.collect()
        self.assertEqual(text, value)

    def test_malformed_input(self):
        self.assertTrue(issubclass(nexus.StampParserException, ValueError))
        for source in (b'', b'55=ABX', b'\x1e55', b'\x1e0=ABX',
                b'\x1e55.-1=ABX', b'\x1e55=\xff'):
            with self.subTest(field=source):
                with self.assertRaises(nexus.StampParserException):
                    nexus.StampField.parse(source)
        for source in (b'', b'\x01\x1e1=HEADER', b'\x01\x1c\x1e55=ABX',
                b'\x01\x1e1=HEADER\x1c',
                b'\x01\x1e1=HEADER\x1c\x1e55=ABX\x1e56',
                b'\x01\x1e1=HEADER\x1c\x1e55=ABX\x1e56=\x00'):
            with self.subTest(message=source):
                with self.assertRaises(nexus.StampParserException):
                    nexus.StampMessage.parse(source)
        for parser, source in (
                (nexus.StampField.parse, b'\x1e55=ABX'),
                (nexus.StampMessage.parse, b'\x01\x1e1=H\x1c\x1e55=ABX')):
            for value in (source.decode('ascii'), bytearray(source),
                    memoryview(source)):
                with self.subTest(parser=parser, type=type(value)):
                    with self.assertRaises(TypeError):
                        parser(value)


if __name__ == '__main__':
    unittest.main()
