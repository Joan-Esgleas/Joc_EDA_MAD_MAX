
#include "Player.hh"

using namespace std;


/**
 * Write the name of your player and save this file
 * as AI<name>.cc
 */
#define PLAYER_NAME Demo





struct PLAYER_NAME : public Player {


    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }


    /**
     * Attributes for your player can be defined here.
     */

  vector<Dir> all_dirs;

  // If the cell following some direction is within
  // the window in the next round and has a bonus, move there.
  // Returns whether a successful movement could be found.
  bool move_to_bonus(const Car& c) {
    for (Dir d : all_dirs)
      if (within_window(c.pos + d, round()+1)) {
          CType t = cell(c.pos + d).type;
          if (t == MISSILE_BONUS or t == WATER_BONUS or t == GAS_BONUS) {
            move(c.cid, d);
            return true;
          }
      }
    return false;
  }

  bool up(Dir d)   { return d == SLOW_UP   or d == UP   or d == FAST_UP;   }
  bool down(Dir d) { return d == SLOW_DOWN or d == DOWN or d == FAST_DOWN; }


    /**
     * Play method.
     *
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     */
  virtual void play () {

    if (round() == 0) {
      // Initialize a vector with all allowed directions.
      all_dirs = {     UP, DEFAULT,      DOWN,
                  SLOW_UP,    SLOW, SLOW_DOWN,
                  FAST_UP,    FAST, FAST_DOWN};
    }

    // For each of my cars
    for (Car_Id cid = begin(me()); cid != end(me()); ++cid) {

      Car c = car(cid);
      
      if (c.alive) { // if the car is alive and

        Pos p = c.pos;

        // has missiles and there is an enemy in the cell ahead, shoot
        Cell ahead = cell(p + DEFAULT);
        if (c.nb_miss > 0 and ahead.type == CAR) {
          Player_Id p = player_of(ahead.cid);
          if (p != me()) shoot(cid);
        }

        // otherwise
        else {
          // if a bonus can be reached move there
          bool success = move_to_bonus(c);

          // otherwise take a random direction if current row is not 0
          // nor number_rows()-1.
          if (not success) {
            int row = first(c.pos);
            if (row != 0 and row != number_rows()-1) {
              Dir d = all_dirs[randomize(0, all_dirs.size()-1)];
              if (within_window(c.pos + d, round()+1)) move(cid, d);
            }
          }
        }
      }
    }
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
