using BlazorApp1.Data;

namespace BlazorApp1.Services;

public static class MessageParser
{
    public static bool TryParseSensorData(string msg, out SensorData data)
    {
        data = new SensorData();
        var hasTemp = false;
        var hasHumidity = false;

        var parts = msg.Split('|');

        foreach (var p in parts)
        {
            if (p.StartsWith("DATE") && p.Length > 4 && int.TryParse(p[4..], out var temp))
            {
                data.Temperature = temp;
                hasTemp = true;
            }

            if (p.StartsWith("HU") && p.Length > 2 && int.TryParse(p[2..], out var hum))
            {
                data.Humidity = hum;
                hasHumidity = true;
            }

            if (p.StartsWith("SN") && p.Length > 2 && int.TryParse(p[2..], out var seq))
                data.Sequence = seq;
        }

        return hasTemp && hasHumidity;
    }

    public static SensorData Parse(string msg)
    {
        TryParseSensorData(msg, out var data);
        return data;
    }
}