
class LightSensor {
  public:
  float readLux();
  LightSensor(int lightSensorPin);

  private:
  int lightSensorPin;
};