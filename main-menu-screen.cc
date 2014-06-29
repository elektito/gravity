#include "main-menu-screen.hh"
#include "button-widget.hh"

MainMenuScreen::MainMenuScreen(SDL_Window *window) :
  Screen(window)
{
  this->widgets.push_back(new ButtonWidget(this,
                                           "New Game",
                                           0.0, 0.45, 0.1,
                                           TextAnchor::CENTER, TextAnchor::TOP,
                                           {255, 0, 0},
                                           {255, 255, 255}));
  this->widgets.push_back(new ButtonWidget(this,
                                           "Exit",
                                           0.0, 0.55, 0.1,
                                           TextAnchor::CENTER, TextAnchor::TOP,
                                           {255, 0, 0},
                                           {255, 255, 255}));

  this->Reset();
}

MainMenuScreen::~MainMenuScreen() {

}

void MainMenuScreen::SwitchScreen(const map<string, string> &lastState) {

}

void MainMenuScreen::HandleEvent(const SDL_Event &e) {
  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void MainMenuScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    if (widget == this->widgets[0]) { // New Game
      this->state["name"] = "menu-new-game-selected";
    }
    else if (widget == this->widgets[1]) { // Exit
      this->state["name"] = "menu-exit-selected";
    }

    break;
  }
}

void MainMenuScreen::Reset() {
  this->state.clear();
  this->state["name"] = "menu-ongoing";
}

void MainMenuScreen::Save(ostream &s) const {

}

void MainMenuScreen::Load(istream &s) {

}

void MainMenuScreen::Advance(float dt) {
  for (auto w : this->widgets)
    w->Advance(dt);
}

void MainMenuScreen::Render(Renderer *renderer) {
  renderer->ClearScreen(0, 0, 0);
  renderer->DrawTextP("Main Menu", 0.0, 0.1, 0.2, {255, 0, 0}, TextAnchor::CENTER, TextAnchor::TOP);

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
