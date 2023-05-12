#ifndef Player_hh
#define Player_hh

#include "Utils.hh"
#include "Board.hh"
#include "Action.hh"
#include "Registry.hh"

using namespace std;


/***
 * Abstract base class for players.
 * *
 * This class uses multiple inheritance from Board
 * and Action, so that their public operations can been used
 * from within Player.
 *
 * In order to create new players, inherit from this class
 * and register them. See the examples AINull.cc and AIDemo.cc.
 */

class Player : public Board, public Action {

public:

  /**
   * Return the number of my player.
   */
  int me ();


  /****************************************************/
  /***************** STOP READING HERE ****************/
  /****************************************************/

private:  
  
  friend class Game;
  friend class SecGame;

  int me_;

  inline void reset (int me, const Board& board, const Action& action) {
    me_ = me;
    *(Board*)this = board;
    *(Action*)this = action;
  }


public:

  void move(Car_Id c, Dir d) {
    if (not car_ok(c))
      cerr << "warning: invalid car identifier" << c << endl;
    else if (not dir_ok(d))
      cerr << "warning: invalid direction " << d << " for car " << c << endl;
    else if (player_of(c) != me())  {
      cerr << "warning: car does not belong to commanding player" << endl;
    }
    else {
      Action::move(c, d);
    }
  }

  void shoot(Car_Id c) {
    if (not car_ok(c))
      cerr << "warning: invalid car identifier" << c << endl;
    else if (player_of(c) != me())  {
      cerr << "warning: car does not belong to commanding player" << endl;
    }
    else {
      Action::shoot(c);
    }
  }

  /**
   * My play intelligence.
   * Will have to be overwritten, thus declared virtual.
   */
  virtual void play () {
  };

};

inline int Player::me () {
  return me_;
}

#endif
