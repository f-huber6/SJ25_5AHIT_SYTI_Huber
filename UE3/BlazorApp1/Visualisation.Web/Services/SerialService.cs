using System.IO.Ports;
using System.Text;

namespace BlazorApp1.Services;

public class SerialService
{
    private SerialPort? _port;
    private StringBuilder _buffer = new StringBuilder();
    private CancellationTokenSource? _readCts;
    private Task? _readTask;

    public bool AckEnabled { get; set; } = true;

    /// <summary>True wenn eine serielle Verbindung aktiv ist.</summary>
    public bool IsConnected => _port?.IsOpen ?? false;

    public event Action<string>? MessageReceived;
    public event Action? ConnectionStateChanged;

    public List<string> GetAvailablePorts()
    {
        return SerialPort.GetPortNames().ToList();
    }

    public async Task ConnectAsync(string portName, int baud = 9600)
    {
        if (IsConnected)
            await DisconnectAsync();

        _port = new SerialPort(portName, baud)
        {
            ReadTimeout = 100,
            WriteTimeout = 1000
        };

        try
        {
            _port.Open();
            _buffer.Clear();

            // Polling-Loop starten (zuverlässiger als DataReceived bei USB-Serial)
            _readCts = new CancellationTokenSource();
            _readTask = ReadLoopAsync(_readCts.Token);

            ConnectionStateChanged?.Invoke();
        }
        catch
        {
            _port.Dispose();
            _port = null;
            throw;
        }

        await Task.CompletedTask;
    }

    public async Task DisconnectAsync()
    {
        if (_port == null)
        {
            await Task.CompletedTask;
            return;
        }

        _readCts?.Cancel();

        if (_readTask != null)
        {
            try { await _readTask.WaitAsync(TimeSpan.FromSeconds(2)); }
            catch { /* Timeout beim Warten auf Read-Loop */ }
        }

        _readCts?.Dispose();
        _readCts = null;
        _readTask = null;

        try
        {
            if (_port.IsOpen)
                _port.Close();
        }
        finally
        {
            _port.Dispose();
            _port = null;
            ConnectionStateChanged?.Invoke();
        }

        await Task.CompletedTask;
    }

    /// <summary>Kontinuierliches Auslesen des Ports - zuverlässiger als DataReceived bei USB-Adapter.</summary>
    private async Task ReadLoopAsync(CancellationToken ct)
    {
        var port = _port;
        if (port == null) return;

        while (!ct.IsCancellationRequested && port.IsOpen)
        {
            try
            {
                if (port.BytesToRead > 0)
                {
                    string data = port.ReadExisting();
                    lock (_buffer)
                    {
                        _buffer.Append(data);
                        ProcessBuffer();
                    }
                }
                else
                {
                    await Task.Delay(50, ct); // Alle 50ms prüfen
                }
            }
            catch (OperationCanceledException) { break; }
            catch (TimeoutException) { /* normal bei ReadTimeout */ }
            catch (InvalidOperationException) { break; } // Port geschlossen
            catch { /* andere Fehler ignorieren, Loop läuft weiter */ }
        }
    }

    private void ProcessBuffer()
    {
        while (true)
        {
            string bufferString = _buffer.ToString();

            int stx = bufferString.IndexOf((char)0x02);
            int etx = bufferString.IndexOf((char)0x03);

            if (stx >= 0 && etx > stx)
            {
                string message = bufferString.Substring(stx + 1, etx - stx - 1);

                _buffer.Remove(0, etx + 1);

                MessageReceived?.Invoke(message);

                if (AckEnabled)
                    SendAck();
            }
            else
            {
                break;
            }
        }
    }

    public void Send(string command)
    {
        _port?.Write(command);
    }

    public void SendAck()
    {
        if (_port != null)
        {
            byte[] ack = new byte[] { 0x06 };
            _port.Write(ack, 0, 1);
        }
    }
}