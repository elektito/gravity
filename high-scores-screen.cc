#include "high-scores-screen.hh"
#include "config.hh"
#include "button-widget.hh"

#include <iomanip>
#include <sstream>

HighScoresScreen::HighScoresScreen(SDL_Window *window) :
  Screen(window)
{
  this->widgets.push_back(new ButtonWidget(this,
                                           "Main Menu",
                                           0.02, 0.02, 0.1,
                                           TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                           {255, 0, 0},
                                           {255, 255, 255}));
  this->Reset();
}

HighScoresScreen::~HighScoresScreen() {

}

void HighScoresScreen::SwitchScreen(const map<string, string> &lastState) {
  int score = std::stoi(lastState.at("score"));
  this->scores.push_back(score);
  std::sort(this->scores.begin(), this->scores.end(), std::greater<int>());
  if (this->scores.size() > Config::HighScores)
    this->scores.resize(Config::HighScores);
}

void HighScoresScreen::HandleEvent(const SDL_Event &e) {
  for (auto w : this->widgets)
    w->HandleEvent(e);
}

void HighScoresScreen::Reset() {
  this->scores.clear();
}

void HighScoresScreen::Save(ostream &s) const {
  s << this->scores.size();
  for (auto score : this->scores)
    s << score;
}

void HighScoresScreen::Load(istream &s) {
  size_t size;
  s >> size;
  for (size_t i = 0; i < size; ++i) {
    int score;
    s >> score;
    this->scores.push_back(score);
  }
}

void HighScoresScreen::Advance(float dt) {
  for (auto w : this->widgets)
    w->Advance(dt);
}

void HighScoresScreen::Render(Renderer *renderer) {
  stringstream ss;

  renderer->ClearScreen(0, 0, 0);
  renderer->DrawTextP("High Scores", 0.0, 0.1, 0.2, {255, 0, 0}, TextAnchor::CENTER, TextAnchor::TOP);
  for (int i =0; i < this->scores.size(); ++i) {
    ss.str("");
    ss.clear();
    ss << setw(6) << setfill('0') << this->scores[i];
    renderer->DrawTextP(ss.str(),
                        0.0, 0.1 + 0.2 + i * 0.1, 0.1,
                        {255, 255, 255},
                        TextAnchor::CENTER, TextAnchor::TOP);
  }

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
