#!/usr/bin/env python3
"""Browser-based serial monitor for SensorNode FSM heap trace logs."""

import argparse
import json
import re
import threading
import webbrowser
from collections import deque
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Deque, Dict, List, Optional, Tuple
from urllib.parse import parse_qs, urlparse

import serial
from serial.tools import list_ports


HEAP_LOG_MARKERS = ("[FSM][HEAP]", "[DT][HEAP]")
ANSI_ESCAPE_PATTERN = re.compile(r"\x1b\[[0-9;]*m")

INDEX_HTML = r"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SensorNode FSM Heap Monitor</title>
  <style>
    :root { color-scheme: dark; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; }
    * { box-sizing: border-box; }
    body { margin: 0; background: #111; color: #e8e8e8; min-height: 100vh; display: flex; flex-direction: column; }
    header { padding: 14px 18px; border-bottom: 1px solid #333; display: flex; gap: 10px; align-items: center; flex-wrap: wrap; }
    h1 { font-size: 17px; margin: 0 18px 0 0; font-weight: 600; }
    label { color: #aaa; font-size: 13px; }
    select, button { height: 34px; border: 1px solid #444; border-radius: 6px; background: #202020; color: #eee; padding: 0 10px; }
    select { min-width: 260px; }
    #baud { min-width: 110px; }
    button { cursor: pointer; }
    button:hover { background: #2b2b2b; }
    button.primary { background: #1769aa; border-color: #287fc1; }
    button.primary:hover { background: #1d78bf; }
    button:disabled, select:disabled { opacity: .55; cursor: default; }
    main { flex: 1; min-height: 0; padding: 12px 18px; }
    #logs { width: 100%; height: 100%; min-height: 420px; margin: 0; padding: 14px; overflow: auto; white-space: pre; border: 1px solid #333; border-radius: 6px; background: #090909; color: #d8e8d8; font: 12px/1.55 Menlo, Monaco, monospace; }
    footer { padding: 8px 18px 12px; color: #999; font-size: 12px; display: flex; justify-content: space-between; gap: 12px; }
    #status.connected { color: #69d07c; }
    #status.error { color: #ff7b72; }
  </style>
</head>
<body>
  <header>
    <h1>SensorNode FSM Heap Monitor</h1>
    <label for="ports">USB port</label>
    <select id="ports"></select>
    <button id="refresh">Refresh</button>
    <label for="baud">Baud</label>
    <select id="baud">
      <option>9600</option><option>19200</option><option>38400</option>
      <option>57600</option><option selected>115200</option><option>230400</option>
    </select>
    <button id="connect" class="primary">Connect</button>
    <button id="clear">Clear</button>
  </header>
  <main><pre id="logs"></pre></main>
  <footer><span id="status">Disconnected</span><span>Filter: [FSM][HEAP], [DT][HEAP]</span></footer>
  <script>
    const ports = document.querySelector('#ports');
    const baud = document.querySelector('#baud');
    const connect = document.querySelector('#connect');
    const refresh = document.querySelector('#refresh');
    const logs = document.querySelector('#logs');
    const status = document.querySelector('#status');
    let connected = false;
    let lastId = 0;

    async function api(path, options = {}) {
      const response = await fetch(path, options);
      const data = await response.json();
      if (!response.ok) throw new Error(data.error || `HTTP ${response.status}`);
      return data;
    }

    function setStatus(text, kind = '') {
      status.textContent = text;
      status.className = kind;
    }

    function updateControls(value) {
      connected = value;
      connect.textContent = value ? 'Disconnect' : 'Connect';
      ports.disabled = value;
      baud.disabled = value;
      refresh.disabled = value;
    }

    async function loadPorts() {
      try {
        const selected = ports.value;
        const data = await api('/api/ports');
        ports.replaceChildren();
        for (const port of data.ports) {
          const option = document.createElement('option');
          option.value = port.device;
          option.textContent = port.description ? `${port.device} - ${port.description}` : port.device;
          ports.append(option);
        }
        if (data.ports.some(port => port.device === selected)) ports.value = selected;
        if (!data.ports.length) setStatus('No USB serial ports found', 'error');
      } catch (error) {
        setStatus(error.message, 'error');
      }
    }

    async function toggleConnection() {
      connect.disabled = true;
      try {
        if (connected) {
          await api('/api/disconnect', { method: 'POST' });
          updateControls(false);
          setStatus('Disconnected');
        } else {
          if (!ports.value) throw new Error('Select a USB serial port first.');
          const data = await api('/api/connect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ port: ports.value, baud: Number(baud.value) })
          });
          updateControls(true);
          setStatus(`Connected: ${data.port} at ${data.baud} baud`, 'connected');
        }
      } catch (error) {
        setStatus(error.message, 'error');
      } finally {
        connect.disabled = false;
      }
    }

    async function poll() {
      try {
        const data = await api(`/api/logs?after=${lastId}`);
        if (data.logs.length) {
          const atBottom = logs.scrollTop + logs.clientHeight >= logs.scrollHeight - 24;
          logs.textContent += data.logs.map(item => item.line).join('\n') + '\n';
          lastId = data.logs[data.logs.length - 1].id;
          if (atBottom) logs.scrollTop = logs.scrollHeight;
        }
        if (connected && !data.connected) {
          updateControls(false);
          setStatus(data.error || 'Serial connection closed', 'error');
        }
      } catch (error) {
        setStatus(error.message, 'error');
      } finally {
        window.setTimeout(poll, 250);
      }
    }

    refresh.addEventListener('click', loadPorts);
    connect.addEventListener('click', toggleConnection);
    document.querySelector('#clear').addEventListener('click', () => { logs.textContent = ''; });
    loadPorts();
    poll();
  </script>
</body>
</html>
"""


class SerialMonitor:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._connection: Optional[serial.Serial] = None
        self._stop = threading.Event()
        self._logs: Deque[Tuple[int, str]] = deque(maxlen=5000)
        self._next_log_id = 1
        self._error = ""

    def ports(self) -> List[Dict[str, str]]:
        ports = []
        for port in list_ports.comports():
            searchable = f"{port.device} {port.description} {port.hwid}".lower()
            if "usb" in searchable or "wch" in searchable:
                ports.append({"device": port.device, "description": port.description or ""})
        return sorted(ports, key=lambda item: item["device"])

    def connect(self, port: str, baud: int) -> None:
        self.disconnect()
        connection = serial.Serial(port, baudrate=baud, timeout=0.25)
        stop = threading.Event()
        with self._lock:
            self._connection = connection
            self._error = ""
            self._stop = stop
        threading.Thread(target=self._read_loop, args=(connection, stop), daemon=True).start()

    def disconnect(self) -> None:
        self._stop.set()
        with self._lock:
            connection = self._connection
            self._connection = None
        if connection is not None:
            try:
                connection.close()
            except (OSError, serial.SerialException):
                pass

    def _read_loop(self, connection: serial.Serial, stop: threading.Event) -> None:
        try:
            while not stop.is_set():
                raw_line = connection.readline()
                if not raw_line:
                    continue
                line = ANSI_ESCAPE_PATTERN.sub("", raw_line.decode("utf-8", errors="replace")).rstrip("\r\n")
                if any(marker in line for marker in HEAP_LOG_MARKERS):
                    with self._lock:
                        self._logs.append((self._next_log_id, line))
                        self._next_log_id += 1
        except (OSError, serial.SerialException) as exc:
            if not stop.is_set():
                with self._lock:
                    if self._connection is connection:
                        self._error = str(exc)
                        self._connection = None

    def logs_after(self, log_id: int) -> Dict[str, object]:
        with self._lock:
            logs = [{"id": item_id, "line": line} for item_id, line in self._logs if item_id > log_id]
            connected = self._connection is not None and self._connection.is_open
            return {"logs": logs, "connected": connected, "error": self._error}


class MonitorRequestHandler(BaseHTTPRequestHandler):
    monitor: SerialMonitor

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/":
            self._send_bytes(200, "text/html; charset=utf-8", INDEX_HTML.encode("utf-8"))
        elif parsed.path == "/api/ports":
            self._send_json(200, {"ports": self.monitor.ports()})
        elif parsed.path == "/api/logs":
            try:
                after = int(parse_qs(parsed.query).get("after", ["0"])[0])
            except ValueError:
                after = 0
            self._send_json(200, self.monitor.logs_after(after))
        else:
            self._send_json(404, {"error": "Not found"})

    def do_POST(self) -> None:
        try:
            if self.path == "/api/connect":
                payload = self._read_json()
                port = str(payload.get("port", ""))
                baud = int(payload.get("baud", 115200))
                if not port:
                    raise ValueError("USB port is required")
                self.monitor.connect(port, baud)
                self._send_json(200, {"port": port, "baud": baud})
            elif self.path == "/api/disconnect":
                self.monitor.disconnect()
                self._send_json(200, {"connected": False})
            else:
                self._send_json(404, {"error": "Not found"})
        except (json.JSONDecodeError, OSError, ValueError, serial.SerialException) as exc:
            self._send_json(400, {"error": str(exc)})

    def _read_json(self) -> Dict[str, object]:
        length = int(self.headers.get("Content-Length", "0"))
        return json.loads(self.rfile.read(length) or b"{}")

    def _send_json(self, status: int, payload: Dict[str, object]) -> None:
        self._send_bytes(status, "application/json; charset=utf-8", json.dumps(payload).encode("utf-8"))

    def _send_bytes(self, status: int, content_type: str, payload: bytes) -> None:
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(payload)

    def log_message(self, format_string: str, *args: object) -> None:
        return


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="SensorNode FSM heap monitor")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--no-browser", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    monitor = SerialMonitor()
    MonitorRequestHandler.monitor = monitor
    server = ThreadingHTTPServer((args.host, args.port), MonitorRequestHandler)
    url = f"http://{args.host}:{server.server_port}"

    print(f"SensorNode FSM Heap Monitor: {url}", flush=True)
    print("Press Ctrl-C to stop.", flush=True)
    if not args.no_browser:
        threading.Timer(0.4, lambda: webbrowser.open(url, new=1)).start()

    try:
        server.serve_forever(poll_interval=0.25)
    except KeyboardInterrupt:
        pass
    finally:
        monitor.disconnect()
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
