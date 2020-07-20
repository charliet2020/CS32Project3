#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h" //newly added include
#include "Level.h"
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>
#include <iomanip>  // defines the manipulator setw
#include <cmath>
using namespace std;

/*
2/21 2:45 pm: Here's a clarification of what StudentWorld::init() must return; 
it replaces the second paragraph of p. 15 of the spec:
When the player has finished the level loaded from level01.txt, the next 
level data file to load is level02.txt; after level02.txt, level03.txt; etc. 
If there is no level data file with the next number, or if the level just completed 
is level 99, the init() method must return GWSTATUS_PLAYER_WON. If the next level file 
exists but is not in the proper format for a level data file, the init() method must 
return GWSTATUS_LEVEL_ERROR. Otherwise, the init() method initializes your data 
structures/objects for the current level and returns GWSTATUS_CONTINUE_GAME.

Note: some comments with expressions are just previous scratchwork and attempts that were there to facilitate my own understanding
*/

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_penelope(nullptr) , m_levelFinished(false), m_destruct(0), m_construct(0)
{
}

//2/18/19 Done with minimum. Now implementing Blocking
bool StudentWorld::interference(double x, double y) const
{
	//cerr << "HI GUYS!" << endl;
	list<Actor*>::const_iterator it = m_actors.begin();

	for (; it != m_actors.end(); it++) 
		if ( !((*it)->canoverlap()) && (*it)->getX() <= x + SPRITE_WIDTH - 1 && (*it)->getX() + SPRITE_WIDTH - 1 >= x &&
			(*it)->getY() <= y + SPRITE_HEIGHT -1 && (*it)->getY() + SPRITE_HEIGHT - 1 >= y ) 
		{ //prevents penelope from moving into other objects
			
			return true;
		}

	/*
	//edited for loop condition
		if ((x + 2 == (*it)->getX() && y == (*it)->getY() || x - 2 == (*it)->getX() && y == (*it)->getY()
			|| x == (*it)->getX() && y + 2 == (*it)->getY() || x == (*it)->getX() && y - 2 == (*it)->getY())
			&& !((*it)->canoverlap()))
			continue; //Suppose citizen A calls this. We want to ignore citizen A from the list
		else 
	*/

	//cerr << "no interference" << endl;
	return false;
}
bool StudentWorld::interference(double x1, double y1, double x2, double y2) const
{
	//x1 and y1 will be used by object1, while x2 and y2 are for object2
	//e.g. citizen checking nearest Penelope
	list<Actor*>::const_iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
		if (((x1 + 2 == (*it)->getX() && y1 == (*it)->getY()) || (x1 - 2 == (*it)->getX() && y1 == (*it)->getY())
			|| (x1 == (*it)->getX() && y1 + 2 == (*it)->getY()) || (x1 == (*it)->getX() && y1 - 2 == (*it)->getY()))
			&& !((*it)->canoverlap()))
			; //Suppose citizen A calls this. We want to ignore citizen A from the list
		else if (((x1 + 1 == (*it)->getX() && y1 == (*it)->getY()) || (x1 - 1 == (*it)->getX() && y1 == (*it)->getY())
			|| (x1 == (*it)->getX() && y1 + 1 == (*it)->getY()) || (x1 == (*it)->getX() && y1 - 1 == (*it)->getY()))
			&& !((*it)->canoverlap()))
			; //suppose zombie A calls this. We want to ignore zombie A from list
		else if (!((*it)->canoverlap()) && (*it)->getX() <= x1 + SPRITE_WIDTH - 1 && (*it)->getX() + SPRITE_WIDTH - 1 >= x1 &&
			(*it)->getY() <= y1 + SPRITE_HEIGHT - 1 && (*it)->getY() + SPRITE_HEIGHT - 1 >= y1) //prevents other citizens from collision
			return true;
		else if (x1 <= x2 + SPRITE_WIDTH - 1 && x1 + SPRITE_WIDTH - 1 >= x2 &&
			y1 <= y2 + SPRITE_HEIGHT - 1 && y1 + SPRITE_HEIGHT - 1 >= y2) //checks for penelope
			return true;

	return false;
}

//Change: added an else statement to accomodate for zombies using interference
/*
bool StudentWorld::zombiemove(double x1, double y1, double x2, double y2)
{


	return false;
}
*/

double StudentWorld::distAway(double x, double y, double& zx, double& zy) const //pass in citizen's coordinates and some arbitrary numbers, 
{
	list<Actor*>::const_iterator it = m_actors.begin(); 
	double dist = 0; zx = 0, zy = 0;
	for (; it != m_actors.end(); it++)
		if ((*it)->canKill())
		{
			double x2 = (*it)->getX(), y2 = (*it)->getY();
			double x_temp = x - x2, y_temp = y - y2;
			double d = x_temp * x_temp + y_temp * y_temp;
			if (d < dist) //if we find a smaller d, then update to new min
			{
				dist = d;
				zx = x2, zy = y2; //used to get coordinates of nearest zombie
			}
			else if (dist == 0) //first time we find zombie, assume that one is min
			{
				dist = d;
				zx = x2, zy = y2;
			}
		}
		else continue;
	return dist;
}

void StudentWorld::actorCounter(int& obj1, int& obj2) const //let obj1 be citizen count, obj2 be zombie count
{
	obj1 = 0, obj2 = 0;
	list<Actor*>::const_iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
		if ((*it)->canexit())
			obj1++;
		else if ((*it)->canKill())
			obj2++;
}

bool StudentWorld::overlap(double x1, double y1) //used by exit only
{
	//let x1 and y1 be the coordinate of the object (e.g. exit)

	list<Actor*>::const_iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
	{
		if ((*it)->canexit())
		{
			double c_x = (*it)->getX() - x1, c_y = (*it)->getY() - y1;
			if (c_x*c_x + c_y * c_y <= 10 * 10)
			{
				if ((*it)->checkliving()) //prevents double counting score when citizen goes in between 2 exits
				{
					(*it)->resetStatus();
					playSound(SOUND_CITIZEN_SAVED);
					cerr << "increase score" << endl;
					increaseScore(500);
				}
				return true;
			}
		}
	}

	double p_x = m_penelope->getX() - x1, p_y = m_penelope->getY() - y1;
	int c = 0, z = 0; actorCounter(c, z);

	if (p_x * p_x + p_y * p_y <= 10 * 10 && c == 0)
		return true;

	//cerr << "level not over" << endl;
	return false;
}

bool StudentWorld::overlap(double x1, double y1, double& x2, double& y2) //used to compare other objects (mainly Pit)
{
	list<Actor*>::const_iterator it = m_actors.begin(); x2 = 0, y2 = 0;
	//x1 and y1 are coordinates of exit
	for (; it != m_actors.end(); it++) //since only zombies and humans can move, pits will never need to check if wall is within range
	{
		double xt = (*it)->getX(), yt = (*it)->getY(); double x = x1 - xt, y = y1 - yt;
		if ((*it)->canKill() && x1 != xt && y1 != yt && x*x + y*y <= 10*10) //only zombies allowed here
		{
			if ((*it)->checkliving())
			{
				(*it)->resetStatus();
				playSound(SOUND_ZOMBIE_DIE);
				if (!(*it)->isSmart())
				{
					cerr << "increase score by 1000" << endl;
					increaseScore(1000);
					int x = randInt(1, 10), y = randInt(1, 4);
					if (x == 1)
					{
						cerr << "1/10 success" << endl;
						if (y == 1 && canaddVaccine((*it)->getX(), (*it)->getY() + SPRITE_HEIGHT)) //edit: replaced canadd with canaddVaccine
						{
							m_actors.push_back(new VaccineGoodie((*it)->getX(), (*it)->getY() + SPRITE_HEIGHT, this));
						}
						else if (y == 2 && canaddVaccine((*it)->getX(), (*it)->getY() - SPRITE_HEIGHT))
							m_actors.push_back(new VaccineGoodie((*it)->getX(), (*it)->getY() - SPRITE_HEIGHT, this));
						else if (y == 3 && canaddVaccine((*it)->getX() + SPRITE_WIDTH, (*it)->getY()))
							m_actors.push_back(new VaccineGoodie((*it)->getX() + SPRITE_WIDTH, (*it)->getY(), this));
						else if (y == 4 && canaddVaccine((*it)->getX() - SPRITE_WIDTH, (*it)->getY()))
							m_actors.push_back(new VaccineGoodie((*it)->getX() - SPRITE_WIDTH, (*it)->getY(), this));
					}
				}
				else
				{
					cerr << "increase score by 2000" << endl;
					increaseScore(2000);
				}
			}
			return true;
		}
		else if (x*x + y * y <= 10 * 10 && (*it)->canexit())//citizens here
		{
			if ((*it)->checkliving())
			{
				cerr << "citizen death" << endl;
				(*it)->resetStatus();
				playSound(SOUND_CITIZEN_DIE);
				increaseScore(-1000);
			}
			return true;
		}
		/*
		else if (x*x + y * y <= 10 * 10 && (*it)->isFlammable())
		{
			if (!(*it)->canKill() && !(*it)->canexit() && !(*it)->canexplode() && (*it)->checkliving())
			{
				cerr << "vaccine/gas/mine good death" << endl;
				(*it)->resetStatus();
			}
			else if ((*it)->checkliving() && (*it)->canexplode())
			{
				cerr << "mine death" << endl;
				(*it)->resetStatus();
				(*it)->beginexplosion();
			}
		}
		*/
	}
	//since we pass by reference, changing x2 and y2 means Penelope must have died if we return true
	x2 = m_penelope->getX(), y2 = m_penelope->getY(); double a = x1 - x2, b = y1 - y2;
	if (a*a + b * b <= 10 * 10)
	{
		cerr << "penelope death" << endl;
		m_penelope->resetStatus();
		return true;
	}
	return false;
}

void StudentWorld::burn(double x1, double y1, double &x2, double &y2)
{ //attempt: use intersection of rectangles rather than overlap for flame
	//reattempt: checked for overlap instead
	list<Actor*>::iterator it = m_actors.begin(); x2 = 0, y2 = 0;
	for (; it != m_actors.end(); it++)
	{
		x2 = (*it)->getX(), y2 = (*it)->getY(); double x = x1 - x2, y = y1 - y2;
		if (!(*it)->blockFlame() && (*it)->canKill() && !(*it)->canexplode() && x*x + y*y <= 10*10) //zombies here, x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
		{
			cerr << "intersection zombie " << endl;
			if ((*it)->checkliving())
			{
				(*it)->resetStatus();
				playSound(SOUND_ZOMBIE_DIE);
				if ((*it)->isSmart())
				{
					increaseScore(2000);
				}
				else
				{
					//calculate chance of dropping vaccine
					increaseScore(1000);
					int x = randInt(1, 10), y = randInt(1, 4);
					if (x == 1)
					{
						cerr << "1/10 success" << endl;
						if (y == 1 && canaddVaccine((*it)->getX(), (*it)->getY() + SPRITE_HEIGHT))
						{
							m_actors.push_back(new VaccineGoodie((*it)->getX(), (*it)->getY() + SPRITE_HEIGHT, this));
						}
						else if (y == 2 && canaddVaccine((*it)->getX(), (*it)->getY() - SPRITE_HEIGHT))
							m_actors.push_back(new VaccineGoodie((*it)->getX(), (*it)->getY() - SPRITE_HEIGHT, this));
						else if (y == 3 && canaddVaccine((*it)->getX() + SPRITE_WIDTH, (*it)->getY()))
							m_actors.push_back(new VaccineGoodie((*it)->getX() + SPRITE_WIDTH, (*it)->getY(), this));
						else if (y == 4 && canaddVaccine((*it)->getX() - SPRITE_WIDTH, (*it)->getY()))
							m_actors.push_back(new VaccineGoodie((*it)->getX() - SPRITE_WIDTH, (*it)->getY(), this));
					}
				}
			}
		}
		else if (!(*it)->blockFlame() && (*it)->canexit() && x*x + y * y <= 10 * 10) //citizens here, x2 < x1 + SPRITE_WIDTH - 1  x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
		{
			cerr << "intersection citizen" << endl;
			if ((*it)->checkliving())
			{
				(*it)->resetStatus();
				playSound(SOUND_CITIZEN_DIE);
				increaseScore(-1000);
			}
		}
		else if (!(*it)->blockFlame() && (*it)->isFlammable() && !(*it)->canexplode() && x*x + y * y <= 10 * 10) //all goodies,  x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
		{
			cerr << "intersection goodie" << endl;
			if ((*it)->checkliving())
				(*it)->resetStatus();
		}
		else if (!(*it)->blockFlame() && (*it)->canexplode() && x*x + y * y <= 10 * 10) //only landmines can explode, x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
		{
			cerr << "intersection landmine" << endl;
			if ((*it)->checkliving())
				(*it)->beginexplosion();
		}
	}

	x2 = m_penelope->getX(), y2 = m_penelope->getY(); double xt = x1 - x2, yt = y1 - y2;
	if (xt * xt + yt*yt <= 10*10)//x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
	{
		m_penelope->resetStatus();
		return;
	}
}

bool StudentWorld::vomit(double x1, double y1, double &x2, double &y2)
{ //x1, y1 are vomit coordinates while x2,y2 are human coordinates
	int r = randInt(1, 3);
	double x = x1 - x2, y = y1 - y2;
	if (x*x + y * y <= 10*10 && r == 1) // 1 in 3 chance of vomiting
		return true;
	else
	{
		//cerr << "r: " << r << endl;
		//cerr << "euclidian distance: " << x * x + y * y << endl;
		return false;
	}
}

void StudentWorld::infect(double x1, double y1, double &x2, double &y2)
{ //change: went from rectangle intersection to overlap
	list<Actor*>::iterator it = m_actors.begin(); x2 = 0, y2 = 0;
	//x2 = (*it)->getX(), y2 = (*it)->getY();
	for (; it != m_actors.end(); it++)
	{
		x2 = (*it)->getX(), y2 = (*it)->getY(); double x = x1 - x2, y = y1 - y2;
		if ((*it)->canexit() && x2 < x1 + SPRITE_WIDTH - 1 && x*x + y*y <= 10*10) //infect citizens, x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
		{
			if (!(*it)->checkInfection())
			{
				(*it)->infectme();
				playSound(SOUND_CITIZEN_INFECTED);
			}
		}
	}

	x2 = m_penelope->getX(), y2 = m_penelope->getY(); double xt = x1 - x2, yt = x1 - x2;
	if (xt*xt + yt * yt <= 10*10) //x2 < x1 + SPRITE_WIDTH - 1 && x2 + SPRITE_WIDTH - 1 > x1 && y2 < y1 + SPRITE_HEIGHT - 1 && y2 + SPRITE_HEIGHT - 1 > y1
	{
		if (!m_penelope->checkInfection())
		{
			m_penelope->infectme();
		}
	}
	//infect Penelope
}

double StudentWorld::minDist(double x1, double y1, double& x2, double& y2) const
{
	list<Actor*>::const_iterator it = m_actors.begin(); x2 = 0, y2 = 0;
	//x1 and y1 are coordinates of zombie that calls this
	double dist = 0;

	if (x2 == m_penelope->getX() && y2 == m_penelope->getY())
	{
		double x = x1 - x2, y = y1 - y2;
		dist = x * x + y * y;
		return dist;
	}
	else
	{
		double min = 0;
		for (; it != m_actors.end(); it++)
			if ((*it)->canexit()) //only care about citizens
			{
				double xc = (*it)->getX(), yc = (*it)->getY();
				double x = x1 - x2, y = y1 - y2;
				double d = x * x + y * y;
				if (d < dist)
				{
					dist = d;
					x2 = xc, y2 = yc; //update coordinates of citizen
				}
				else if (dist == 0) //means first citizen found is closest
				{
					dist = d;
					x2 = xc, y2 = yc;
				}
			}
		return dist;
	}

}

int StudentWorld::init()
{
	//Loading Level
	Level lev(assetPath());

	m_construct = 0, m_destruct = 0;
	ostringstream oss;
	oss << "level0" << to_string(getLevel()) << ".txt";
	string levelFile = oss.str();

	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found || getLevel() > 99)
	{
		cerr << "Cannot find level0" << getLevel() << ".txt data file" << endl;
		return GWSTATUS_PLAYER_WON;
	}
	else if (result == Level::load_fail_bad_format)
	{
		cerr << "Your level was improperly formatted" << endl;
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level::load_success)
	{
		cerr << "Successfully loaded level" << endl;

		for (int levelY = 0; levelY < LEVEL_HEIGHT; levelY++) //decrease from 15 to 0: Penelope(7, 14)
			for (int levelX = 0; levelX < LEVEL_WIDTH; levelX++)
			{
				Level::MazeEntry ge = lev.getContentsOf(levelX, levelY); // level_x=5, level_y=10
														// so x=80 and y=160
				switch (ge)
				{
				case Level::empty:
					//cerr << "Location " << levelX << ", " << levelY << " is empty\n";
					break;
				case Level::player:				
					//cerr << "Location " << levelX << ", " << levelY << " is where Penelope starts\n";

					if (m_penelope == nullptr) //prevents duplicate Penelope
						m_penelope = new Penelope(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this); //created a penelope character, added this pointer as third parameter of studentWorld parameter
					Construction();
					break;
				case Level::wall:
					//cerr << "Location " << levelX << ", " << levelY << " holds a Wall\n";
					m_actors.push_back(new Wall(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::exit:
					m_actors.push_back(new Exit(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::citizen:
					m_actors.push_back(new Citizen(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::dumb_zombie:
					m_actors.push_back(new DumbZombie(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::smart_zombie:
					m_actors.push_back(new SmartZombie(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::gas_can_goodie:
					m_actors.push_back(new GasCanGoodie(levelX*SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::landmine_goodie:
					m_actors.push_back(new LandmineGoodie(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::vaccine_goodie:
					m_actors.push_back(new VaccineGoodie(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				case Level::pit:
					m_actors.push_back(new Pit(levelX * SPRITE_WIDTH, levelY * SPRITE_HEIGHT, this));
					Construction();
					break;
				default:
					cerr << "Location is other" << endl;
					break;
				}

			}

	}
	//Construction();
	cerr << "constructor: " << getConstructor() << endl;

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	m_levelFinished = false;
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
	//if (getLives() == 0)
		//return GWSTATUS_PLAYER_DIED;

	//for part 1, assume penelope doesn't die
	if (m_penelope->checkliving())
		m_penelope->doSomething(); //added doSomething
	
	

	list<Actor*>::iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
		if ((*it)->checkliving())
			(*it)->doSomething();

	if (!m_penelope->checkliving())
	{
		cerr << "penelope lost a life" << endl;
		playSound(SOUND_PLAYER_DIE);
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (m_levelFinished)
	{
		playSound(SOUND_LEVEL_FINISHED);
		return GWSTATUS_FINISHED_LEVEL;
	}


	it = m_actors.begin();
	while (it != m_actors.end())
	{
		if (!(*it)->checkliving())
		{
			delete (*it);
			it = m_actors.erase(it);
			Destruction();
			cerr << "destructor: " << getDestructor() << endl;
		}
		else it++;
	}
	
	//Update game status line
	ostringstream oss;
	oss.fill('0');
	if (getScore() < 0)
	{
		oss << "Score: " << "-" << setw(5) << to_string(getScore() * -1) << "  Level: " << to_string(getLevel()) << "  Lives: "
			<< to_string(getLives()) << "  Vaccines: " << to_string(m_penelope->vaccineCount()) << "  Flames: " << to_string(m_penelope->flameCount())
			<< "  Mines: " << to_string(m_penelope->mineCount()) << "  Infected: " << to_string(m_penelope->infectCount());
	}
	else
	{
		oss << "Score: " << setw(6) << to_string(getScore()) << "  Level: " << to_string(getLevel()) << "  Lives: "
			<< to_string(getLives()) << "  Vaccines: " << to_string(m_penelope->vaccineCount()) << "  Flames: " << to_string(m_penelope->flameCount())
			<< "  Mines: " << to_string(m_penelope->mineCount()) << "  Infected: " << to_string(m_penelope->infectCount());
	}
	string text = oss.str();
	setGameStatText(text);

    
	return GWSTATUS_CONTINUE_GAME;

}

void StudentWorld::addActor(Actor* p)
{
	m_actors.push_back(p);
}

bool StudentWorld::canadd(double x, double y) const
{
	list<Actor*>::const_iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
	{
		double x1 = (*it)->getX(), y1 = (*it)->getY();
		double xf = x1 - x, yf = y1 - y;
		if ((*it)->blockFlame() && xf*xf + yf*yf <= 10*10) //only wall and exit can block flames
			return false;
	}

	return true;
}

bool StudentWorld::canaddVaccine(double x, double y) 
{
	list<Actor*>::iterator it = m_actors.begin();
	for ( ; it!=m_actors.end(); it++)
	{
		double x1 = (*it)->getX(), y1 = (*it)->getY();
		double xf = x1 - x, yf = y1 - y;
		if (xf*xf + yf * yf <= 10 * 10) //do not put vaccine if new vaccine will overlap with an existent object
			return false; //not sure if they mean bounding box
	}
	return true;
}

void StudentWorld::findHuman(double x1, double y1, double &x2, double &y2) //compare citizen and zombie
{
	list<Actor*>::iterator it = m_actors.begin();
	for (; it != m_actors.end(); it++)
	{
		double x2 = (*it)->getX(), y2 = (*it)->getY();
		if ((*it)->canexit() &&(x1 == x2 || y1 == y2)) //only worry about citizens in list
			return;
	}

	return;
}

void StudentWorld::cleanUp()
{
	//later, this may be called prematurely when Penelope dies or finishes level
	//perhaps set to nullptr after deletion
	list<Actor*>::iterator it = m_actors.begin();

	while (it != m_actors.end())
	{
		delete (*it);
		it = m_actors.erase(it);
		Destruction();
		cerr << "destructor count: " << getDestructor() << endl;
	}
	delete m_penelope;
	m_penelope = nullptr; //prevents dangling pointer issue, and is better than accessing deleted pointer
	Destruction();
	cerr << "destructor count: " << getDestructor() << endl;

	//delete m_wall;
}

StudentWorld::~StudentWorld()
{
	//cerr << "before StudentWorld cleanUp" << endl;
	cleanUp();
	//cerr << "after StudentWorld cleanUp" << endl;
}
