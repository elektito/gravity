#include "credits-screen.hh"
#include "resource-cache.hh"
#include "helpers.hh"

CreditsScreen::CreditsScreen(SDL_Window *window) :
  Screen(window),
  background(window, ResourceCache::GetTexture("background"))
{
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("main-menu"),
                                                0.02, 0.02, 0.05,
                                                TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                                {255, 0, 0, 200},
                                                {255, 255, 255, 200}));
  this->widgets.push_back(new ImageWidget(this,
                                          ResourceCache::GetTexture("credits"),
                                          0.0, 0.0, 0.6,
                                          TextAnchor::CENTER, TextAnchor::CENTER));
}

CreditsScreen::~CreditsScreen() {

}

void CreditsScreen::SwitchScreen(const map<string, string> &lastState) {
  this->state.clear();
  this->state["name"] = "menu-ongoing";

  for (auto w : this->widgets)
    w->Reset();
}

void CreditsScreen::HandleEvent(const SDL_Event &e) {
  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void CreditsScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    PlaySound("button-click");

    if (widget == this->widgets[0]) { // Main Menu
      this->state["name"] = "credits-manu-selected";
    }

    break;

  case BUTTON_MOUSE_ENTER:
    PlaySound("mouse-over");
    break;
  }
}

void CreditsScreen::Reset() {
  this->state.clear();
  this->state["name"] = "credits-ongoing";
}

void CreditsScreen::Save(ostream &s) const {

}

void CreditsScreen::Load(istream &s) {

}

void CreditsScreen::Advance(float dt) {
  for (auto w : this->widgets)
    w->Advance(dt);
}

void CreditsScreen::Render(Renderer *renderer) {
  this->background.Draw();

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
