#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<mutex>
class Base;
class Soldier;
class Vision;
class Model;

class Coordinate {
public:
	int x, y;
};

class Square {
public:
	int type;
	bool hasSoldier;
	bool hasBase;
	Soldier* soldier;
	Base* base;

	Square();

	void setBase(Base* base);

	void setSoldier(Soldier* soldier);

	void removeSoldier();
};

class Chessboard {
	friend class Model; // Allow class Model to access bases[2]
	friend class View;
	friend class VisionTeam;
	friend class VisionAll;
private:
	std::vector<Vision*> visions; // Observers of chessboard
	Model* model; // Observer of chessboard
	std::vector<Soldier*> soldiers;		
	Base* bases[2];
	bool busy;
	int length;
	int width;
	int size;
	Square* chessboard;

public:
	std::mutex mtx, move_mtx, atk_mtx;
	bool pause;
	Chessboard(std::string mapPath, std::string basePath); 
	// Initialize chessboard from map path
	// Initialize bases from base path

	void registerVision(Vision* vision);

	void registerModel(Model* model);

	void notifyObservers(); // Update team visions and model views

	int getLength();

	int getWidth();

	int getSize();

	//int Cor2Pos(Coordinate* cor);

	//void Pos2Cor(Coordinate* cor, int pos);

	std::vector<int> posAround(int pos, int distance); 
	std::vector<int> posAround(int pos, int distanceFrom, int distanceTo);
	// Return all positions within the distance of given position

	void setSoldier(int pos, Soldier* soldier); // Need to notify observers

	void removeSoldier(int pos); // Need to notify observers

	Square* getSquare(int pos);

	int getEnemyBasePos(int team);

	void printBoardConsole();
};