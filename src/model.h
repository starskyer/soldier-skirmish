#pragma once
#include<iostream>
#include<string>
#include<vector>
class Base;
class Chessboard;
class Soldier;
class View;

class Model {
	friend class View;
private:
	std::vector<View*> views; // Observers of model
	Chessboard* chessboard; // Soldiers, bases and visions are included in chessboard

public:
	std::string mapPath, basePath;

	Model(std::string mapPath, std::string basePath);

	void start();

	void start_test();

	void registerObservers(View* view);

	void notifyObservers(); // Update views

	void pause();
};