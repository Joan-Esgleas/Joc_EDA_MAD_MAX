#include <algorithm>
#include <iomanip>

#include "Board.hh"
#include "Action.hh"

#include <cctype>

using namespace std;


Board::Board (istream& is) {

  string s, version;

  // Game settings.
  is >> s >> version;
  my_assert(s == "mad_max_2020");
  my_assert(version == "v1");

  is >> s;
  my_assert(s == "secgame");
  is >> s;

  is >> s;
  my_assert(s == "seed");
  is >> s;
  
  is >> s >> nb_players;
  my_assert(s == "nb_players");
  my_assert(nb_players == 4);

  is >> s >> nb_rnds;
  my_assert(s == "nb_rnds");
  my_assert(nb_rnds >= 1);
  my_assert(nb_rnds == 300); // PDF description

  is >> s >> nb_rows;
  my_assert(s == "nb_rows");
  my_assert(nb_rows >= 10);
  my_assert(nb_rows >= 15 and nb_rows <= 20); // PDF description

  is >> s >> nb_uni_cols;
  my_assert(s == "nb_uni_cols");
  my_assert(nb_uni_cols >= 10);
  my_assert(nb_uni_cols >= 60 and nb_uni_cols <= 100); // PDF description

  is >> s >> nb_win_cols;
  my_assert(s == "nb_win_cols");
  my_assert(nb_win_cols >= 10);
  my_assert(nb_win_cols >= 30 and nb_win_cols <= 40); // PDF description
  my_assert(nb_win_cols <= nb_uni_cols);

  is >> s >> nb_cars_x_player;
  my_assert(s == "nb_cars_x_player");
  my_assert(nb_cars_x_player >= 1);

  is >> s >> nb_rnds_regen;
  my_assert(s == "nb_rnds_regen");
  my_assert(nb_rnds_regen >= 1);

  is >> s >> nb_miss_bonuses;
  my_assert(s == "nb_miss_bonuses");
  my_assert(nb_miss_bonuses >= 0);
  my_assert(nb_miss_bonuses >= 5 and nb_miss_bonuses <= 30); // PDF description
  
  is >> s >> nb_gas_bonuses;
  my_assert(s == "nb_gas_bonuses");
  my_assert(nb_gas_bonuses >= 0);
  my_assert(nb_gas_bonuses >= 5 and nb_gas_bonuses <= 15); // PDF description
  
  is >> s >> nb_water_bonuses;
  my_assert(s == "nb_water_bonuses");
  my_assert(nb_water_bonuses >= 0);
  my_assert(nb_water_bonuses >= 20 and nb_water_bonuses <= 150); // PDF description

  is >> s >> bonus_miss;
  my_assert(s == "bonus_miss");
  my_assert(bonus_miss >= 0);
  my_assert(bonus_miss == 5); // PDF description

  is >> s >> bonus_g;
  my_assert(s == "bonus_gas");
  my_assert(bonus_g >= 0);
  my_assert(bonus_g == 30); // PDF description

  is >> s >> water_pnts;
  my_assert(s == "water_pnts");
  my_assert(water_pnts >= 0);
  my_assert(water_pnts == 10); // PDF description

  is >> s >> kill_pnts;
  my_assert(s == "kill_pnts");
  my_assert(kill_pnts >= 0);
  my_assert(kill_pnts == 30); // PDF description

  is >> s >> init_gas;
  my_assert(s == "init_gas");
  my_assert(init_gas >= 0);
  my_assert(init_gas == 60); // PDF description

  names = vector<string>(nb_players);
  is >> s;
  my_assert(s == "names");
  for (Player_Id p = 0; p < nb_players; ++p) {
    is >> names[p];
    my_assert(names[p] != "");
  }


  // Game state.
  is >> s >> rnd;
  my_assert(s == "rnd");
  my_assert(rnd >= 0);
  my_assert(rnd < nb_rnds);

  // Reading auxiliary lines labelling columns (tens and units).
  is >> s;
  is >> s;

  cells = vector< vector<Cell> >(number_rows(), vector<Cell>(number_universe_columns()));

  // The map shows empty cells, tyres and bonuses.
  // Cars and missiles are not drawn on the map but listed later.

  for (int i = 0; i < number_rows(); ++i) {

    // Reading auxiliary row label.
    is >> s;

    for (int j = 0; j < number_universe_columns(); ++j) {
      cellPriv({i, j}).cid = cellPriv({i, j}).mid = -1;
      char c;
      is >> c;
      switch (c) {
      case '.': cellPriv({i, j}).type = EMPTY;         break;
      case 'X': cellPriv({i, j}).type = TYRE;          break;
      case 'W': cellPriv({i, j}).type = WATER_BONUS;   break;
      case 'G': cellPriv({i, j}).type = GAS_BONUS;     break;
      case 'M': cellPriv({i, j}).type = MISSILE_BONUS; break;
      default:  my_assert(false);
      }
    }
  }


  is >> s;
  my_assert(s == "cars");

  // Reading auxiliary line.
  is >> s;  my_assert(s == "cid");
  is >> s;  my_assert(s == "row");
  is >> s;  my_assert(s == "column");
  is >> s;  my_assert(s == "nb_miss");
  is >> s;  my_assert(s == "gas");
  is >> s;  my_assert(s == "alive");
  is >> s;  my_assert(s == "time");

  int n_all_cars = number_cars();
  cars = vector<Car>(n_all_cars);
  for (int k = 0; k < n_all_cars; ++k) {

    int cid, i, j, m, g, t;
    char a;
    is >> cid >> i >> j >> m >> g >> a >> t;

    j = normalize(j);
    
    my_assert(cid == k);
    my_assert(within_universe({i, j}));
    my_assert(m >= 0);
    my_assert(g >= 0);
    my_assert(a == 'y' or a == 'n');
    my_assert(t >= 0);
    my_assert(a == 'n' or t == 0);
    my_assert(a == 'n' or cellPriv({i, j}).type == EMPTY);
    my_assert(cellPriv({i, j}).type != TYRE);

    cars[k].cid     = cid;
    cars[k].pos     = {i, j};
    cars[k].nb_miss = m;
    cars[k].gas     = g;
    cars[k].alive   = (a == 'y');
    cars[k].time    = t;
    if (cars[k].alive)
      cellPriv({i, j}) = {CAR, cid, -1};
  }


  is >> s;
  my_assert(s == "missiles");
  int n_missiles;
  is >> n_missiles;

  // Reading auxiliary line.
  is >> s;  my_assert(s == "mid");
  is >> s;  my_assert(s == "cid");
  is >> s;  my_assert(s == "row");
  is >> s;  my_assert(s == "column");

  for (int k = 0; k < n_missiles; ++k) {

    int mid, cid, i, j;
    is >> mid >> cid >> i >> j;

    j = normalize(j);
    
    my_assert(car_ok(cid));
    my_assert(within_universe({i, j}));
    my_assert(cellPriv({i, j}).type == EMPTY);

    miss[mid] = {mid, cid, {i, j}};
    cellPriv({i, j}) = {MISSILE, -1, mid};
  }


  is >> s;
  my_assert(s == "players");

  // Reading auxiliary line.
  is >> s;  my_assert(s == "pid");
  is >> s;  my_assert(s == "score");
  is >> s;  my_assert(s == "status");

  scores = vector<int>   (number_players());
  statuses = vector<double>(number_players());

  for (int k = 0; k < number_players(); ++k) {

    int id, sc;
    double st;
    is >> id >> sc >> st;

    my_assert(id == k);
    my_assert(sc >= 0);
    my_assert(st >=-1);

    scores[k] = sc;
    statuses[k] = st;
  }

  assert(ok());
}

extern int seed;

void Board::print_settings(ostream& os, bool secgame) const {
  os << "mad_max_2020"     << "\t" << "v1"                         << endl;
  os << "secgame"          << "\t" << secgame                      << endl;
  os << "seed"             << "\t" << seed                         << endl;  
  os << "nb_players"       << "\t" << number_players()             << endl;
  os << "nb_rnds"          << "\t" << number_rounds()              << endl;
  os << "nb_rows"          << "\t" << number_rows()                << endl;
  os << "nb_uni_cols"      << "\t" << number_universe_columns()    << endl;
  os << "nb_win_cols"      << "\t" << number_window_columns()      << endl;
  os << "nb_cars_x_player" << "\t" << number_cars_per_player()     << endl;
  os << "nb_rnds_regen"    << "\t" << number_rounds_to_regenerate()<< endl;
  os << "nb_miss_bonuses"  << "\t" << number_missile_bonuses()     << endl;
  os << "nb_gas_bonuses"   << "\t" << number_gas_bonuses()         << endl;
  os << "nb_water_bonuses" << "\t" << number_water_bonuses()       << endl;
  os << "bonus_miss"       << "\t" << bonus_missiles()             << endl;
  os << "bonus_gas"        << "\t" << bonus_gas()                  << endl;  
  os << "water_pnts"       << "\t" << water_points()               << endl;
  os << "kill_pnts"        << "\t" << kill_points()                << endl;
  os << "init_gas"         << "\t" << initial_gas()                << endl;

  os << "names";
  for (Player_Id p = 0; p < number_players(); ++p) os << " " << name(p);
  os << endl << endl;
}


void Board::print_state(ostream& os) const {

  os << endl;
  os << "rnd " << round() << endl << endl;


  // Printing auxiliary lines labelling columns.
  os << "   ";
  for (int j = 0; j < number_universe_columns(); ++j) os << (j / 10);
  os << endl;

  os << "   ";
  for (int j = 0; j < number_universe_columns(); ++j) os << (j % 10);
  os << endl;
  os << endl;

  for (int i = 0; i < number_rows(); ++i) {
    os << (i / 10) << (i % 10) << " "; // Printing column labels.
    for (int j = 0; j < number_universe_columns(); ++j) {
      CType c = cell({i, j}).type;
      switch (c) {
      case TYRE:       os << 'X'; break;
      case WATER_BONUS:    os << 'W'; break;
      case MISSILE_BONUS:  os << 'M'; break;
      case GAS_BONUS:      os << 'G'; break;
      default:             os << '.'; break;
      };
    }
    os << endl;
  }
  os << endl << endl;



  os << "cars" << endl;
  os << "cid"      << "\t"
     << "row"     << "\t"
     << "column"  << "\t"
     << "nb_miss" << "\t"
     << "gas"     << "\t"
     << "alive"   << "\t"
     << "time"    << endl;

  for (const Car& c: cars)
    os << c.cid                    << "\t"
       << first( c.pos)            << "\t"
       << normalize(second(c.pos)) << "\t"
       << c.nb_miss                << "\t"
       << c.gas                    << "\t"
       << (c.alive ? 'y' : 'n')    << "\t"
       << c.time                   << endl;
  os << endl;



  os << "missiles"  << endl;
  os << miss.size() << endl;
  os << "mid"       << "\t"
     << "cid"       << "\t"
     << "row"       << "\t"
     << "column"    << endl;

  for (const auto& x: miss) {
    const Missile& m = x.second;
    os << m.mid                    << "\t"
       << m.cid                    << "\t"
       << first( m.pos)            << "\t"
       << normalize(second(m.pos)) << endl;
  }
  os << endl;



  os << "players" << endl;
  os << "pid"     << "\t"
     << "score"   << "\t"
     << "status"  << endl;

  for (Player_Id p = 0; p < number_players(); ++p)
    os << p         << "\t"
       << score(p)  << "\t"
       << status(p) << endl;

  os << endl;
}


Board Board::next (const vector<Action>& asked, vector<Action>& done) {

  assert(ok());


  // b will be the new board we shall return.
  Board b = *this;

  // Increment the round.
  ++b.rnd;

  // Move missiles.
  queue<Missile> q;
  for (const auto& x : b.miss)
    q.push(x.second);

  // Missiles should move continuously and at the same time,
  // but we move them discretely. We process them in a way
  // that they never crash with each other.
  while (not q.empty()) {
    Missile m = q.front();
    q.pop();
    if (b.cellPriv(m.pos + DEFAULT).type != MISSILE and
        b.cellPriv(m.pos +    FAST).type != MISSILE)
      b.move_missile(b.miss[m.mid]);
    else q.push(m);
  }

  // Sort players randomly.
  vector<int> random_players = random_permutation(number_players());

  // Handle commands of each player.
  vector<bool> commanded(number_cars(), false);
  for (int k = 0; k < number_players(); ++k) {
    Player_Id p = random_players[k];
    for (const Instruction& ins : asked[p].instructs) {
      if (not(
	      car_ok(ins.cid)                         and
	      player_of(ins.cid) == p                 and
	      (ins.type == MOVE or ins.type == SHOOT) and
	      not commanded[ins.cid] and
	      dir_ok(ins.dir)
	      ))
	continue;

      if (b.cars[ins.cid].alive and b.apply(ins)) {
	commanded[ins.cid] = true;
	done[p].cids.insert(ins.cid);
	done[p].instructs.push_back(ins);
      }
    }
    for (Car_Id c = begin(p); c != end(p); ++c)
      if (not commanded[c] and b.cars[c].alive)
        b.apply({c, MOVE, DEFAULT});
  }

  // Decrease gas
  for (Car& c: b.cars) {
    if (c.alive) {
      --c.gas;
      if (c.gas == 0)
  	b.kill_car(c);
    }
  }
  
  // Regenerate cars.
  for (Car& c : b.cars)
    if (not c.alive) {
      if (c.time == 0) b.regenerate_car(c);
      else --c.time;
    }

  // Place new bonuses.
  int nb_miss_bonuses_to_place  = nb_miss_bonuses;
  int nb_water_bonuses_to_place = nb_water_bonuses;
  int nb_gas_bonuses_to_place   = nb_gas_bonuses;
  for (int i = 0; i < number_rows(); ++i)
    for (int j = 0; j < number_universe_columns(); ++j) {
      if      (b.cellPriv({i, j}).type == MISSILE_BONUS) --nb_miss_bonuses_to_place;
      else if (b.cellPriv({i, j}).type == WATER_BONUS  ) --nb_water_bonuses_to_place;
      else if (b.cellPriv({i, j}).type == GAS_BONUS    ) --nb_gas_bonuses_to_place;
    }

  while (nb_miss_bonuses_to_place  > 0 and b.place_new_bonus(MISSILE_BONUS))
    --nb_miss_bonuses_to_place;

  while (nb_water_bonuses_to_place > 0 and b.place_new_bonus(WATER_BONUS))
    --nb_water_bonuses_to_place;

  while (nb_gas_bonuses_to_place > 0 and b.place_new_bonus(GAS_BONUS))
    --nb_gas_bonuses_to_place;


  assert(b.ok());
  return b;
}


bool Board::move_car(Car& k, Dir dir) {

  my_assert(k.alive);
  my_assert(cellPriv(k.pos).type == CAR);
  my_assert(cellPriv(k.pos).cid  == k.cid   );
  my_assert(cellPriv(k.pos).mid  == -1      );

  if (not within_window(k.pos + dir, round())) return false;

  if (dir == SLOW) return true;

  const auto& all = dir2all.at(dir);
  my_assert(not all.empty());
  for (Dir d : all) {
    my_assert(d != Dir(0, 0));
    Cell& c = cellPriv(k.pos + d);
    if (c.type == WATER_BONUS) {
      scores[player_of(k.cid)] += water_points();
      c.type = EMPTY;
    }
    if (c.type == GAS_BONUS) {
      k.gas += bonus_gas();
      c.type = EMPTY;
    }    
    else if (c.type == MISSILE_BONUS) {
      k.nb_miss += bonus_missiles();
      c.type = EMPTY;
    }
    else if (c.type == TYRE) {

      cellPriv(k.pos + d) = {CAR,  k.cid, -1};
      cellPriv(k.pos    ) = {EMPTY,   -1, -1};
      k.pos += d;
      k.pos.imag(normalize(k.pos.imag()));
      kill_car(k);

      c.type = EMPTY;
      return true;
    }
    else if (c.type == CAR) {
      Car& t = cars[c.cid];
      my_assert(k.cid != t.cid);

      cellPriv(k.pos + d) = {CAR,   k.cid, -1};
      cellPriv(k.pos    ) = {EMPTY,    -1, -1};
      k.pos += d;
      k.pos.imag(normalize(k.pos.imag()));
      kill_car(k);
      kill_car(t);
      return true;
    }
    else if (c.type == MISSILE) {
      my_assert(miss.contains(c.mid));
      Missile m = miss[c.mid];
      if (player_of(m.cid) != player_of(k.cid))
        scores[player_of(m.cid)] += kill_points();

      cellPriv(k.pos + d) = {CAR,   k.cid, -1};
      cellPriv(k.pos    ) = {EMPTY,    -1, -1};
      k.pos += d;
      k.pos.imag(normalize(k.pos.imag()));
      kill_car(k);

      kill_missile(m);
      return true;
    }
  }
  cellPriv(k.pos + dir) = {CAR,   k.cid, -1};
  cellPriv(k.pos      ) = {EMPTY,    -1, -1};

  k.pos += dir;
  k.pos.imag(normalize(k.pos.imag()));
  
  return true;
}


bool Board::shoot(Car& c) {

  my_assert(c.alive);
  my_assert(cellPriv(c.pos).type == CAR);
  my_assert(cellPriv(c.pos).cid  == c.cid   );
  my_assert(cellPriv(c.pos).mid  == -1      );

  if (c.nb_miss > 0) {
    --c.nb_miss;
    Missile_Id mid = new_missile_identifier();
    my_assert(not miss.contains(mid));
    miss[mid] = {mid, c.cid, c.pos};
    move_missile(miss[mid]);
    cellPriv(c.pos) = {CAR, c.cid, -1 }; // Correcting.
    return true;
  }
  else return false;
}


bool Board::move_missile(Missile& m) {

  const auto& all = dir2all.at(FAST);
  my_assert(not all.empty());
  for (Dir d : all) {
    my_assert(d != Dir(0, 0));
    Cell& c = cellPriv(m.pos + d);
    my_assert(c.type != MISSILE);
    if (c.type == WATER_BONUS   or
	c.type == GAS_BONUS     or
        c.type == MISSILE_BONUS or
        c.type == TYRE) {
      kill_missile(m);
      c.type = EMPTY;
      return true;
    }
    else if (c.type == CAR) {
      Car& k = cars[c.cid];
      if (player_of(m.cid) != player_of(k.cid))
        scores[player_of(m.cid)] += kill_points();
      kill_car(k);
      kill_missile(m);
      return true;
    }
  }
  cellPriv(m.pos + FAST) = {MISSILE, -1, m.mid};
  cellPriv(m.pos)        = {EMPTY,   -1, -1   };

  m.pos += FAST;
  m.pos.imag(normalize(m.pos.imag()));
  
  return true;
}


bool Board::place_new_bonus(CType bonus) {
  my_assert(bonus == WATER_BONUS or bonus == MISSILE_BONUS or bonus == GAS_BONUS);
  Pos p;
  int r = round();
  int f = number_rows();
  int cu = number_universe_columns();
  int cw = number_window_columns();
  if (random_empty_position({0, r+cw}, {f-1, r+cu-1}, 5, {MISSILE, CAR}, p)) {
    cellPriv(p) = {bonus, -1, -1};
    return true;
  }
  else return false;
}


bool Board::regenerate_car(Car& k) {
  Pos p;
  int r = round();
  int f = number_rows();
  int c = number_window_columns();
  if (random_empty_position({0, r}, {f-1, r+c-1}, 5, {MISSILE, CAR, TYRE}, p)) {
    k.pos = p;
    k.alive = true;
    k.time = 0;
    k.gas = initial_gas();
    cellPriv(p) = {CAR, k.cid, -1};
    return true;
  }
  else return false;
}


bool Board::random_empty_position(Pos up_left, Pos down_right, int n, const set<CType>& s, Pos& p) {
  const int MAX_ATTEMPS = 20;
  for (int k = 0; k < MAX_ATTEMPS; ++k) {
    int i = randomize( first(up_left),  first(down_right));
    int j = randomize(second(up_left), second(down_right));
    p = {i, j % number_universe_columns() };
    my_assert(within_universe(p));
    if (cellPriv(p).type == EMPTY and is_free(p, n, s))
      return true;
  }
  return false;
}


bool Board::is_free(Pos p, int n, const set<CType>& s) {
  my_assert(n % 2 == 1);
  for (int i = first(p) - n/2; i <= first(p) + n/2; ++i)
    for (int j = second(p) - n/2; j <= second(p) + n/2; ++j) {
      Pos q = {i, j};
      if (within_universe(q)) {
        CType t = cellPriv(q).type;
        if (s.contains(t)) return false;
      }
    }
  return true;
}


void Board::kill_car(Car& c) {
  my_assert(c.alive);
  cellPriv(c.pos) = {EMPTY, -1, -1};
  c.alive = false;
  c.time = number_rounds_to_regenerate();
}


void Board::kill_missile(Missile& m) {
  cellPriv(m.pos) = {EMPTY, -1, -1};
  miss.erase(m.mid);
}


bool Board::ok() const {

  if (not(nb_players == 4)) {
    cerr << "debug: problems with parameter setting (0)." << endl;
    return false;
  }
  if (not(nb_rnds >= 1)) {
    cerr << "debug: problems with parameter setting (1)." << endl;
    return false;
  }
  if (not(nb_rows >= 10)) {
    cerr << "debug: problems with parameter setting (2)." << endl;
    return false;
  }
  if (not(nb_uni_cols >= 10)) {
    cerr << "debug: problems with parameter setting (3)." << endl;
    return false;
  }
  if (not(nb_win_cols >= 10)) {
    cerr << "debug: problems with parameter setting (4)." << endl;
    return false;
  }
  if (not(nb_win_cols <= nb_uni_cols)) {
    cerr << "debug: problems with parameter setting (5)." << endl;
    return false;
  }
  if (not(nb_cars_x_player >= 1)) {
    cerr << "debug: problems with parameter setting (6)." << endl;
    return false;
  }
  if (not(nb_rnds_regen >= 1)) {
    cerr << "debug: problems with parameter setting (7)." << endl;
    return false;
  }
  if (not(nb_miss_bonuses >= 0)) {
    cerr << "debug: problems with parameter setting (8)." << endl;
    return false;
  }
  if (not(nb_gas_bonuses >= 0)) {
    cerr << "debug: problems with parameter setting (9)." << endl;
    return false;
  }  
  if (not(nb_water_bonuses >= 0)) {
    cerr << "debug: problems with parameter setting (10)." << endl;
    return false;
  }
  if (not(bonus_miss >= 0)) {
    cerr << "debug: problems with parameter setting (11)." << endl;
    return false;
  }
  if (not(bonus_g >= 0)) {
    cerr << "debug: problems with parameter setting (12)." << endl;
    return false;
  }  
  if (not(water_pnts >= 0)) {
    cerr << "debug: problems with parameter setting (13)." << endl;
    return false;
  }
  if (not(kill_pnts >= 0)) {
    cerr << "debug: problems with parameter setting (14)." << endl;
    return false;
  }
  if (not(init_gas >= 0)){
    cerr << "debug: problems with parameter setting (15)." << endl;
    return false;
  }
  if (not(rnd >= 0)) {
    cerr << "debug: problems with number of rounds (1)." << endl;
    return false;
  }
  if (not(rnd <= nb_rnds)) {
    cerr << "debug: problems with number of rounds (2)." << endl;
    return false;
  }

  if (int(cells.size()) != number_rows()) {
    cerr << "debug: problems with row dimension of the board." << endl;
    return false;
  }
  if (int(cells[0].size()) != number_universe_columns()) {
    cerr << "debug: problems with column dimension of the board." << endl;
    return false;
  }

  if (int(names.size()) != number_players()) {
    cerr << "debug: problems with number of players and names." << endl;
    return false;
  }
  for (const string& s : names)
    if (s == "") {
      cerr << "debug: names cannot be empty." << endl;
      return false;
    }

  int nb_miss_bonuses_on_board = 0;
  int nb_point_bonuses_on_board = 0;
  int nb_gas_bonuses_on_board = 0;
  vector<int> nb_cars_on_board(number_players());
  for (int i = 0; i < number_rows(); ++i) {
    for (int j = 0; j < number_universe_columns(); ++j) {

      int cid = cells[i][j].cid;
      if (cid < -1 or cid > number_cars()) {
        cerr << "debug: problem with car identifiers in cells." << endl;
        return false;
      }
      if (cid == -1 and cells[i][j].type == CAR) {
        cerr << "debug: cell should have a valid car identifier." << endl;
        return false;
      }
      if (cid != -1 and cells[i][j].type != CAR) {
        cerr << "debug: cell should have an invalid car identifier." << endl;
        return false;
      }
      
      int mid = cells[i][j].mid;
      if (mid < -1) {
        cerr << "debug: problem with missile identifiers in cells." << endl;
        return false;
      }
      if (mid == -1 and cells[i][j].type == MISSILE) {
        cerr << "debug: cell should have a valid missile identifier." << endl;
        return false;
      }
      if (mid != -1 and cells[i][j].type != MISSILE) {
        cerr << "debug: cell should have an invalid missile identifier." << endl;
        return false;
      }
      if (cells[i][j].type == MISSILE and not miss.contains(mid)) {
        cerr << "debug: missile map is corrupted." << endl;
        return false;
      }
      
      if      (cells[i][j].type == MISSILE_BONUS) ++nb_miss_bonuses_on_board;
      else if (cells[i][j].type == WATER_BONUS)   ++nb_point_bonuses_on_board;
      else if (cells[i][j].type == GAS_BONUS)     ++nb_gas_bonuses_on_board;
      else if (cells[i][j].type == CAR)           ++nb_cars_on_board[player_of(cid)];
    }
  }
  if (nb_miss_bonuses_on_board > nb_miss_bonuses) {
    cerr << "debug: problems with the number of missile bonuses" << endl;
    return false;
  }
  if (nb_point_bonuses_on_board > nb_water_bonuses) {
    cerr << "debug: problems with the number of water bonuses" << endl;
    return false;
  }
  if (nb_gas_bonuses_on_board > nb_gas_bonuses) {
    cerr << "debug: problems with the number of gas bonuses" << endl;
    return false;
  }
  for (int n : nb_cars_on_board)
    if (n > number_cars_per_player()) {
      cerr << "debug: problems with the number of cars" << endl;
      return false;
    }

  if (int(cars.size()) != number_cars()) {
      cerr << "debug: problems with the size of cars" << endl;
      return false;
  }

  vector<int> alive(number_players());
  for (int k = 0; k < int(cars.size()); ++k) {
    const Car& c = cars[k];
    if (k != c.cid) {
      cerr << "debug: cars is in an inconsistent state" << endl;
      return false;
    }
    Player_Id p = player_of(c.cid);
    if (c.alive) {
      ++alive[p];
      if (cell(c.pos).type != CAR) {
        cerr << "debug: problems with the correspondence between cells and cars" << endl;
        return false;
      }
    }
    if (c.nb_miss < 0) {
      cerr << "debug: number of available missiles cannot be negative" << endl;
      return false;
    }
    if (c.gas < 0) {
      cerr << "debug: remaining gas cannot be negative" << endl;
      return false;
    }
    if (c.time < 0) {
      cerr << "debug: time cannot be negative" << endl;
      return false;
    }
    if (c.alive and c.time > 0) {
      cerr << "debug: found inconsistency between alive and time" << endl;
      return false;
    }
  }
  for (Player_Id p = 0; p < number_players(); ++p)
    if (nb_cars_on_board[p] != alive[p]) {
      cerr << "debug: problems with the number of alive cars" << endl;
      return false;
    }

  for (const auto& x : miss) {
    const Missile& m = x.second;
    if (x.first != m.mid) {
      cerr << "debug: miss is in an inconsistent state" << endl;
      return false;
    }
    if (not car_ok(m.cid)) {
      cerr << "debug: missile has an invalid car identifier" << endl;
      return false;
    }
    if (cell(m.pos).type != MISSILE) {
      cerr << "debug: problems with the correspondence between cells and missiles" << endl;
      return false;
    }
  }

  if (int(scores.size()) != number_players()) {
    cerr << "debug: problems with number of players and scores." << endl;
    return false;
  }
  for (int s : scores)
    if (s < 0) {
      cerr << "debug: there cannot be negative scores." << endl;
      return false;
    }

  if (int(statuses.size()) != number_players()) {
    cerr << "debug: problems with number of players and status." << endl;
    return false;
  }
  for (int s : statuses)
    if (s != -1 and not(0 <= s and s <= 1)) {
      cerr << "debug: problems with the status." << endl;
      return false;
    }

  return true;
}
