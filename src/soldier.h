#pragma once
#include<iostream>
#include<mutex>
class AttackBehavior;
class Chessboard;

class Soldier {
public:
	int pos;
	int team;
	int maxHP;
	int atk1, atk2;
	int atkRange; // Delegate AttackBehavior to attack
	int moveSpeed; // Squares the soldier can move in a period
	int visionRange;
	Chessboard* chessboard;
	AttackBehavior *attackBehavior1, *attackBehavior2;
	
	int currentHP;

	virtual int decide(int* target); // Decide what to do every period

	// Execute the following action will cost a period of time
	void move(int target); // Will modify the chessboard

	void attack(int type, int target); // May modify the chessboard

	void stop();

	void soldierAction(); //Endlessly decide and choose to move, attack or stop

	virtual int type() = 0; // Soldier type
};

class Infantry : public Soldier {
public:
	Infantry(int pos, int team, Chessboard* chessboard);

	int type();
};

class Archer : public Soldier {
public:
	Archer(int pos, int team, Chessboard* chessboard);

	int type();
};

class SiegeGun : public Soldier {
public:
	SiegeGun(int pos, int team, Chessboard* chessboard);

	int decide(int* target);

	int type();
};

class SoldierFactory {
public:
	Soldier* createSoldier(int type, int pos, int team, Chessboard* chessboard);
};