#pragma once
#include<iostream>
class Chessboard;

class AttackBehavior {
// All the attack behavior will cost a period of time
public:
	virtual void attack(int target) = 0; // May modify the chessboard
};

class BaseAttack : public AttackBehavior {
// Attack enemy at target position.
// Attack behavior itself will not check if the position is in attack range.
private:
	int atk;
	Chessboard* chessboard;
public:
	BaseAttack(int atk, Chessboard* chessboard);

	void attack(int target);
};

class SoldierNormalAttack : public AttackBehavior {
private:
	int atk;
	Chessboard* chessboard;
public:
	SoldierNormalAttack(int atk, Chessboard* chessboard);

	void attack(int target);
};

class SoldierBuildingAttack : public AttackBehavior {
private:
	int atk;
	Chessboard* chessboard;
public:
	SoldierBuildingAttack(int atk, Chessboard* chessboard);

	void attack(int target);
};