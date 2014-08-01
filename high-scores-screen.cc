#include "high-scores-screen.hh"
#include "config.hh"
#include "image-button-widget.hh"
#include "image-widget.hh"
#include "resource-cache.hh"

#include <iomanip>
#include <sstream>
#include <algorithm>

HighScoresScreen::HighScoresScreen(SDL_Window *window) :
  Screen(window),
  currentScoreIndex(-1),
  background(window, ResourceCache::GetTexture("background"))
{
  this->widgets.push_back(new ImageButtonWidget(this,
                                                ResourceCache::GetTexture("main-menu"),
                                                0.02, 0.02, 0.05,
                                                TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                                {255, 0, 0, 200},
                                                {255, 255, 255, 200}));
  this->widgets.push_back(new ImageWidget(this,
                                          ResourceCache::GetTexture("high-scores"),
                                          0.0, 0.1, 0.1,
                                          TextAnchor::CENTER, TextAnchor::TOP,
                                          {255, 0, 0, 255}));

  for (int i =0; i < 5; ++i) {
    NumberWidget *label = new NumberWidget(this,
                                          0,
                                          0.0, 0.1 + 0.2 + i * 0.1, 0.08,
                                          TextAnchor::CENTER, TextAnchor::TOP,
                                          6,
                                          {255, 255, 255, 128});
    this->widgets.push_back(label);
    this->labels.push_back(label);
  }

  this->Reset();
}

HighScoresScreen::~HighScoresScreen() {

}

void HighScoresScreen::SwitchScreen(const map<string, string> &lastState) {
  this->currentScoreIndex = -1;

  if (lastState.at("name") == "game-over") {
    int score = std::stoi(lastState.at("score"));

    this->scores.push_back(score);
    std::sort(this->scores.begin(), this->scores.end(), std::greater<int>());

    auto it = find(this->scores.rbegin(), this->scores.rend(), score);
    if (it != this->scores.rend())
      this->currentScoreIndex = this->scores.rend() - it - 1;

    if (this->scores.size() > Config::HighScores)
      this->scores.resize(Config::HighScores);

    if (this->currentScoreIndex >= this->scores.size())
      this->currentScoreIndex = -1;
  }

  for (auto w : this->widgets)
    w->Reset();

  this->state.clear();
  this->state["name"] = "highscores-ongoing";
}

void HighScoresScreen::HandleEvent(const SDL_Event &e) {
  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void HighScoresScreen::HandleWidgetEvent(int event_type, Widget *widget) {
  switch (event_type) {
  case BUTTON_CLICK:
    if (widget == this->widgets[0]) { // Main Menu
      this->state["name"] = "highscores-manu-selected";
    }

    break;
  }
}

void HighScoresScreen::Reset() {
  this->state.clear();
  this->state["name"] = "highscores-ongoing";
  this->scores.clear();
}

void HighScoresScreen::Save(ostream &s) const {
  size_t size = this->scores.size();
  s.write((const char*) &size, sizeof(size));
  for (auto score : this->scores)
    s.write((const char*) &score, sizeof(score));
}

void HighScoresScreen::Load(istream &s) {
  size_t size;
  s.read((char*) &size, sizeof(size));
  if (size > Config::HighScores) {
    cout << "Invalid save file." << endl;
    exit(1);
  }

  this->scores.clear();
  for (size_t i = 0; i < size; ++i) {
    int score;
    s.read((char*) &score, sizeof(score));
    this->scores.push_back(score);
  }
}

void HighScoresScreen::Advance(float dt) {
  for (auto w : this->widgets)
    w->Advance(dt);
}

void HighScoresScreen::Render(Renderer *renderer) {
  this->background.Draw();

  for (int i =0; i < this->scores.size(); ++i) {
    SDL_Color c;
    if (i == this->currentScoreIndex)
      c = {255, 255, 0, 200};
    else
      c = {255, 255, 255, 200};
    this->labels[i]->SetColor(c);
    this->labels[i]->SetNumber(this->scores[i]);
    this->labels[i]->SetVisible(true);
  }

  for (int i = this->scores.size(); i < 5; ++i)
    this->labels[i]->SetVisible(false);

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
