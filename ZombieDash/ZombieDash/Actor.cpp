#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>
using namespace std;

//Actor Implementation
Actor::Actor(int imageID, double startX, double startY, Direction dir, int depth, StudentWorld* sworld)
	: GraphObject(imageID, startX, startY, dir, depth), m_world(sworld), m_alive(true), 
	m_infectStatus(false), m_infectCount(0), m_canoverlap(false), m_paralyzed(false), m_canexit(false),
	m_cankill(false), m_construct(0), m_destruct(0)
{
	//constructor();
	//cerr << "construct count: " << getConstruct() << endl;
}

Actor::~Actor()
{}

//Penelope Implementation
Penelope::Penelope(double start_x, double start_y, StudentWorld* sworld)
	: Actor(IID_PLAYER, start_x, start_y, right, 0, sworld) , m_flamecharges(0),
	m_mines(0), m_vaccines(0)
{}

void Penelope::doSomething()
{
	if (!checkliving())
		return;

	//checking infection
	if (checkInfection())
		increaseInfect();
	if (infectCount() == 500)
	{
		resetStatus();
		return;
	}
	//adding movement
	int ch;
	if (getWorld()->getKey(ch)) 
	{
		
		//user hit a key during tick
		switch (ch)
		{
			//Note: she moves exactly 4 pixels
		case KEY_PRESS_LEFT:
			setDirection(left);
			if (!getWorld()->interference(getX() - 4, getY()))
			moveTo(getX() - 4, getY()); 
			break;
		case KEY_PRESS_RIGHT:
			setDirection(right);
			if (!getWorld()->interference(getX() + 4, getY()))
			moveTo(getX() + 4, getY());
			break;
		case KEY_PRESS_UP: 
			setDirection(up);
			//cerr << "Case: (Penelope x, Penelope y)" << getX() << ", " << getY() << endl;
			if (!(getWorld()->interference(getX(), getY() + 4)))
				moveTo(getX(), getY() + 4);
			break;
		case KEY_PRESS_DOWN:
			setDirection(down);
			if (!getWorld()->interference(getX(), getY() - 4))
			moveTo(getX(), getY() - 4);
			break;
		case KEY_PRESS_ENTER: //using vaccine
			if (vaccineCount() > 0)
			{
				changeVaccine(-1);
				resetInfection(); //shel'll waste the vaccine if she isn't infected
			}
			break;
		case KEY_PRESS_SPACE:  //using flamethrower
			if (flameCount() > 0)
			{
				changeFlame(-1);
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				cerr << "flames created from player" << endl;
					switch (getDirection())
					{
					case left:
						for (int i = 1; i <= 3; i++)
							if (getWorld()->canadd(getX() - i * SPRITE_WIDTH, getY()))
								getWorld()->addActor(new Flame(getX() - i * SPRITE_WIDTH, getY(), left, getWorld()));
							else break;
						break;
					case right:
						for (int i = 1; i <= 3; i++)
							if (getWorld()->canadd(getX() + i * SPRITE_WIDTH, getY()))
								getWorld()->addActor(new Flame(getX() + i * SPRITE_WIDTH, getY(), right, getWorld()));
							else break;
						break;
					case up:
						for (int i = 1; i <= 3; i++)
							if (getWorld()->canadd(getX(), getY() + i * SPRITE_HEIGHT))
							{
								getWorld()->addActor(new Flame(getX(), getY() + i * SPRITE_HEIGHT, up, getWorld()));
							}
	
							else break;
						break;
					case down:
						for (int i = 1; i <= 3; i++)
							if (getWorld()->canadd(getX(), getY() - i*SPRITE_HEIGHT))
								getWorld()->addActor(new Flame(getX() , getY() - i*SPRITE_HEIGHT, down, getWorld()));
							else break;
						break;
					default:
						break;
					}
			}
			break;
		case KEY_PRESS_TAB: //using mine
			if (mineCount() > 0)
			{
				changeMine(-1);
				getWorld()->addActor(new Landmine(getX(), getY(), getWorld()));
			}
			break;
		default:
			break;
		}
	}

}

Penelope::~Penelope()
{}

//Wall Implementation
Wall::Wall(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_WALL, startX, startY, right, 0, sworld) //idk what to put for size
{}
void Wall::doSomething()
{}


//Exit Implementation
Exit::Exit(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_EXIT, startX, startY, right, 1, sworld)
{
	resetOverlap(); //since my list assumes all actors cannot intersect, I specifically give Exit permission to change its overlap member variable
}

void Exit::doSomething()
{
	
	bool overlap = getWorld()->overlap(getX(), getY());
	int c = 0, z = 0; getWorld()->actorCounter(c, z);

	if (overlap && c == 0)
		getWorld()->levelFinished();
	else if (overlap) { return; } 
	
}

//Citizen Implementation
Citizen::Citizen(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_CITIZEN, startX, startY, right, 0, sworld)
{
	resetCanexit();
}

void Citizen::doSomething()
{
	if (!checkliving()) //check to see if alive
		return;

	//check infect status
	if (checkInfection())
		increaseInfect();

	//play sound only when infected first time

	if (infectCount() == 500)
	{
		resetStatus(); //zombie born
		getWorld()->playSound(SOUND_ZOMBIE_BORN);
		getWorld()->increaseScore(-1000);
		
		//introducing new zombie
		int x = randInt(1, 10);
		if (x > 3) //possibly buggy since not sure how to create in studentworld object
		{
			getWorld()->addActor(new DumbZombie(getX(), getY(), getWorld()));
		}
		else
		{
			getWorld()->addActor(new SmartZombie(getX(), getY(), getWorld()));
		}

		return;
	}
	//check paralysis
	
	if (isParalyzed())
	{
		resetParalysis();
		return;
	}
	else resetParalysis();
	

	//determining distance to Penelope and nearest zombie
	double x = getX() - getWorld()->getPenelope()->getX(), 
		y = getY() - getWorld()->getPenelope()->getY();
	double dis_p = x * x + y * y;

	int c = 0, z = 0;
	getWorld()->actorCounter(c, z); //used to count citizens and zombies each

	
	double p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	double zx = 0, zy = 0, dis_z = getWorld()->distAway(getX(), getY(), zx, zy);
	//at this point, zx and zy should change, or else we know no zombies in level
	if (dis_p <= 80 * 80 && (z == 0 || dis_p < dis_z)) 
	{
		if (getY() == p_y ) //same row 
		{
			if (getX() < p_x && !getWorld()->interference(getX() + 2, getY(), p_x, p_y))
			{
				setDirection(right);
				moveTo(getX() + 2, getY());
				return;
			}
			else if (getX() > p_x && !getWorld()->interference(getX() - 2, getY(), p_x, p_y))
			{
				setDirection(left);
				moveTo(getX() - 2, getY());
				return;
			}
		}
		else if (getX() == p_x) //same column
		{
			if (getY() < p_y && !getWorld()->interference(getX(), getY() + 2, p_x, p_y))
			{
				setDirection(up);
				moveTo(getX(), getY() + 2);
				return;
			}
			else if (getY() > p_y && !getWorld()->interference(getX(), getY() - 2, p_x, p_y))
			{
				setDirection(down);
				moveTo(getX(), getY() - 2);
				return;
			}
		}
		else //neither same row nor same column
		{
			if (getY() < p_y && getX() < p_x) //Penelope is northeast of citizen
			{
				int x = randInt(1, 2);
				if (x == 1 && !getWorld()->interference(getX(), getY() + 2, p_x, p_y))
				{
					setDirection(up);
					moveTo(getX(), getY() + 2);
					return;
				}
				else if (!getWorld()->interference(getX() + 2, getY(), p_x, p_y)) //just automatically take other route, don't wait on chance
				{
					setDirection(right);
					moveTo(getX() + 2, getY());
					return;
				}
			}
			else if (getY() < p_y && getX() > p_x) //penelope is northwest of citizen
			{
				int x = randInt(1, 2);
				if (x == 1 && !getWorld()->interference(getX(), getY() + 2, p_x, p_y))
				{
					setDirection(up);
					moveTo(getX(), getY() + 2);
					return;
				}
				else if (!getWorld()->interference(getX() - 2, getY(), p_x, p_y))
				{
					setDirection(left);
					moveTo(getX() - 2, getY());
					return;
				}
			}
			else if (getX() < p_x && getY() > p_y) //penelope is southeast of citizen
			{
				int x = randInt(1, 2);
				if (x == 1 && !getWorld()->interference(getX() , getY() - 2, p_x, p_y))
				{
					setDirection(down);
					moveTo(getX(), getY() - 2);
					return;
				}
				else if (!getWorld()->interference(getX() + 2, getY(), p_x, p_y))
				{
					setDirection(right);
					moveTo(getX() + 2, getY());
					return;
				}
			}
			else if (getX() > p_x && getY() > p_y) //penelope is southwest of citizen
			{
				int x = randInt(1, 2);
				if (x == 1 && !getWorld()->interference(getX(), getY() - 2, p_x, p_y))
				{
					setDirection(down);
					moveTo(getX(), getY() - 2);
					return;
				}
				else if (!getWorld()->interference(getX() -2, getY(), p_x, p_y))
				{
					setDirection(left);
					moveTo(getX() - 2, getY());
					return;
				}
			}
		}
	}

	else if (z != 0 && dis_z <= 80*80) //removed else here since we could have same row, but blocking happening
	{
	double dis_zNorth = -1, dis_zSouth = -1, dis_zWest = -1, dis_zEast = -1;
		if (!getWorld()->interference(getX(), getY() + 2, zx, zy)) //checks for blocking when moving up
		{
			dis_zNorth = getWorld()->distAway(getX(), getY() + 2, zx, zy);
		}
		if (!getWorld()->interference(getX(), getY() - 2, zx, zy)) //check for blocking moving down
		{
			dis_zSouth = getWorld()->distAway(getX(), getY() - 2, zx, zy);
		}
		if (!getWorld()->interference(getX() - 2, getY(), zx, zy)) //check for blocking moving left
		{
			dis_zWest = getWorld()->distAway(getX() - 2, getY(), zx, zy);
		}
		if (!getWorld()->interference(getX() + 2, getY(), zx, zy)) //check for blocking moving right
		{
			dis_zEast = getWorld()->distAway(getX() + 2, getY(), zx, zy);
		}
		double arr[5] = {dis_z, dis_zNorth, dis_zSouth, dis_zWest, dis_zEast};
		double max = arr[0];
		for (int i = 1; i < 5; i++) //find max among new distances and current distance
			if (arr[i] > max)
				max = arr[i];
		if (max == dis_z) //no better place for citizen to move to than current spot
			return;
		else if (max == dis_zNorth)
		{
			setDirection(up);
			moveTo(getX(), getY() + 2);
			return;
		}
		else if (max == dis_zSouth)
		{
			setDirection(down);
			moveTo(getX(), getY() - 2);
			return;
		}
		else if (max == dis_zWest)
		{
			setDirection(left);
			moveTo(getX() - 2, getY());
			return;
		}
		else //east must be max
		{
			setDirection(right);
			moveTo(getX() + 2, getY());
			return;
		}
	}
	else
	{} //do nothing

}

//Zombie Implementation
Zombie::Zombie(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_ZOMBIE, startX, startY, right, 0, sworld) , m_moveplan(0)
{
	resetCankill();
}

//DumbZombie Implementation
DumbZombie::DumbZombie(double startX, double startY, StudentWorld* sworld)
	: Zombie(startX, startY, sworld)
{}

void DumbZombie::doSomething()
{
	if (!checkliving()) //check if alive
		return;

	if (isParalyzed())
	{
		resetParalysis();
		return;
	}
	else resetParalysis();

	double xc = 0, yc = 0, p_x = getWorld()->getPenelope()->getX(), 
		p_y = getWorld()->getPenelope()->getY(); //arbitrary citizen coordinates that will be changed in findHuman
	getWorld()->findHuman(getX(), getY(), xc, yc);
	//change: got rid of the condition of being in the same row
	if (getDirection() == right ) //&& (getY() == yc || getY() == p_y)
	{
		if ((xc > getX() || p_x > getX()) && (getWorld()->vomit(getX() + SPRITE_WIDTH, getY(), 
			xc, yc) || getWorld()->vomit(getX() + SPRITE_WIDTH, getY(), p_x, p_y))) //someone is to the right
		{
			//cerr << "introduce vomit" << endl;
			getWorld()->addActor(new Vomit(getX() + SPRITE_WIDTH, getY(), right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == left ) //&& (getY() == yc || getY() == p_y)
	{
		if ((xc < getX() || p_x < getX()) && (getWorld()->vomit(getX() - SPRITE_WIDTH, getY(),
			xc, yc) || getWorld()->vomit(getX() - SPRITE_WIDTH, getY(), p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX() - SPRITE_WIDTH, getY(), right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == up ) //&& (getX() == xc || getX() == p_x)
	{
		if ((yc > getY() || p_y > getY()) && (getWorld()->vomit(getX(), getY() + SPRITE_HEIGHT, xc, yc)
			|| getWorld()->vomit(getX(), getY() + SPRITE_HEIGHT, p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX(), getY() + SPRITE_HEIGHT, right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == down ) //&& (getX() == xc || getX() == p_x)
	{
		if ((yc < getY() || p_y < getY()) && (getWorld()->vomit(getX(), getY() - SPRITE_HEIGHT, xc, yc)
			|| getWorld()->vomit(getX(), getY() - SPRITE_HEIGHT, p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX(), getY() - SPRITE_HEIGHT, right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}

	//check movement plan
	if (getMovePlan() == 0)
	{
		int x = randInt(3, 10), y = randInt(1, 4);
		resetMove(x);
		if (y == 1)
			setDirection(right);
		else if (y == 2)
			setDirection(up);
		else if (y == 3)
			setDirection(left);
		else if (y == 4)
			setDirection(down);
	}
	//determining destination coordinate
	p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	if (getDirection() == right)
	{
		if (!getWorld()->interference(getX() + 1, getY(), p_x, p_y))
		{
			moveTo(getX() + 1, getY());
			changeMove(-1);
		}
		else
		{
			resetMove(0);
		}
	}
	else if (getDirection() == up)
	{
		if (!getWorld()->interference(getX(), getY() + 1, p_x, p_y))
		{
			moveTo(getX(), getY() + 1);
			changeMove(-1);
		}
		else resetMove(0);
	}
	else if (getDirection() == left)
	{
		if (!getWorld()->interference(getX() - 1, getY(), p_x, p_y))
		{
			moveTo(getX() - 1, getY());
			changeMove(-1);
		}
		else resetMove(0);
	}
	else if (getDirection() == down)
	{
		if (!getWorld()->interference(getX(), getY() - 1, p_x, p_y))
		{
			moveTo(getX(), getY() - 1);
			changeMove(-1);
		}
		else resetMove(0);
	}
}

//SmartZombie Implementation
SmartZombie::SmartZombie(double startX, double startY, StudentWorld* sworld)
	: Zombie(startX, startY, sworld)
{}

void SmartZombie::doSomething()
{
	if (!checkliving()) //check if alive
		return;

	if (isParalyzed()) //paralysis ticks
	{
		resetParalysis();
		return;
	}
	else resetParalysis();

	//check person in front to vomit
	double xc = 0, yc = 0, p_x = getWorld()->getPenelope()->getX(),
		p_y = getWorld()->getPenelope()->getY(); //arbitrary citizen coordinates that will be changed in findHuman
	getWorld()->findHuman(getX(), getY(), xc, yc);
	//change: got rid of the condition of being in the same row
	if (getDirection() == right) //&& (getY() == yc || getY() == p_y)
	{
		if ((xc > getX() || p_x > getX()) && (getWorld()->vomit(getX() + SPRITE_WIDTH, getY(),
			xc, yc) || getWorld()->vomit(getX() + SPRITE_WIDTH, getY(), p_x, p_y))) //someone is to the right
		{
			//cerr << "introduce vomit" << endl;
			getWorld()->addActor(new Vomit(getX() + SPRITE_WIDTH, getY(), right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == left) //&& (getY() == yc || getY() == p_y)
	{
		if ((xc < getX() || p_x < getX()) && (getWorld()->vomit(getX() - SPRITE_WIDTH, getY(),
			xc, yc) || getWorld()->vomit(getX() - SPRITE_WIDTH, getY(), p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX() - SPRITE_WIDTH, getY(), right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == up) //&& (getX() == xc || getX() == p_x)
	{
		if ((yc > getY() || p_y > getY()) && (getWorld()->vomit(getX(), getY() + SPRITE_HEIGHT, xc, yc)
			|| getWorld()->vomit(getX(), getY() + SPRITE_HEIGHT, p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX(), getY() + SPRITE_HEIGHT, right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	else if (getDirection() == down) //&& (getX() == xc || getX() == p_x)
	{
		if ((yc < getY() || p_y < getY()) && (getWorld()->vomit(getX(), getY() - SPRITE_HEIGHT, xc, yc)
			|| getWorld()->vomit(getX(), getY() - SPRITE_HEIGHT, p_x, p_y)))
		{
			getWorld()->addActor(new Vomit(getX(), getY() - SPRITE_HEIGHT, right, getWorld()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	//new movement plan
	if (getMovePlan() == 0)
	{
		int x = randInt(3, 10); resetMove(x);

		//selecting person closest to smartzombie
		//minDist gets closest citizen available
		double dis_p = getWorld()->minDist(getX(), getY(), p_x, p_y);
		double xc_temp = 0, yc_temp = 0;
		double dis_c = getWorld()->minDist(getX(), getY(), xc_temp, yc_temp);

		//xc_temp will have new values after function since they're passed by reference
		if (dis_c <= dis_p)
		{
			if (dis_c > 80 * 80) //more than 80 pixels away
			{
				int i = randInt(1, 4);
				if (i == 1)
					setDirection(up);
				else if (i == 2)
					setDirection(down);
				else if (i == 3)
					setDirection(left);
				else if (i == 4)
					setDirection(right);
			}
			else //here, it's dis_c <= 80*80
			{
				if (getY() == yc_temp && getX() < xc_temp) //same row, zombie on left 
				{
					setDirection(right);
				}
				else if (getY() == yc_temp && getX() > xc_temp) //same row, zombie on right
				{
					setDirection(left);
				}
				else if (getX() == xc_temp && getY() < yc_temp) //same column, zombie below
				{
					setDirection(up);
				}
				else if (getX() == xc_temp && getY() > yc_temp) //same column, zombie above
				{
					setDirection(down);
				}
				else if (getX() < xc_temp && getY() < yc_temp) //citizen northeast of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(up);
					else setDirection(right);
				}
				else if (getX() < xc_temp && getY() > yc_temp) //citizen southeast of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(down);
					else setDirection(right);
				}
				else if (getX() > xc_temp && getY() < yc_temp) //citizen northwest of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(up);
					else setDirection(left);
				}
				else if (getX() > xc_temp && getY() < yc_temp) //citizen southwest of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(down);
					else setDirection(left);
				}
			}
		}
		else //penelope is closer
		{
			if (dis_p > 80 * 80)
			{
				int i = randInt(1, 4);
				if (i == 1)
					setDirection(up);
				else if (i == 2)
					setDirection(down);
				else if (i == 3)
					setDirection(left);
				else if (i == 4)
					setDirection(right);
			}
			else
			{
				if (getY() == p_y && getX() < p_x) //same row, zombie on left 
				{
					setDirection(right);
				}
				else if (getY() == p_y && getX() > p_x) //same row, zombie on right
				{
					setDirection(left);
				}
				else if (getX() == p_x && getY() < p_y) //same column, zombie below
				{
					setDirection(up);
				}
				else if (getX() == p_x && getY() > p_y) //same column, zombie above
				{
					setDirection(down);
				}
				else if (getX() < p_x && getY() < p_y) //Penelope northeast of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(up);
					else setDirection(right);
				}
				else if (getX() < p_x && getY() > p_y) //Penelope southeast of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(down);
					else setDirection(right);
				}
				else if (getX() > p_x && getY() < p_y) //Penelope northwest of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(up);
					else setDirection(left);
				}
				else if (getX() > p_x && getY() < p_y) //Penelope southwest of zombie
				{
					int i = randInt(1, 2);
					if (i == 1)
						setDirection(down);
					else setDirection(left);
				}
			}
		}
	}

	//determining destination coordinate
	p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	if (getDirection() == right)
	{
		if (!getWorld()->interference(getX() + 1, getY(), p_x, p_y))
		{
			moveTo(getX() + 1, getY());
			changeMove(-1);
		}
		else
		{
			resetMove(0);
		}
	}
	else if (getDirection() == up)
	{
		if (!getWorld()->interference(getX(), getY() + 1, p_x, p_y))
		{
			moveTo(getX(), getY() + 1);
			changeMove(-1);
		}
		else resetMove(0);
	}
	else if (getDirection() == left)
	{
		if (!getWorld()->interference(getX() - 1, getY(), p_x, p_y))
		{
			moveTo(getX() - 1, getY());
			changeMove(-1);
		}
		else resetMove(0);
	}
	else if (getDirection() == down)
	{
		if (!getWorld()->interference(getX(), getY() - 1, p_x, p_y))
		{
			moveTo(getX(), getY() - 1);
			changeMove(-1);
		}
		else resetMove(0);
	}
}

//Goodies Implementation
Goodie::Goodie(int imageID, double startX, double startY, StudentWorld* sworld)
	: Actor(imageID, startX, startY, right, 1, sworld)
{
	resetOverlap();
}

GasCanGoodie::GasCanGoodie(double startX, double startY, StudentWorld* sworld)
	: Goodie(IID_GAS_CAN_GOODIE, startX, startY, sworld)
{}

void GasCanGoodie::doSomething()
{
	if (!checkliving())
		return;

	double p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	double x = getX() - p_x, y = getY() - p_y;
	if (x*x + y * y <= 10 * 10)
	{
		getWorld()->increaseScore(50);
		resetStatus();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->getPenelope()->changeFlame(5);
	}
}

LandmineGoodie::LandmineGoodie(double startX, double startY, StudentWorld* sworld)
	: Goodie(IID_LANDMINE_GOODIE, startX, startY, sworld)
{}

void LandmineGoodie::doSomething()
{
	if (!checkliving())
		return;
	double p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	double x = getX() - p_x, y = getY() - p_y;
	if (x*x + y * y <= 10 * 10)
	{
		getWorld()->increaseScore(50);
		resetStatus();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->getPenelope()->changeMine(2);
	}
}

VaccineGoodie::VaccineGoodie(double startX, double startY, StudentWorld* sworld)
	: Goodie(IID_VACCINE_GOODIE, startX, startY, sworld)
{}

void VaccineGoodie::doSomething()
{
	if (!checkliving())
		return;
	double p_x = getWorld()->getPenelope()->getX(), p_y = getWorld()->getPenelope()->getY();
	double x = getX() - p_x, y = getY() - p_y;
	if (x*x + y * y <= 10 * 10)
	{
		getWorld()->increaseScore(50);
		resetStatus();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->getPenelope()->changeVaccine(1);
	}
}
//Landmine Implementation
Landmine::Landmine(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_LANDMINE, startX, startY, right, 1, sworld), m_safetytick(30), m_active(false)
{
	resetOverlap();
}

void Landmine::beginexplosion()
{
	resetStatus();
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
	//introduce flames

	if (getWorld()->canadd(getX(), getY()))
		getWorld()->addActor(new Flame(getX(), getY(), up, getWorld())); //current

	if (getWorld()->canadd(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT))
		getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up, getWorld())); //northeast
	
	if (getWorld()->canadd(getX() + SPRITE_WIDTH, getY()))
		getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY(), up, getWorld())); //east
	
	if (getWorld()->canadd(getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT))
			getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up, getWorld())); //southeast
	
	if (getWorld()->canadd(getX(), getY() - SPRITE_HEIGHT))
		getWorld()->addActor(new Flame(getX(), getY() - SPRITE_HEIGHT, up, getWorld())); //south
	
	if (getWorld()->canadd(getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT))
		getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT, up, getWorld())); //southwest
	
	if (getWorld()->canadd(getX() - SPRITE_WIDTH, getY()))
		getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY(), up, getWorld())); //west
	
	if (getWorld()->canadd(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT))
		getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT, up, getWorld())); //northwest
	
	if (getWorld()->canadd(getX(), getY() + SPRITE_HEIGHT))
		getWorld()->addActor(new Flame(getX(), getY() + SPRITE_HEIGHT, up, getWorld())); //north
	
	getWorld()->addActor(new Pit(getX(), getY(), getWorld()));
}

void Landmine::doSomething()
{
	if (!checkliving())
		return;

	if (m_safetytick > 0)
	{
		m_safetytick--;
		return;
	}
	else if (!isActive())
	{
		activate();
		return;
	}

	double xt = 0, yt = 0;
	if (isActive() && getWorld()->overlap(getX(), getY(), xt, yt)) //add overlap
		beginexplosion();

}

//Pit Implementation
Pit::Pit(double startX, double startY, StudentWorld* sworld)
	: Actor(IID_PIT, startX, startY, right, 0, sworld)
{
	resetOverlap();
}

void Pit::doSomething()
{
	double x1 = getX(), y1 = getY(); double x2 = 0, y2 = 0, x_p = getWorld()->getPenelope()->getX(), y_p = getWorld()->getPenelope()->getY();
	
	if (getWorld()->overlap(x1, y1, x2, y2))
		return;
}

//Projectile Implementation
Projectile::Projectile(int imageID, double startX, double startY, Direction dir, StudentWorld* sworld)
	: Actor(imageID, startX, startY, dir, 0, sworld), m_tickCount(0)
{
	resetOverlap();
}

//Flame Implementation
Flame::Flame(double startX, double startY, Direction dir, StudentWorld* sworld)
	: Projectile(IID_FLAME, startX, startY, dir, sworld)
{}

void Flame::doSomething()
{
	
	if (!checkliving())
		return;

	if (tickCount() >= 1) //0, runs through, 1, runs through, so 
		resetStatus();
	else increaseTick();
	
	double x1 = 0, y1 = 0;
	getWorld()->burn(getX(), getY(), x1, y1);
		
	//can now damage overlapping objects
}

//Vomit Implementation
Vomit::Vomit(double startX, double startY, Direction dir, StudentWorld* sworld)
	: Projectile(IID_VOMIT, startX, startY, dir, sworld)
{}

void Vomit::doSomething()
{
	if (!checkliving())
		return;

	if (tickCount() >= 1) //0, runs through, 1, runs through, so 
		resetStatus();
	else increaseTick();

	double x1 = 0, y1 = 0;
	getWorld()->infect(getX(), getY(), x1, y1);
}
