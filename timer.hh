#ifndef _GRAVITY_TIMER_HH_
#define _GRAVITY_TIMER_HH_

#include <SDL2/SDL.h>

#include <functional>
#include <vector>

using namespace std;

class Timer {
protected:
  typedef function<void (float elapsed)> timer_callback;

  Uint32 startTime;
  float timeout;
  bool periodic;

  Uint32 pauseTime;
  bool paused;

  bool expired;

  timer_callback callback;

  static vector<Timer*> timers;

public:
  Timer(timer_callback callback);
  virtual ~Timer();

  void Set(float timeout, bool periodic=false);
  void Check();
  void Pause();
  void Unpause();
  bool TogglePause();
  bool IsPaused();

  static void CheckAll();
  static void PauseAll();
  static void UnpauseAll();
  static void TogglePauseAll();
};

#endif /* _GRAVITY_TIMER_HH_ */
