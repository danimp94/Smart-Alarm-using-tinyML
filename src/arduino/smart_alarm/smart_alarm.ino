/* Copyright 2021 Carlos Gil, Daniel Moreno.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <TensorFlowLite.h>
#include <Arduino_LSM9DS1.h>  // required library for IMU

#include "main_functions.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "constants.h"
#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

/*
To include: test.cpp? 
- Include the unit test framework header (#include "tensorflow/lite/micro/testing/micro_test.h")
- Validate input shape


link: https://www.tensorflow.org/lite/microcontrollers/get_started_low_level
*/

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::MicroInterpreter* interpreter = nullptr;
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  
  int inference_count = 0;
  
  const int imuIndex = 0; // 0 - accelerometer, 1 - gyroscope, 2 - magnetometer
  bool startStream = false;
  //float accBuffer[3];
  
  
  constexpr int kTensorArenaSize = 2000;
  uint8_t tensor_arena[kTensorArenaSize];

//----------------------------------
  constexpr int label_count = 5;
  const char* labels[label_count] = {"0", "1", "2", "3", "4"};
//---------------------------------- 

}  // namespace

//--------------------
  int max_x = 0;
  int max_y = 0;
  int max_z = 0;
  int lastReportTime = 0;
  int input_array[6];
  int Heart_rate_counter = 0;
  int BPM = 0;
//--------------------

void setup() { 
  // Start serial
  Serial.begin(9600);
  Serial.println("Started");
  while (!Serial);

  // Start IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU");
    while (1);
  }
  
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  //static tflite::AllOpsResolver resolver;
  

    static tflite::MicroMutableOpResolver<4> micro_op_resolver;
    //micro_op_resolver.AddDepthwiseConv2D();
    //micro_op_resolver.AddFullyConnected();
    //micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();


  // Instantiate the interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Validate input shape.
  input = interpreter->input(0);

/*  // Set model input settings
  TfLiteTensor* model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != raster_height) ||
      (model_input->dims->data[2] != raster_width) ||
      (model_input->dims->data[3] != raster_channels) ||
      (model_input->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad input tensor parameters in model");
    return;
  }

    // Set model output settings
  TfLiteTensor* model_output = interpreter->output(0);
  if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) ||
      (model_output->dims->data[1] != label_count) ||
      (model_output->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad output tensor parameters in model");
    return;
  }
*/

  // Validate input shape.
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  inference_count = 0;
}

void loop() 
{

// --- Read data from IMU
  startStream = true;
  float x, y, z;
  if (startStream) 
  {
    if (IMU.accelerationAvailable()) 
    {
      // IMU.readAcceleration(accBuffer[0], accBuffer[1], accBuffer[3]);
      IMU.readAcceleration(x, y, z);
 
    }
  }


  // --- Read data from Heart Rate Sensor
  
//------------------------Data Pre-processing--------------------------

// Max value for each axis in 1 second
  if(abs(x) > abs(max_x)){
     max_x = abs(x);
 }
  if(abs(y) > abs(max_y)){
     max_y = abs(y);
 }    
  if(abs(z) > abs(max_z)){
     max_z = abs(z);
 }
  
 if(millis() - lastReportTime > 1000){
  
   Serial.println("MAX X: " + String(max_x * 9.807) + "\t");
   Serial.println("MAX Y: " + String(max_y * 9.807) + "\t");
   Serial.println("MAX Z: " + String(max_z * 9.807) + "\t");

   // Storage max values for each second and scale to m/s2
   input_array[0] = max_x * 9.807 ;
   input_array[1] = max_y * 9.807 ;
   input_array[2] = max_z * 9.807 ;
   
   // Generate all features

   // Clean max values 
   max_x = 0;
   max_y = 0;
   max_z = 0;

   Heart_rate_counter = ++Heart_rate_counter

   if(Heart_rate_counter = 15{
      input_array[3] = BPM
      Heart_rate_counter = 0
   }
   
   lastReportTime = millis();
 }

 

 
 
//---------------------------------------------------------------------
  
  // Calculate an x value to feed into the model. We compare the current
  // inference_count to the number of inferences per cycle to determine
  // our position within the range of possible x values the model was
  // trained on, and use this to calculate a value.
  float position = static_cast<float>(inference_count) /
                   static_cast<float>(kInferencesPerCycle);
  x = position * kXrange;

  // Quantize the input from floating-point to integer
  int8_t x_quantized = x / input->params.scale + input->params.zero_point;
  // Place the quantized input in the model's input tensor
  input->data.int8[0] = x_quantized;

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: %f\n",
                         static_cast<double>(x));
    return;
  }

  // Obtain the quantized output from model's output tensor
  int8_t y_quantized = output->data.int8[0];
  // Dequantize the output from integer to floating-point
 y = (y_quantized - output->params.zero_point) * output->params.scale;

  // Output the results. A custom HandleOutput function can be implemented
  // for each supported hardware target.
  HandleOutput(error_reporter, x, y);

  // Increment the inference_counter, and reset it if we have reached
  // the total number per cycle
  inference_count += 1;
  if (inference_count >= kInferencesPerCycle) inference_count = 0;
}
