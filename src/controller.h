#pragma once
#include "frameClock.h"
#include "view.h"

namespace skirmish {
class Controller {
  public:
    Controller(Model &model, View &view) : model_(model), view_(view) {}
    void run(bool smokeTest = false, const std::filesystem::path &screenshot = {});

  private:
    Model &model_;
    View &view_;
    int vision_ = 0;
};
} // namespace skirmish
