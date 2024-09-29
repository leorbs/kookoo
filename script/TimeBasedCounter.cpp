
class TimeBasedCounter {
private:
  unsigned long times[3];
  unsigned long withinTime = 5000;

public:
  TimeBasedCounter() : times{0, 0, 0} {}

  bool addTimeAndCheck(unsigned long currentTime) {
    for( unsigned int i = 0; i < sizeof(times)/sizeof(times[0]); i = i + 1 ) {
      if(currentTime > times[i] + withinTime) {
        times[i] = currentTime;
        return false;
      }
    }

    reset();
    //no slot in array found where we can remember time. That means there have to be sizeof(times)/sizeof(times[0]) instances of calls to this function
    return true;
  }

  void reset() {
    for( unsigned int i = 0; i < sizeof(times)/sizeof(times[0]); i = i + 1 ) {
      times[i] = 0;
    }
  }

  int getCurrentShakeCounter(unsigned long currentTime){
    int counter = 0;
    for( unsigned int i = 0; i < sizeof(times)/sizeof(times[0]); i = i + 1 ) {
      if(currentTime < times[i] + withinTime) {
        counter = counter + 1;
      }
    }
    return counter;
  }

  unsigned long getLatestTime(){
    unsigned long newest = 0;
    for( unsigned int i = 0; i < sizeof(times)/sizeof(times[0]); i = i + 1 ) {
      if(times[i] > newest) {
        newest = times[i];
      }
    }
    return newest;
  }

};