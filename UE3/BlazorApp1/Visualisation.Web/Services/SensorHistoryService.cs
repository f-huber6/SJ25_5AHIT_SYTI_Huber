using BlazorApp1.Data;

namespace BlazorApp1.Services;

/// <summary>Speichert Sensorwerte für die Chart-Visualisierung.</summary>
public class SensorHistoryService : IDisposable
{
    private readonly SerialService _serial;

    public SensorHistoryService(SerialService serial)
    {
        _serial = serial;
        _serial.MessageReceived += OnMessageReceived;
    }

    private void OnMessageReceived(string rawMessage)
    {
        var isSensorData = MessageParser.TryParseSensorData(rawMessage, out var data);

        AddRawMessage(rawMessage, notify: !isSensorData);

        if (isSensorData)
            AddReading(data, notify: true);
    }

    private const int MaxHistoryPoints = 200;
    private readonly List<SensorDataPoint> _history = new();
    private readonly List<string> _rawMessages = new();
    private const int MaxMessages = 100;

    public event Action? DataChanged;

    public IReadOnlyList<SensorDataPoint> History
    {
        get { lock (_history) { return _history.ToList(); } }
    }

    public IReadOnlyList<string> RawMessages
    {
        get { lock (_rawMessages) { return _rawMessages.ToList(); } }
    }

    private void AddReading(SensorData data, bool notify)
    {
        lock (_history)
        {
            _history.Add(new SensorDataPoint(
                DateTime.Now,
                data.Temperature,
                data.Humidity,
                data.Sequence));

            while (_history.Count > MaxHistoryPoints)
                _history.RemoveAt(0);
        }

        if (notify)
            DataChanged?.Invoke();
    }

    private void AddRawMessage(string rawMessage, bool notify)
    {
        lock (_rawMessages)
        {
            _rawMessages.Add(rawMessage);
            while (_rawMessages.Count > MaxMessages)
                _rawMessages.RemoveAt(0);
        }

        if (notify)
            DataChanged?.Invoke();
    }

    public record SensorDataPoint(DateTime Timestamp, int Temperature, int Humidity, int Sequence);

    public void Dispose()
    {
        _serial.MessageReceived -= OnMessageReceived;
    }
}
