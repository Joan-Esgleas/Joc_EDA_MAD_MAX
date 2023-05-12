#ifndef Action_hh
#define Action_hh

#include "Utils.hh"

using namespace std;

struct Instruction;


/**
 * Class that stores all instructions requested by a player in a round.
 */

class Action {

public:

  /**
   * Creates an empty action.
   */
  Action();

  
  /**
   * Moves the specified car in a given direction.
   * The movement should be such that the resulting position is within the window,
   * the car should be alive,
   * the car identifier and the direction should be ok,
   * the car should belong to the commanding player,
   * and no other instruction should have been commanded to the car before in the same round.
   * Otherwise the instruction is ignored.
   */
  void move(Car_Id c, Dir d);

  
  /**
   * Makes the specified car shoot a missile.
   * There should be at least one missile in the car ammunition,
   * the car should be alive,
   * the car identifier should be ok,      
   * the car should belong to the commanding player,
   * and no other instruction should have been commanded to the car before in the same round.
   * Otherwise the instruction is ignored.
   */
  void shoot(Car_Id c);

  
  /****************************************************/
  /***************** STOP READING HERE ****************/
  /****************************************************/

  
private:

  friend class Game;
  friend class SecGame;
  friend class Board;

  set<int>            cids;
  vector<Instruction> instructs;

  /**
   * Constructor reading one action from a stream.
   */
  Action(istream& is);

  /**
   * Print the action to a stream.
   */
  void print(ostream& os) const;
  
};


/**
 * Enum type for all possible instructions.
 */

enum IType {
  UNDEFINED, MOVE, SHOOT
};


/**
 * Conversion from IType to char.
 */
inline char i2c(IType a) {
  switch (a) {
  case MOVE     : return 'm';
  case SHOOT    : return 's';
  default       : return 'u';
  }
}


/**
 * Conversion from char to IType.
 */
inline IType c2i (char c) {
  switch (c) {
  case 'm':   return MOVE;
  case 's':   return SHOOT;
  default :   return UNDEFINED;
  }
}


/**
 * The description of an instruction stores
 * the identifier of the car that is being commanded,
 * the type of the instruction (UNDEFINED | MOVE | SHOOT),
 * and the movement direction.
 */

struct Instruction {
  Car_Id      cid;
  IType       type;
  Dir          dir;
};


inline Action::Action() {
}


inline void Action::move(Car_Id c, Dir d) {
  if (cids.contains(c))
    cerr << "warning: instruction alread requested for car " << c << endl;
  else {
    cids.insert(c);
    instructs.push_back({c, MOVE, d});
  }
}


inline void Action::shoot(Car_Id c) {
  if (cids.contains(c))
    cerr << "warning: instruction alread requested for car " << c << endl;
  else {
    cids.insert(c);
    instructs.push_back({c, SHOOT, DEFAULT});
  }
}


#endif // Action_hh
