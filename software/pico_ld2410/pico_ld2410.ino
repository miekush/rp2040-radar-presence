#include <Adafruit_NeoPixel.h>

//define pins
#define RGB_PIN           16
#define TX_PIN            4
#define RX_PIN            5

//define parameters
#define NUM_SAMPLES       5
#define DISTANCE_THRES_1  120
#define DISTANCE_THRES_2  200

//init rgb neopixel
Adafruit_NeoPixel rgb(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

uint8_t data[17];
uint8_t sampleCount=0;
uint16_t distanceSum=0;
uint8_t detectionSum=0;

bool averageDetection=false;
uint16_t averageDistance=0;

uint16_t targetDistance[NUM_SAMPLES];
uint8_t targetDetection[NUM_SAMPLES];

bool dataReady = false;

//debug modes
bool serialDebugBasic=false;
bool serialDebugAdvanced=false;

void setup()
{
  //debug output
  if(serialDebugBasic || serialDebugAdvanced)
  {
    Serial.begin(9600);
  }

  //init uart for sensor
  Serial2.setTX(TX_PIN);
  Serial2.setRX(RX_PIN);
  Serial2.begin(256000);

  //init rgb
  rgb.begin();
  rgb.clear();
}

void loop()
{
  //take sample
  readSensor();

  //check if samples complete
  if(dataReady)
  {
    detectionSum=0;
    distanceSum=0;

    for(int i=0; i<NUM_SAMPLES; i++)
    {
      detectionSum += targetDetection[i];
      distanceSum += targetDistance[i];
    }
    //take average
    averageDetection = detectionSum / NUM_SAMPLES;
    averageDistance = distanceSum / NUM_SAMPLES;

    //print distance info to terminal
    if(serialDebugBasic || serialDebugAdvanced)
    {
      Serial.print("SUM: ");
      Serial.print(distanceSum);
      Serial.print(", ");
      Serial.print("AVG: ");
      Serial.print(averageDistance);

      if(!serialDebugAdvanced)
      {
        Serial.println("");
      }
    }    

    //check for target
    if(averageDetection && (averageDistance < DISTANCE_THRES_1))
    {
      rgb.setPixelColor(0, rgb.Color(0, 255, 0));
      rgb.show();
    }
    else if(averageDetection && (averageDistance < DISTANCE_THRES_2))
    {
      rgb.setPixelColor(0, rgb.Color(255, 165, 0));
      rgb.show();
    }
    else
    {
      rgb.setPixelColor(0, rgb.Color(0, 0, 0));
      rgb.show();    
    }

    dataReady=false;
  }
}

void readSensor(void)
{
  if(Serial2.available())
  {
    uint8_t byte = Serial2.read();

    //check for EOF
    if(byte == 0xF5)
    {
      //read frame
      Serial2.readBytes(data, 17);

      //extract detection signal and distance signal
      targetDetection[sampleCount] = data[8];
      targetDistance[sampleCount] = (data[9] | (data[10] << 8));
      //increment sample count
      sampleCount++;

      //samples complete, ready to average
      if(sampleCount>(NUM_SAMPLES-1))
      {
        dataReady=true;
        sampleCount=0;
      }

      //print raw frame for debugging
      if(serialDebugAdvanced)
      {
        Serial.print("DATA: ");
        for(int i=4; i<17; i++)
        {
          Serial.printf("%02x", data[i]);
          Serial.print(" ");
        }
        Serial.println("");
      }
    }
  }
}
