#pragma once
#include<iostream>
class Model;
class View;

class Controller {
private:
	Model* model;
	View* view;

public:
	Controller(Model* model, bool smokeTest = false);

	void start();

	void setVision();

	void pause();
};
