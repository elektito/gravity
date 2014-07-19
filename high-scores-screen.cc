#include "high-scores-screen.hh"
#include "config.hh"
#include "button-widget.hh"

#include <iomanip>
#include <sstream>
#include <algorithm>

HighScoresScreen::HighScoresScreen(SDL_Window *window) :
  Screen(window),
  currentScoreIndex(-1)
{
  this->widgets.push_back(new ButtonWidget(this,
                                           "Main Menu",
                                           0.02, 0.02, 0.1,
                                           TextAnchor::RIGHT, TextAnchor::BOTTOM,
                                           {255, 0, 0},
                                           {255, 255, 255}));
  this->widgets.push_back(new LabelWidget(this,
                                          "High Scores",
                                          0.0, 0.1, 0.2,
                                          TextAnchor::CENTER, TextAnchor::TOP,
                                          {255, 0, 0}));

  for (int i =0; i < 5; ++i) {
    LabelWidget *label = new LabelWidget(this,
                                         "000000",
                                         0.0, 0.1 + 0.2 + i * 0.1, 0.1,
                                         TextAnchor::CENTER, TextAnchor::TOP,
                                         {255, 255, 255});
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
    if (this->scores.size() > Config::HighScores)
      this->scores.resize(Config::HighScores);

    // See if the current score made it into the high scores.
    for (int i = 0; i < this->scores.size(); ++i)
      if (score == this->scores[i])
        if (i != this->scores.size() - 1 || score > this->scores[i])
          this->currentScoreIndex = i;
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
  stringstream ss;

  renderer->ClearScreen();

  for (int i =0; i < this->scores.size(); ++i) {
    ss.str("");
    ss.clear();
    ss << setw(6) << setfill('0') << this->scores[i];
    SDL_Color c;
    if (i == this->currentScoreIndex)
      c = {255, 255, 0};
    else
      c = {255, 255, 255};
    this->labels[i]->SetColor(c);
    this->labels[i]->SetText(ss.str());
    this->labels[i]->SetVisible(true);
  }

  for (int i = this->scores.size(); i < 5; ++i)
    this->labels[i]->SetVisible(false);

  for (auto w : this->widgets)
    w->Render(renderer);

  renderer->PresentScreen();
}
