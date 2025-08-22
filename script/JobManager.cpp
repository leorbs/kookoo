#include <SoftTimers.h>

class JobManager {
private:
  // SoftTimer for job duration and backoff timer
  SoftTimer jobTimer;
  SoftTimer backoffTimer;

  // Pointers to enable and disable functions
  void (*enableFunction)();
  void (*disableFunction)();

  // Job duration and backoff duration in milliseconds
  uint16_t jobDuration;
  uint16_t backoffDuration;

  // Internal state of job
  bool isJobRunning = false;
  bool isInBackoff = false;

  // "Run once" mode flag
  bool runOnceModeActive = false;
  bool hasRunOnce = false;  // Tracks if the job has already run once

public:
  // Constructor to initialize the JobManager with job duration, backoff time, and function pointers
  JobManager(
    uint16_t jobDurationMillis, 
    uint16_t backoffDurationMillis, 
    void (*enableFn)(), 
    void (*disableFn)(), 
    bool runOnceMode = false,
    bool startInBackoffMode = false
  ) : jobDuration(jobDurationMillis), 
      backoffDuration(backoffDurationMillis), 
      enableFunction(enableFn), 
      disableFunction(disableFn), 
      runOnceModeActive(runOnceMode) {
    
    jobTimer.setTimeOutTime(jobDuration);  // Set the job duration timeout
    backoffTimer.setTimeOutTime(backoffDuration);  // Set the backoff duration timeout
    if(startInBackoffMode){
      if(runOnceMode) {
        //this enables the run once mode to start in the hasRun state
        hasRunOnce = true;
      }
      backoffTimer.reset();
      isInBackoff = true;
    }
  }

    // Constructor to initialize the JobManager with job duration, backoff time, and function pointers
  JobManager(
    uint16_t jobDurationMillis, 
    void (*enableFn)(), 
    void (*disableFn)(), 
    bool runOnceMode = false
  ) : jobDuration(jobDurationMillis), 
      backoffDuration(0), 
      enableFunction(enableFn), 
      disableFunction(disableFn), 
      runOnceModeActive(runOnceMode){
    
    jobTimer.setTimeOutTime(jobDuration);  // Set the job duration timeout
    backoffTimer.setTimeOutTime(backoffDuration);  // Set the backoff duration timeout
  }

  // Function to start the job if it's not running, not in backoff, and allowed by runOnce mode
  void startJob() {
    if (!isJobRunning && !isInBackoff && (!runOnceModeActive || !hasRunOnce)) {
      isJobRunning = true;
      hasRunOnce = true;  // Mark that the job has run once if in "run once" mode

      enableFunction();  // Call the enable function when starting the job

      jobTimer.reset();  // Reset the job timer for the job duration
    }
  }
  
  // Function to reset the job (can only be reset after the backoff has passed)
  void resetRunOnce() {
    if (!isJobRunning && !isInBackoff) {
      hasRunOnce = false;  // Allow the job to run again if in "run once" mode
    }
  }

  // Function to restart the job timer (can only restart the timer while the job is running)
  void restartJobTimer() {
    if (isJobRunning && !isInBackoff) {
      jobTimer.reset();
    }
  }

  // Function to reset the backoff timer
  void renewBackoff() {
    if(isInBackoff && !backoffTimer.hasTimedOut()) {
      backoffTimer.reset();  // Reset the backoff timer
    }

  }

  void endJob() {
    if(isJobRunning) {
      if (disableFunction) {
        disableFunction();  // Call the disable function when stopping the job
      }
      isJobRunning = false;
    }

    if(backoffDuration != 0) {
        isInBackoff = true;  // Enter backoff state
        backoffTimer.reset();  // Reset the backoff timer
      }

  }

  // Function to check and handle the job and backoff in loop()
  void handleJob() {
    // Check if the job has timed out
    if (isJobRunning && jobTimer.hasTimedOut()) {
      endJob();
    }

    // Check if backoff has ended
    if (isInBackoff && backoffTimer.hasTimedOut()) {
      isInBackoff = false;  // End the backoff period, allowing the job to start again
    }
  }

  // Utility functions to get remaining job or backoff time
  uint16_t getRemainingJobTime() {
    return jobTimer.getRemainingTime();
  }

  uint16_t getRemainingBackoffTime() {
    return backoffTimer.getRemainingTime();
  }

  void setNewDurationTime(uint16_t newDuration){
    jobDuration = newDuration;
    jobTimer.setTimeOutTime(jobDuration);
  }

  void setNewBackoffTime(uint16_t newBackoffDuration){
    backoffDuration = newBackoffDuration;
    backoffTimer.setTimeOutTime(backoffDuration);
  }

  uint16_t getJobDuration(){
    return jobDuration;
  }

  uint16_t getBackoffDuration(){
    return backoffDuration;
  }

  // Function to check if job is currently running
  bool isJobActive() {
    return isJobRunning;
  }

  // Function to check if backoff is active
  bool isBackoffActive() {
    return isInBackoff;
  }
};
