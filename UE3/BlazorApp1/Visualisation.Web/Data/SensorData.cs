namespace BlazorApp1.Data;

public class SensorData
{
    public int Temperature { get; set; }
    public int Humidity { get; set; }
    public int Sequence { get; set; }
    public DateTime Timestamp { get; set; } = DateTime.Now;
}