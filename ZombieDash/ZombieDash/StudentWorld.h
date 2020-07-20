#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class Penelope; //to make pointer valid
class Actor;
class Wall;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
	virtual bool interference(double x, double y) const; //used to access beginning of m_actors for Penelope
	virtual bool overlap(double x1, double y1); //used by exit to get Penelope/citizen to leave level
	virtual bool interference(double x1, double y1, double x2, double y2) const; //used by other actors themselves
	virtual bool overlap(double x1, double y1, double& x2, double& y2); //can used by flame, vomit, and pit
	virtual void burn(double x1, double y1, double& x2, double& y2);
	bool vomit(double x1, double y1, double& x2, double& y2);
	void infect(double x1, double y1, double& x2, double& y2);
	double minDist(double x1, double y1, double& x2, double& y2) const;

	void findHuman(double x1, double y1, double& x2, double& y2);

	virtual double distAway(double x, double y, double& zx, double& zy) const;
	virtual void actorCounter(int& obj1, int& obj2) const; //used to count #zombies and citizens in level

	//void addActor(char actor, double x, double y, Direction dir);
	void addActor(Actor* p);
	bool canaddVaccine(double x, double y);
	bool canadd(double x, double y) const;

	virtual void levelFinished() { m_levelFinished = true; } //can be called 
	virtual bool getLevelFinished() const { return m_levelFinished; }

	int getDestructor() const { return m_destruct; }
	int getConstructor() const { return m_construct; }
	void Destruction() { m_destruct++; }
	void Construction() { m_construct++; }

	Actor* getPenelope() const { return m_penelope; } //so citizen can access Penelope's coordinates
	//StudentWorld's destructor
	virtual ~StudentWorld();
private:
	Actor* m_penelope; //changed Penelope* to Actor*
	std::list<Actor*> m_actors; //list of actor pointers, including wall
	bool m_levelFinished;
	int m_destruct;
	int m_construct;
};

#endif // STUDENTWORLD_H_
