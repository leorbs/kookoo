#include <SoftTimers.h>

class JobManager {
private:
  // SoftTimer for job duration and backoff timer
  SoftTimer jobTimer;
  SoftTimer backoffTimer;

  // Pointers to enable and disable functions
  void (*enableFunction)();
  void (*disableFunction)();

  // Pointers to enable function with parameters
  void (*enableFunctionWithParams)(void*);

  // Parameters for the enable function with parameters
  void* enableFunctionParams;

  // Job duration and backoff duration in milliseconds
  uint32_t jobDuration;
  uint32_t backoffDuration;

  // Internal state of job
  bool isJobRunning = false;
  bool isInBackoff = false;

  // "Run once" mode flag
  bool runOnceModeActive = false;
  bool hasRunOnce = false;  // Tracks if the job has already run once

public:
  // Constructor to initialize the JobManager with job duration, backoff time, and function pointers
  JobManager(
    uint32_t jobDurationMillis, 
    uint32_t backoffDurationMillis, 
    void (*enableFn)(), 
    void (*disableFn)(), 
    bool runOnceMode = false
  ) : jobDuration(jobDurationMillis), 
      backoffDuration(backoffDurationMillis), 
      enableFunction(enableFn), 
      disableFunction(disableFn), 
      runOnceModeActive(runOnceMode) {
    
    jobTimer.setTimeOutTime(jobDuration);  // Set the job duration timeout
    backoffTimer.setTimeOutTime(backoffDuration);  // Set the backoff duration timeout
  }

  // Overloaded constructor to handle enable function with parameters
  JobManager(
    uint32_t jobDurationMillis, 
    uint32_t backoffDurationMillis, 
    void (*enableFnWithParams)(void*), 
    void* params, 
    void (*disableFn)(), 
    bool runOnceMode = false
  ) : jobDuration(jobDurationMillis), 
      backoffDuration(backoffDurationMillis), 
      enableFunctionWithParams(enableFnWithParams), 
      enableFunctionParams(params), 
      disableFunction(disableFn), 
      runOnceModeActive(runOnceMode) {
    
    jobTimer.setTimeOutTime(jobDuration);  // Set the job duration timeout
    backoffTimer.setTimeOutTime(backoffDuration);  // Set the backoff duration timeout
  }

  //set new params and then start the job
  void startJob(void* params) {
    enableFunctionParams = params;
    startJob();
  }

  // Function to start the job if it's not running, not in backoff, and allowed by runOnce mode
  void startJob() {
    if (!isJobRunning && !isInBackoff && (!runOnceModeActive || !hasRunOnce)) {
      isJobRunning = true;
      hasRunOnce = true;  // Mark that the job has run once if in "run once" mode
      if (enableFunction) {
        enableFunction();  // Call the enable function when starting the job
      } else if (enableFunctionWithParams) {
        enableFunctionWithParams(enableFunctionParams);  // Call the enable function with parameters
      }
      jobTimer.reset();  // Reset the job timer for the job duration
    }
  }
  
  // Function to reset the job (can only be reset after the backoff has passed)
  void resetJob() {
    if (!isInBackoff) {
      hasRunOnce = false;  // Allow the job to run again if in "run once" mode
    }
  }

  // Function to reset the backoff timer
  void resetBackoff() {
    isInBackoff = false;  // Allow the job to start again after reset
  }

  // Function to check and handle the job and backoff in loop()
  void handleJob() {
    // Check if the job has timed out
    if (isJobRunning && jobTimer.hasTimedOut()) {
      // Disable the job
      if (disableFunction) {
        disableFunction();  // Call the disable function when stopping the job
      }
      isJobRunning = false;
      isInBackoff = true;  // Enter backoff state
      backoffTimer.reset();  // Reset the backoff timer
    }

    // Check if backoff has ended
    if (isInBackoff && backoffTimer.hasTimedOut()) {
      isInBackoff = false;  // End the backoff period, allowing the job to start again
    }
  }

  // Utility functions to get remaining job or backoff time
  uint32_t getRemainingJobTime() {
    return jobTimer.getRemainingTime();
  }

  uint32_t getRemainingBackoffTime() {
    return backoffTimer.getRemainingTime();
  }

  void setNewDurationTime(uint32_t newDuration){
    jobTimer.setTimeOutTime(newDuration);
  }

  void setNewBackoffTime(uint32_t newDuration){
    backoffTimer.setTimeOutTime(newDuration);
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