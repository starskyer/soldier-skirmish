#pragma once
#include<iostream>
class Chessboard;

class Vision {
public:
	Chessboard* chessboard;
	int size;
	bool* hasVision;

	void visionUpdate(); // Doesn't need to update chessboard itself
	// Because chessboard is a pointer

	virtual void getVision() = 0; // Get the vision of a specified team, or both team
};

class VisionTeam : public Vision {
private:
	int team;

public:
	VisionTeam(Chessboard* chessboard, int team);

	void getVision();
};

class VisionAll : public Vision {
public:
	VisionAll(Chessboard* chessboard);

	void getVision();
};