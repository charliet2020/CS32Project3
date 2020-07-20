#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"


class StudentWorld;
class Actor : public GraphObject
{
public: //change: getting rid of double size, replace with StudentWorld* sWorld
	Actor(int imageID, double startX, double startY, Direction dir , int depth , StudentWorld* sworld);
	virtual void doSomething() = 0;
	//virtual bool living() { m_alive = true;} //Penelope, zombies and citizens are alive
	StudentWorld* getWorld() { return m_world; }
	
	//part 2 stuff
	virtual bool checkliving() const { return m_alive; }
	virtual bool canhurt() const { return true; } //only walls, pits and exits are immune
	virtual bool canoverlap() const { return m_canoverlap; } //using euclidean distance
	virtual bool canexit() const { return m_canexit; } //only Penelope and actors may pass through exits
	virtual bool canKill() const { return m_cankill; }
	virtual bool checkInfection() const { return m_infectStatus; }
	virtual bool isSmart() const { return false; }
	virtual bool isFlammable() const { return true; } //flames cannot pass through exits or walls
	virtual bool blockFlame() const { return false; }
	virtual bool canexplode() const { return false; }

	virtual void beginexplosion() {}

	int infectCount() const { return m_infectCount; }


	void resetOverlap() { m_canoverlap = true; } //enable us to step on exits/goodies/pits/flame/mines
	void resetCanexit() { m_canexit = !m_canexit; }
	void resetCankill() { m_cankill = !m_cankill; }
	void resetInfection() { m_infectStatus = false, m_infectCount = 0; } //enables vaccine to cure
	void resetStatus() { m_alive = false; } //no problem if called multiple times
	void increaseInfect() { m_infectCount++; }
	void infectme() { m_infectStatus = true; }

	//paralysis implementation for citizens and zombies
	bool isParalyzed() const { return m_paralyzed; }
	void resetParalysis() { m_paralyzed = !m_paralyzed; }

	//Just to enable m_penelope to access Penelope's version. These don't do anything special 
	virtual int flameCount() const { return -1; }
	virtual int vaccineCount() const { return -1; }
	virtual int mineCount() const { return -1; }
	virtual void changeFlame(int n) { ; }
	virtual void changeVaccine(int n) { ; }
	virtual void changeMine(int n) { ; }
	void constructor() { m_construct++; }
	void destructor() { m_destruct++; }
	int getConstruct() { return m_construct; }
	int getDestruct() { return m_destruct; }


	virtual ~Actor();
private:
	StudentWorld* m_world; //to access things in StudentWorld
	bool m_alive; //alive status 
	bool m_infectStatus; //infection status
	int m_infectCount; //track infection count
	bool m_canoverlap; // help check overlap 
	bool m_paralyzed; //check for paralysis
	bool m_canexit; //give permission to leave level
	bool m_cankill; //checks for zombies in list
	int m_construct; //debuggin
	int m_destruct;
};

//For part 1, assume Penelope is always alive
class Penelope : public Actor
{
public:
	Penelope(double start_x, double start_y, StudentWorld* sworld);
	virtual void doSomething();

	virtual ~Penelope();

	virtual bool canexit() const { return true; }

	virtual int flameCount() const { return m_flamecharges; }
	virtual int vaccineCount() const { return m_vaccines; }
	virtual int mineCount() const { return m_mines; }

	virtual void changeFlame(int n) { m_flamecharges += n; }
	virtual void changeVaccine(int n) { m_vaccines += n; }
	virtual void changeMine(int n) { m_mines += n; }
private:
	int m_flamecharges;
	int m_mines;
	int m_vaccines;
};

class Wall : public Actor
{
public:
	Wall(double startX, double startY, StudentWorld* sworld);
	virtual void doSomething();
	virtual ~Wall() {}
	virtual bool canhurt() const { return false; } //although walls and exits can block flame, pits cannot
	virtual bool isFlammable() const { return false; }
	virtual bool blockFlame() const { return true; }
private:
};

class Exit : public Actor
{
public:
	Exit(double startX, double startY, StudentWorld* sworld);
	virtual void doSomething();
	virtual bool canhurt() const { return false; }
	virtual bool isFlammable() const { return false; }
	virtual ~Exit() {}
	virtual bool blockFlame() const { return true; }

private:
};


class Citizen : public Actor
{
public:
	Citizen(double startX, double startY, StudentWorld* sworld);
	virtual void doSomething();
	virtual ~Citizen() {}

private:
};

class Zombie : public Actor
{
public:
	Zombie(double startX, double startY, StudentWorld* sworld);
	virtual ~Zombie() {}
	int getMovePlan() const { return m_moveplan; }
	void resetMove(int x) { m_moveplan = x; }
	void changeMove(int x) { m_moveplan += x; }
private:
	int m_moveplan;
};

class DumbZombie : public Zombie
{
public:
	DumbZombie(double startX, double startY, StudentWorld* sworld);
	virtual void doSomething();
	virtual ~DumbZombie() {}
private:
};

class SmartZombie : public Zombie
{
public:
	SmartZombie(double startX, double startY, StudentWorld* sworld);
	virtual void doSomething();
	virtual ~SmartZombie() {}
	virtual bool isSmart() const { return true; }
private:
};

class Goodie : public Actor
{
public: 
	Goodie(int imageID, double startX, double startY, StudentWorld* sworld);
	virtual ~Goodie() {}

private:
};

class GasCanGoodie : public Goodie
{
public:
	GasCanGoodie(double startX, double startY, StudentWorld* sworld);
	virtual ~GasCanGoodie() {}
	virtual void doSomething();
private:
};


class VaccineGoodie : public Goodie
{
public:
	VaccineGoodie(double startX, double startY, StudentWorld* sworld);
	virtual ~VaccineGoodie() {}
	virtual void doSomething();
private:
};

class LandmineGoodie : public Goodie
{
public:
	LandmineGoodie(double startX, double startY, StudentWorld* sworld);
	virtual ~LandmineGoodie() {}
	virtual void doSomething();
private:
};

class Landmine : public Actor
{
public:
	Landmine(double startX, double startY, StudentWorld* sworld);
	virtual ~Landmine() {}
	virtual void doSomething();
	virtual bool canexplode() const { return true; }
	virtual void beginexplosion();
	int safetyCount() const { return m_safetytick; }
	bool isActive() const { return m_active; }
	void activate() { m_active = true; }
private:
	int m_safetytick;
	bool m_active;
};

class Pit : public Actor
{
public:
	Pit(double startX, double startY, StudentWorld* sworld);
	virtual ~Pit() {}
	virtual void doSomething();
	virtual bool canhurt() const { return false; }
	virtual bool isFlammable() const { return false; }
private:
};

class Projectile : public Actor
{
public:
	Projectile(int imageID, double startX, double startY, Direction dir, StudentWorld* sworld);
	virtual ~Projectile() {}
	int tickCount() const { return m_tickCount; }
	virtual bool canhurt() const { return false; }
	void increaseTick() { m_tickCount++; }
	virtual bool isFlammable() const { return false; }
private:
	int m_tickCount;
};

class Flame : public Projectile
{
public:
	Flame(double startX, double startY, Direction dir, StudentWorld* sworld);
	virtual ~Flame() {}
	virtual void doSomething();
private:
};

class Vomit : public Projectile
{
public:
	Vomit(double startX, double startY, Direction dir, StudentWorld* sworld);
	virtual ~Vomit() {}
	virtual void doSomething();
private:

};
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

#endif // ACTOR_H_
