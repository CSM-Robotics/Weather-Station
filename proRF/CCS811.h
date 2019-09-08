#include <SparkFunCCS811.h>

class CCS {
  public:
    bool startSensor();
    bool readSensor(float* readings);
  private:
    CCS811 sensor;
};
