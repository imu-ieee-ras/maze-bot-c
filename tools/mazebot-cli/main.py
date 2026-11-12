"""MazeBot HC-06 terminal monitor. Python 3.10+."""
import argparse
import csv
import math
import re
import signal
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from threading import Event
from uuid import uuid4

import serial
from serial.tools import list_ports
from rich.console import Console, Group
from rich.live import Live
from rich.panel import Panel
from rich.table import Table
from rich.text import Text

KEYS = ('T', 'L', 'F', 'R', 'ACT')
FIELDS = ('received_at_utc', 'source', *KEYS)
ACTIONS = {'FWD': 'İleri', 'TURN_L': 'Sola dön', 'TURN_R': 'Sağa dön',
           'BACK': 'Geri', 'STOP': 'Dur'}


def parse_line(raw: bytes) -> dict:
    text = raw.decode('ascii').strip()
    values = {}
    for field in text.split(','):
        if field.count('=') != 1:
            raise ValueError('Her alan key=value olmalı')
        key, value = (part.strip() for part in field.split('='))
        if key not in KEYS or key in values or not value:
            raise ValueError(f'Eksik, bilinmeyen veya tekrarlı alan: {key}')
        values[key] = value
    if set(values) != set(KEYS):
        raise ValueError('T,L,F,R,ACT alanları gerekli')
    if not re.fullmatch(r'[0-9]+', values['T']):
        raise ValueError('T negatif olmayan tam sayı (ms) olmalı')
    result = {'T': int(values['T'])}
    for key in ('L', 'F', 'R'):
        value = float(values[key])
        if not math.isfinite(value) or value < 0:
            raise ValueError(f'{key} negatif olmayan sonlu sayı olmalı')
        result[key] = value
    if values['ACT'] not in ACTIONS:
        raise ValueError('ACT: FWD, TURN_L, TURN_R, BACK veya STOP olmalı')
    result['ACT'] = values['ACT']
    return result


class LineBuffer:
    """Preserve fragmented lines, bound memory, resync after overlong lines."""
    def __init__(self, limit=1024):
        self.buffer = bytearray()
        self.limit = limit
        self.dropping = False
        self.errors = 0

    def feed(self, chunk):
        lines = []
        for byte in chunk:
            if byte == 10:
                if not self.dropping and self.buffer.strip():
                    lines.append(bytes(self.buffer))
                self.buffer.clear()
                self.dropping = False
            elif not self.dropping:
                self.buffer.append(byte)
                if len(self.buffer) > self.limit:
                    self.buffer.clear()
                    self.dropping = True
                    self.errors += 1
        return lines


class Recorder:
    def __init__(self, directory: Path, source: str):
        directory.mkdir(parents=True, exist_ok=True)
        stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
        self.path = directory / f'{source}_{stamp}_{uuid4().hex[:6]}.csv'
        self.file = self.path.open('x', encoding='utf-8', newline='')
        self.source = source
        self.count = 0
        try:
            self.writer = csv.DictWriter(self.file, fieldnames=FIELDS)
            self.writer.writeheader()
            self.file.flush()
        except BaseException:
            self.file.close()
            raise

    def write(self, values):
        self.writer.writerow({'received_at_utc': datetime.now(timezone.utc).isoformat(
            timespec='milliseconds'), 'source': self.source, **values})
        self.file.flush()
        self.count += 1

    def close(self):
        self.file.close()


@dataclass
class State:
    mode: str
    status: str = 'Hazırlanıyor'
    active: bool = False
    last: dict | None = None
    received: float | None = None
    invalid: int = 0
    error: str = ''
    recorder: Recorder | None = None

    def accept(self, raw):
        try:
            values = parse_line(raw)
        except (ValueError, UnicodeError, OverflowError) as error:
            self.invalid += 1
            self.error = f'Satır atlandı: {error}'
            return
        self.recorder.write(values)
        self.last = values
        self.received = time.monotonic()


def render(state, stale_after):
    age = None if state.received is None else time.monotonic() - state.received
    fresh = state.active and age is not None and age < stale_after
    mode = 'SİMÜLASYON • GERÇEK ROBOT VERİSİ DEĞİL' if state.mode == 'sim' else 'HC-06 • SERİ PORT'
    table = Table(expand=True)
    table.add_column('Alan')
    table.add_column('Değer', justify='right')
    table.add_column('Açıklama')
    values = state.last
    table.add_row('T', '—' if values is None else str(values['T']), 'Robot süresi (ms)')
    for key, label in [('L', 'Sol'), ('F', 'Ön'), ('R', 'Sağ')]:
        value = '—' if values is None else ('Yankı yok / yol açık' if values[key] == 400
                                            else f'{values[key]:.1f} cm')
        table.add_row(key, value, label)
    table.add_row('ACT', '—' if values is None else values['ACT'],
                  '—' if values is None else ACTIONS[values['ACT']])
    if not fresh:
        table.style = 'dim'
    info = Table.grid(padding=(0, 2))
    rec = state.recorder
    for label, value in [
        ('Durum', state.status), ('Veri', 'GÜNCEL' if fresh else 'VERİ YOK / GÜNCEL DEĞİL'),
        ('Son geçerli veri', '—' if age is None else f'{age:.1f} saniye önce'),
        ('CSV kaydı', 'AÇIK' if rec and not rec.file.closed else 'KAPALI'),
        ('Kayıt sayısı', str(rec.count if rec else 0)), ('Hatalı satır', str(state.invalid)),
        ('Dosya', str(rec.path) if rec else '—'),
    ]:
        info.add_row(Text(label), Text(value))
    parts = [Panel(Text(mode, style='bold cyan'), title='MazeBot • TEAM-C'), table, Panel(info)]
    if state.error:
        parts.append(Panel(Text(state.error, style='yellow'), title='Son uyarı / hata'))
    parts.append(Text('Çıkış: Ctrl+C  |  400 cm = yankı yok / yol açık', style='dim'))
    return Group(*parts)


def simulation_line(t):
    # Display scenarios only; these do not implement a maze-solving algorithm.
    index = int(t / 3) % 5
    action, left, front, right = [
        ('FWD', 10.2, 35.0, 400.0), ('TURN_L', 70.0, 8.0, 10.0),
        ('TURN_R', 10.0, 8.0, 70.0), ('BACK', 5.0, 4.0, 5.0),
        ('STOP', 10.0, 10.0, 10.0),
    ][index]
    return f'T={int(t*1000)},L={left},F={front},R={right},ACT={action}\n'.encode('ascii')


def run(args):
    state = State(args.mode)
    stop = Event()
    old_handlers = {}
    port = None
    framer = LineBuffer()
    console = Console()
    exit_code = 0
    for sig in (signal.SIGINT, signal.SIGTERM):
        old_handlers[sig] = signal.signal(sig, lambda *_: stop.set())
    with Live(render(state, args.stale_after), console=console, refresh_per_second=4) as live:
        try:
            if args.mode == 'serial':
                state.status = f'Port açılıyor: {args.port}'
                live.update(render(state, args.stale_after), refresh=True)
                port = serial.Serial(args.port, baudrate=args.baud, timeout=0)
                state.status = f'Port açık: {args.port} • {args.baud} baud • veri bekleniyor'
            else:
                state.status = 'Simülasyon çalışıyor'
            state.recorder = Recorder(args.output, args.mode)
            state.active = True
            start = time.monotonic()
            next_sample = start
            next_render = start
            while not stop.is_set():
                now = time.monotonic()
                if args.duration and now - start >= args.duration:
                    break
                chunk = b''
                if port:
                    chunk = port.read(4096)
                elif now >= next_sample:
                    chunk = simulation_line(now - start)
                    next_sample = now + 1 / args.hz
                before = framer.errors
                for line in framer.feed(chunk):
                    state.accept(line)
                if framer.errors > before:
                    state.invalid += framer.errors - before
                    state.error = '1024 baytı aşan satır atlandı; satır sonu bekleniyor'
                if args.mode == 'serial':
                    reference = state.received if state.received is not None else start
                    if now - reference >= args.idle_timeout:
                        state.status = 'Geçerli veri zaman aşımı • port kapatıldı'
                        state.error = 'Bağlantı, baud ve satır biçimini kontrol et. Fiziksel kopma kesinleştirilemedi.'
                        exit_code = 1
                        break
                if now >= next_render:
                    live.update(render(state, args.stale_after))
                    next_render = now + .25
                stop.wait(.01)
            if exit_code == 0:
                state.status = 'Durduruldu'
        except (OSError, serial.SerialException, ValueError) as error:
            state.error = str(error)
            state.status = 'Hata • durduruldu'
            exit_code = 1
        finally:
            state.active = False
            try:
                if port is not None:
                    port.close()
            finally:
                try:
                    if state.recorder:
                        state.recorder.close()
                finally:
                    for sig, handler in old_handlers.items():
                        signal.signal(sig, handler)
                    live.update(render(state, args.stale_after), refresh=True)
    return exit_code


def positive_float(value):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError('Pozitif sonlu sayı gerekli')
    return number


def positive_int(value):
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError('Pozitif tam sayı gerekli')
    return number


def parse_args():
    parser = argparse.ArgumentParser(description='MazeBot HC-06 canlı ekran ve CSV kaydı')
    parser.add_argument('--mode', choices=['sim', 'serial'], default='sim')
    parser.add_argument('--port', help='Windows: COM5, Linux: yapılandırılan /dev/rfcomm0 gibi')
    parser.add_argument('--baud', type=positive_int, help='Firmware ekibinden doğrulanacak baud hızı')
    parser.add_argument('--output', type=Path, default=Path('logs'))
    parser.add_argument('--duration', type=positive_float, help='Bu saniye sonunda çık')
    parser.add_argument('--hz', type=positive_float, default=5, help='Simülasyon veri sıklığı')
    parser.add_argument('--stale-after', type=positive_float, default=2)
    parser.add_argument('--idle-timeout', type=positive_float, default=30,
                        help='Seri modda geçerli veri gelmezse bu saniye sonunda kapat')
    parser.add_argument('--list-ports', action='store_true', help='Seri portları listele ve çık')
    args = parser.parse_args()
    if args.mode == 'serial' and not args.list_ports and (not args.port or not args.baud):
        parser.error('Seri mod için --port ve --baud gerekli; gerçek değerleri doğrula')
    return args


if __name__ == '__main__':
    arguments = parse_args()
    if arguments.list_ports:
        ports = list(list_ports.comports())
        for item in ports:
            Console().print(Text(f'{item.device}: {item.description}'))
        if not ports:
            Console().print('Seri port bulunamadı.')
    else:
        raise SystemExit(run(arguments))
