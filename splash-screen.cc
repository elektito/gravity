#include "splash-screen.hh"
#include "resource-cache.hh"

SplashScreen::SplashScreen(SDL_Window *window) :
  Screen(window),
  background(window, ResourceCache::GetTexture("splash"))
{
}

SplashScreen::~SplashScreen() {

}

void SplashScreen::SwitchScreen(const map<string, string> &lastState) {

}

void SplashScreen::HandleEvent(const SDL_Event &e) {

}

void SplashScreen::Reset() {

}

void SplashScreen::Save(ostream &s) const {

}

void SplashScreen::Load(istream &s) {

}

void SplashScreen::Advance(float dt) {
  static int n = 0;

  switch (n) {
  case 0:
    ResourceCache::GetTexture("background");
    break;
  case 1:
    ResourceCache::GetTexture("sun");
    break;
  case 2:
    ResourceCache::GetTexture("planet");
    break;
  case 3:
    ResourceCache::GetTexture("trail-point");
    break;
  case 4:
    ResourceCache::GetTexture("plus-score");
    break;
  case 5:
    ResourceCache::GetTexture("minus-score");
    break;
  case 6:
    ResourceCache::GetTexture("plus-time");
    break;
  case 7:
    ResourceCache::GetTexture("minus-time");
    break;
  case 8:
    ResourceCache::GetTexture("plus-planet");
    break;
  case 9:
    ResourceCache::GetTexture("enemy");
    break;
  case 10:
    ResourceCache::GetTexture("pause");
    break;
  default:
    this->state["name"] = "splash-over";
    break;
  }

  ++n;
}

void SplashScreen::Render(Renderer *renderer) {
  this->background.Draw();
  renderer->PresentScreen();
}
