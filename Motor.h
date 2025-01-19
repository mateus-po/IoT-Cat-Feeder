
class Motor {
  public:
  void moveForward();
  void moveBackward();
  void stopMotor();
  Motor(int motor1Pin1, int motor1Pin2, int enable1Pin);

  private:
  int motor1Pin1;
  int motor1Pin2;
  int enable1Pin;
};