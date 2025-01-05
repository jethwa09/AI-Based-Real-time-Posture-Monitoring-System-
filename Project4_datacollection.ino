// Include the Arduino LSM9DS1 library for IMU functionality
#include <Arduino_LSM9DS1.h>

void setup() {
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600);
  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial);
  
  // Initialize the IMU (Inertial Measurement Unit)
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    // If IMU initialization fails, enter an infinite loop
    while (1);
  }
}

void loop() {
  // Declare variables to store sensor readings
  float ax, ay, az;  // Accelerometer readings (x, y, z)
  float gx, gy, gz;  // Gyroscope readings (x, y, z)
  float mx, my, mz;  // Magnetometer readings (x, y, z)
  
  // Check if new sensor data is available from all three sensors
  if (IMU.accelerationAvailable() && 
      IMU.gyroscopeAvailable() && 
      IMU.magneticFieldAvailable()) {
      
    // Read accelerometer data
    IMU.readAcceleration(ax, ay, az);
    // Read gyroscope data
    IMU.readGyroscope(gx, gy, gz);
    // Read magnetometer data
    IMU.readMagneticField(mx, my, mz);
    
    // Print all sensor data to serial, comma-separated
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.print(gz); Serial.print(",");
    Serial.print(mx); Serial.print(",");
    Serial.print(my); Serial.print(",");
    Serial.println(mz);  // Use println for the last value to end the line
  }
  
  // Wait for 10 milliseconds before the next reading (approximately 100Hz sampling rate)
  delay(10);
}