#include<iostream>
#include<thread>
#include<stdexcept>
#include "controller.h"
#include "model.h"
#include "view.h"

Controller::Controller(Model* model, bool smokeTest) {
	this->model = model;
	view = new View(model, this);
	if (!view->SDLinitialize()) {
		view->close();
		throw std::runtime_error("SDL initialization or asset loading failed; see diagnostics above");
	}
	view->printBoardSDL();
	if (smokeTest) view->close();
	else view->SDLWork();
}

void Controller::start() {
	std::thread startThread(&Model::start_test, model);
	// model->start();
	// view->SDLWork();
	startThread.detach();
}

void Controller::setVision() {
	// vision should be 0, 1 or 2
	view->vision = (view->vision + 1) % 3;
}

void Controller::pause() {
	model->pause();
}
