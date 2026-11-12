import csv
import signal
import tempfile
import time
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch
from main import LineBuffer, parse_line, Recorder, State, render, run
from rich.console import Console
from io import StringIO

LINE = b'T=15230,L=10.2,F=35.0,R=400.0,ACT=FWD'


class Tests(unittest.TestCase):
    def test_contract(self):
        values = parse_line(LINE)
        self.assertEqual(values, dict(T=15230, L=10.2, F=35., R=400., ACT='FWD'))
        self.assertEqual(parse_line(b'ACT=STOP,R=0,F=2,L=3,T=0\r\n')['ACT'], 'STOP')

    def test_invalid_lines(self):
        for raw in [b'bad', b'', LINE+b',T=99', LINE.replace(b'F=35.0,', b''),
                    LINE.replace(b'10.2', b'nan'), LINE.replace(b'10.2', b'-1'),
                    LINE.replace(b'FWD', b'RUN'), LINE.replace(b'T=15230', b'T=1.2'),
                    LINE+b',foo=1', LINE+b'\xff']:
            with self.subTest(raw=raw), self.assertRaises((ValueError, UnicodeError)):
                parse_line(raw)

    def test_fragmented_multiple_and_oversized_lines(self):
        buffer = LineBuffer()
        self.assertEqual(buffer.feed(LINE[:10]), [])
        self.assertEqual(buffer.feed(LINE[10:]+b'\r\n'+LINE+b'\n'), [LINE+b'\r', LINE])
        self.assertEqual(buffer.feed(b'x'*1500), [])
        self.assertLessEqual(len(buffer.buffer), 1024)
        self.assertEqual(buffer.feed(b'ignored\n'+LINE+b'\n'), [LINE])
        self.assertEqual(buffer.errors, 1)

    def test_csv_and_invalid_not_recorded(self):
        with tempfile.TemporaryDirectory() as folder:
            rec = Recorder(Path(folder), 'sim')
            other = Recorder(Path(folder), 'sim')
            self.assertNotEqual(rec.path, other.path)
            state = State('sim', recorder=rec)
            state.accept(b'bad')
            state.accept(LINE)
            rec.close()
            other.close()
            with rec.path.open() as stream:
                rows = list(csv.DictReader(stream))
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]['R'], '400.0')
            self.assertEqual(rows[0]['ACT'], 'FWD')
            self.assertEqual(state.invalid, 1)
            self.assertEqual(rec.count, 1)

    def test_stale_display(self):
        state = State('sim', active=True, last=parse_line(LINE), received=time.monotonic()-3)
        stream = StringIO()
        Console(file=stream, width=100).print(render(state, 2))
        self.assertIn('GÜNCEL DEĞİL', stream.getvalue())
        self.assertIn('Yankı yok / yol açık', stream.getvalue())

    def test_serial_error_closes_port_and_csv(self):
        class Port:
            closed = False
            calls = 0
            def read(self, size):
                self.calls += 1
                if self.calls == 1:
                    return LINE+b'\n'
                raise OSError('test connection lost')
            def close(self):
                self.closed = True
        port = Port()
        with tempfile.TemporaryDirectory() as folder:
            args = SimpleNamespace(mode='serial', port='TEST', baud=9600, output=Path(folder),
                                   stale_after=2, idle_timeout=30, duration=1, hz=5)
            with patch('main.serial.Serial', return_value=port), patch('main.Console', return_value=Console(file=StringIO())):
                self.assertEqual(run(args), 1)
            self.assertTrue(port.closed)
            file = next(Path(folder).glob('*.csv'))
            # File can be reopened; final row survived connection failure.
            with file.open() as stream:
                self.assertEqual(len(list(csv.DictReader(stream))), 1)

    def test_serial_idle_timeout_closes(self):
        class Port:
            closed = False
            def read(self, size): return b''
            def close(self): self.closed = True
        port = Port()
        with tempfile.TemporaryDirectory() as folder:
            args = SimpleNamespace(mode='serial', port='TEST', baud=9600, output=Path(folder),
                                   stale_after=.01, idle_timeout=.03, duration=1, hz=5)
            with patch('main.serial.Serial', return_value=port), patch('main.Console', return_value=Console(file=StringIO())):
                self.assertEqual(run(args), 1)
            self.assertTrue(port.closed)


if __name__ == '__main__':
    unittest.main()
