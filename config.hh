#ifndef _GRAVITY_CONFIG_HH_
#define _GRAVITY_CONFIG_HH_

class Config {
public:
  static const bool HardwareAcceleration;
  static const bool VSync;
  static const int HighScores;
  static const float PhysicsTimeStep;
  static const int ScreenWidth;
  static const int ScreenHeight;
  static const int TimeStep;
  static const int GameTime;
  static const float CameraMinWidth;
  static const float CameraMinHeight;
  static const float CameraMaxWidth;
  static const float CameraMaxHeight;
};

#endif /* _GRAVITY_CONFIG_HH_ */
