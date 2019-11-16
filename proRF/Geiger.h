#pragma once

class Geiger {
  public:
    bool startSensor();
	bool readSensor(float* readings);
  private:
    // no Sparkfun lib for this, come back later

};
