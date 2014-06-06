#include "timer.hh"

#include <algorithm>

vector<Timer*> Timer::timers;

Timer::Timer(timer_callback callback) :
  callback(callback),
  expired(true)
{
  timers.push_back(this);
}

Timer::~Timer() {
  timers.erase(find(timers.begin(), timers.end(), this));
}

void Timer::Set(float timeout, bool periodic) {
  this->startTime = SDL_GetTicks();
  this->timeout = timeout;
  this->periodic = periodic;
  this->expired = false;
}

void Timer::Check() {
  if (this->expired)
    return;

  float elapsed = (SDL_GetTicks() - this->startTime) / 1000.0;
  if (elapsed >= this->timeout) {
    if (this->periodic)
      this->startTime = SDL_GetTicks();

    this->callback(elapsed);
    this->expired = !this->periodic;
  }
}

void Timer::Pause() {
  if (expired)
    return;

  this->pauseTime = SDL_GetTicks();
  this->paused = true;
}

void Timer::Unpause() {
  if (this->expired)
    return;

  if (this->paused)
    this->startTime += SDL_GetTicks() - this->pauseTime;

  this->paused = false;
}

bool Timer::TogglePause() {
  if (this->paused)
    this->Unpause();
  else
    this->Pause();

  return this->paused;
}

bool Timer::IsPaused() {
  return this->paused;
}

void Timer::CheckAll() {
  for (auto t : timers)
    t->Check();
}

void Timer::PauseAll() {
  for (auto t : timers)
    t->Pause();
}

void Timer::UnpauseAll() {
  for (auto t : timers)
    t->Unpause();
}

void Timer::TogglePauseAll() {
  for (auto t : timers)
    t->TogglePause();
}
