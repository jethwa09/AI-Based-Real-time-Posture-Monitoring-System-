#include <TensorFlowLite.h>
// all_ops_resolver allows interpreter to load operations used by model
#include <tensorflow/lite/micro/all_ops_resolver.h>
// micro_error_reporter log errors and output (for debugging purposes)
#include <tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h>
//micro_interpreter is TensorFlow Lite interpreter (this will run the model)
#include <tensorflow/lite/micro/micro_interpreter.h>
// Schema that defines structure of TensorFlow Lite FlatBuffer data,
// used to make sense of model data in model.h
#include <tensorflow/lite/schema/schema_generated.h>
//#include <tensorflow/lite/version.h>
// the actual trained model converted to Arduino header format
#include "model.h"
#include <Arduino_LSM9DS1.h>

// Global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;
tflite::MicroMutableOpResolver<8> tflOpsResolver;
const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM
constexpr int kTensorArenaSize = 8 * 1024;
uint8_t tensorArena[kTensorArenaSize] __attribute__((aligned(16)));
// Constants
const int WINDOW_SIZE = 40;  // 2 seconds at 20Hz
const int NUM_SENSORS = 3;   // x, y, z values per sensor

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  tflOpsResolver.AddFullyConnected();

  // Get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, kTensorArenaSize);
  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  Serial.println("Setup complete");
}

void loop() {

    if (Serial.available()) {
    int command = Serial.read() - '0';
    
    switch(command) {
      case 1: // Accelerometer
      case 2: // Gyroscope
      case 3: // Magnetometer
        collectAndPredict(command);
        break;
      default:
        Serial.println("Invalid command");
        break;
    }
  }

  float ax, ay, az, gx, gy, gz, mx, my, mz;

  // Read accelerometer data
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
  }

  // Read gyroscope data
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
  }

  // Read magnetometer data
  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mx, my, mz);
  }

  // Normalize sensor data between 0 to 1 and store in the model's input tensor
  tflInputTensor->data.f[0] = (ax + 4.0) / 8.0;
  tflInputTensor->data.f[1] = (ay + 4.0) / 8.0;
  tflInputTensor->data.f[2] = (az + 4.0) / 8.0;
  tflInputTensor->data.f[3] = (gx + 2000.0) / 4000.0;
  tflInputTensor->data.f[4] = (gy + 2000.0) / 4000.0;
  tflInputTensor->data.f[5] = (gz + 2000.0) / 4000.0;
  tflInputTensor->data.f[6] = (mx + 400.0) / 800.0;
  tflInputTensor->data.f[7] = (my + 400.0) / 800.0;
  tflInputTensor->data.f[8] = (mz + 400.0) / 800.0;

  // Run inferencing
  TfLiteStatus invokeStatus = tflInterpreter->Invoke();
  if (invokeStatus != kTfLiteOk) {
    Serial.println("Invoke failed!");
    while (1);
    return;
  }

  // Print the output of the model
  Serial.print("Model output: ");
  Serial.println(tflOutputTensor->data.f[0], 6);

  // Print raw sensor data
  Serial.print("Accel: ");
  Serial.print(ax);
  Serial.print(", ");
  Serial.print(ay);
  Serial.print(", ");
  Serial.println(az);

  Serial.print("Gyro: ");
  Serial.print(gx);
  Serial.print(", ");
  Serial.print(gy);
  Serial.print(", ");
  Serial.println(gz);

  Serial.print("Mag: ");
  Serial.print(mx);
  Serial.print(", ");
  Serial.print(my);
  Serial.print(", ");
  Serial.println(mz);

  Serial.println();

  delay(1000);
}



void collectAndPredict(int sensorType) {
  

  // Reset data collection
  int currentIndex = 0;
  
  // Collect WINDOW_SIZE samples
  while (currentIndex < WINDOW_SIZE) {
    if (readSensorData(sensorType)) {
      currentIndex++;
      delay(50); // 20Hz sampling rate
    }
  }
  
  // Prepare data for inference
  for (int i = 0; i < WINDOW_SIZE; i++) {
    for (int j = 0; j < NUM_SENSORS; j++) {
      // Normalize data between 0 and 1
      tflInputTensor->data.f[i * NUM_SENSORS + j] = 
        (sensorData[i][j] + 4.0) / 8.0;
    }
  }

  // Run inference
  TfLiteStatus invokeStatus = tflInterpreter->Invoke();
  if (invokeStatus != kTfLiteOk) {
    Serial.println("Inference failed!");
    return;
  }
  /*

  // Find highest probability class
  float maxProb = 0;
  int predictedClass = 0;
  for (int i = 0; i < 5; i++) {
    if (tflOutputTensor->data.f[i] > maxProb) {
      maxProb = tflOutputTensor->data.f[i];
      predictedClass = i + 1;
    }
  }

  // Send result to base station
  Serial.println(predictedClass); */
}



bool readSensorData(int sensorType) {
  float x, y, z;
  bool dataRead = false;
  
  switch(sensorType) {
    case 1: // Accelerometer
      if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);
        dataRead = true;
      }
      break;
      
    case 2: // Gyroscope
      if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(x, y, z);
        dataRead = true;
      }
      break;
      
    case 3: // Magnetometer
      if (IMU.magneticFieldAvailable()) {
        IMU.readMagneticField(x, y, z);
        dataRead = true;
      }
      break;
  }
  
  if (dataRead) {
    sensorData[currentIndex][0] = x;
    sensorData[currentIndex][1] = y;
    sensorData[currentIndex][2] = z;
    return true;
  }
  
  return false;
}