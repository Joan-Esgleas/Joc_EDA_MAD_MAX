#include "Player.hh"
#include<queue>
#include<algorithm>
#include<cmath>
#include<list>

using namespace std;


/**
 * Write the name of your player and save this file
 * as AI<name>.cc
 */
#define PLAYER_NAME Maikel_v4

struct PLAYER_NAME : public Player {


    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }
    
    #define INF 999999;

    // A* algorithm stuff
    struct Nodo {
        Pos posicion;
        Dir direccion;
        int costo_g;
        int costo_h;
    Nodo* padre;

    Nodo(Pos pos, Dir dir, int g, int h, Nodo* padre)
        : posicion(pos), direccion(dir), costo_g(g), costo_h(h), padre(padre) {}

    int costo_total() const {
        return costo_g + costo_h;
    }
    };
    struct ComparadorNodos {
        bool operator()(const Nodo* nodo1, const Nodo* nodo2) const {
            return nodo1->costo_total() > nodo2->costo_total();
        }
    };
    bool Posicion_valida(Pos p, Car_Id c_id) {
        if (not within_universe(p)) return false;
        vector<Dir> all_dirs = {UP,DEFAULT,DOWN, SLOW_UP,SLOW, SLOW_DOWN};

        bool other_car = false;

        for(Dir d: all_dirs) {
            if(within_universe(p-d)) {
                other_car = cell(p-d).type == CAR and cell(p-d).cid != c_id;
            }
            if(other_car) break;
        }

        if(other_car or cell(p).type == TYRE or cell(p+Dir(0,-1)).type == MISSILE or cell(p+Dir(0,-2)).type == MISSILE or not within_window(p,round())) return false;
        return true;
    }   
    list<Dir> Camino_rapido(Pos ini, Pos fin, Car_Id c_id) {
        list<Dir> camino;
        if (!Posicion_valida(ini,c_id) || !Posicion_valida(fin,c_id)) {
            //cerr << "Posiciones no válidas." << endl;
            return camino;
        }

        vector<vector<bool>> visitado(number_universe_columns(), vector<bool>(number_universe_columns(), false));
        priority_queue<Nodo*, vector<Nodo*>, ComparadorNodos> abiertos;

        abiertos.push(new Nodo(ini, DEFAULT, 0, abs(fin - ini), nullptr));

        while (!abiertos.empty()) {
            Nodo* actual = abiertos.top();
            abiertos.pop();

            if (actual->posicion == fin) {
                // Construir el camino desde el nodo final hasta el inicio
                Nodo* nodo = actual;
                while (nodo->padre != nullptr) {
                    camino.push_front(nodo->direccion);
                    nodo = nodo->padre;
                }

                // Liberar la memoria de los nodos creados
                while (!abiertos.empty()) {
                    delete abiertos.top();
                    abiertos.pop();
                }

                return camino;
            }

        visitado[actual->posicion.real()][actual->posicion.imag()] = true;

            for (Dir dir : {UP, DEFAULT, DOWN, SLOW_UP, SLOW, SLOW_DOWN, FAST_UP, FAST, FAST_DOWN}) {
                Pos nueva_posicion = actual->posicion + dir;
                Pos pos_intermedia_1 = actual->posicion + dir;
                Pos pos_intermedia_2 = actual->posicion + dir;
                Pos pos_intermedia_3 = actual->posicion + dir;
                Pos pos_intermedia_4 = actual->posicion + dir;
                if (dir == UP or dir == DOWN) {
                    if (dir == UP) {
                        pos_intermedia_1 = actual->posicion + Dir(-1,0);
                        pos_intermedia_2 = actual->posicion + Dir(0,1);
                    }
                    else if (dir == DOWN) {
                        pos_intermedia_1 = actual->posicion + Dir(1,0);
                        pos_intermedia_2 = actual->posicion + Dir(0,1);
                    }
                }
                else if (dir == FAST) {
                    pos_intermedia_1 = actual->posicion + Dir(0,1);
                    pos_intermedia_2 = actual->posicion + Dir(0,2);
                }
                if (dir == FAST_UP or dir == FAST_DOWN) {
                    if (dir == FAST_UP) {
                        pos_intermedia_1 = actual->posicion + Dir(-1,0);
                        pos_intermedia_2 = actual->posicion + Dir(0,1);
                        pos_intermedia_3 = actual->posicion + Dir(0,2);
                        pos_intermedia_4 = actual->posicion + Dir(-1,1);
                    }
                    else if (dir == FAST_DOWN) {
                        pos_intermedia_1 = actual->posicion + Dir(1,0);
                        pos_intermedia_2 = actual->posicion + Dir(0,1);
                        pos_intermedia_3 = actual->posicion + Dir(0,2);
                        pos_intermedia_4 = actual->posicion + Dir(1,1);
                    }
                }
                bool P_v = Posicion_valida(nueva_posicion,c_id) && Posicion_valida(pos_intermedia_1,c_id) && Posicion_valida(pos_intermedia_2,c_id) && Posicion_valida(pos_intermedia_3,c_id) && Posicion_valida(pos_intermedia_4,c_id);
                if (P_v && !visitado[nueva_posicion.real()][nueva_posicion.imag()]) {
                    int costo_g = actual->costo_g + 1;
                    int costo_h = abs(fin - nueva_posicion);
                    abiertos.push(new Nodo(nueva_posicion, dir, costo_g, costo_h, actual));
                    visitado[nueva_posicion.real()][nueva_posicion.imag()] = true;
                }
            }
        }

        //cerr << "No se encontró un camino válido." << endl;
        return camino;
    }

    // Utility
    typedef pair<int,Pos> dist_and_p;
    struct cmp_dist_p {
        bool operator()(const dist_and_p& a, const dist_and_p& b) const {
        // Comparación basada en la magnitud del número complejo
        return a.first > b.first;
        }
    };
    struct cmp_dist_c {
        bool operator()(const pair<int,Car>& a, const pair<int,Car>& b) const {
        // Comparación basada en la magnitud del número complejo
        return a.first > b.first;
        }
    };
    typedef priority_queue<dist_and_p, vector<dist_and_p>, cmp_dist_p> priority_queue_dist_and_p;
    typedef priority_queue<pair<int,Car>, vector<pair<int,Car>>, cmp_dist_c> priority_queue_dist_and_c;
    typedef struct Coche {
        Car datos;
        Pos objetivo;
        priority_queue_dist_and_p Posiciones_water_bonus;
        priority_queue_dist_and_p Posiciones_gas_bonus;
        priority_queue_dist_and_p Posiciones_missil_bonus;
        priority_queue_dist_and_c Posiciones_coches_enemigos;
    }Coche;
    vector<Coche> c;
    void add_elem(priority_queue_dist_and_p& v, dist_and_p elem) {
        v.push(elem);
    }
    void add_elem(priority_queue_dist_and_c& v, pair<int,Car> elem) {
        v.push(elem);
    }


    int Distancia(Pos ini, Pos fin) {
        int x_diff = abs(first(fin) - first(ini));
        int y_diff = abs(second(fin) - second(ini));
    
        int diagonal_moves = min(x_diff, y_diff);
        int straight_moves = max(x_diff, y_diff) - diagonal_moves;
    
        int total_moves = diagonal_moves + straight_moves;
    
        return total_moves;
    }
    void Registrar_coche(Car_Id id, Pos p) {
        for(Coche cn:c) {
            if(cn.datos.cid == id) return;
        }
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            pair<int,Car> cot_1;
            int dist_1 = Distancia(c[i].datos.pos,p);
            cot_1 = make_pair(dist_1, car(id));
            add_elem(c[i].Posiciones_coches_enemigos, cot_1);
        }
    }
    void Registrar_gas_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_gas_bonus, dp);
        }        
    }
    void Registrar_water_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_water_bonus, dp);
        }
    }
    void Registrar_missile_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_missil_bonus, dp);
        }
    }
    void Registrar_board() {
        for(int i = 0; i < number_rows(); ++i) {
            for (int j = 0; j < number_universe_columns(); ++j)
            {
                Pos p = {i,j};
                Cell c = cell(p);
                if (c.type == CAR) Registrar_coche(c.cid, p);
                else if (c.type == GAS_BONUS) Registrar_gas_bonus(p);
                else if (c.type == WATER_BONUS) Registrar_water_bonus(p);
                else if (c.type == MISSILE_BONUS) Registrar_missile_bonus(p);
            }
            
        }
    }

    bool evadir(int num_coche) {
        vector<Dir> all_dirs = {UP,DEFAULT,DOWN, SLOW_UP,SLOW, SLOW_DOWN};
        Pos pp = c[num_coche].datos.pos + DEFAULT;
        Dir dd = DEFAULT;
        bool trobat = false;
        for(Dir d: all_dirs) {
            pp = c[num_coche].datos.pos + d;
            dd = d;
            if(within_universe(pp)) {
                if(cell(pp+Dir(0,-1)).type != MISSILE or cell(pp+Dir(0,-2)).type != MISSILE) {
                    if (cell(pp).type == GAS_BONUS or cell(pp).type == MISSILE_BONUS or cell(pp).type == WATER_BONUS or cell(pp).type == EMPTY) {
                        trobat = true;
                        break;
                    }
                }
            }
        }
        if(trobat) {
            move(c[num_coche].datos.cid,dd);
            return true;
        }
        return false;
    }
    bool Ir_gas(int num_coche) {
        int id_compa = 0;
        if(num_coche == id_compa) id_compa = 1;
        Pos obj_compa = c[id_compa].objetivo;
        Pos p = c[num_coche].Posiciones_gas_bonus.top().second;
        if (first(p) == first(obj_compa) and second(p) == second(obj_compa)) {
            c[num_coche].Posiciones_gas_bonus.pop();
            p = c[num_coche].Posiciones_gas_bonus.top().second;
        }
        list<Dir> cam = Camino_rapido(c[num_coche].datos.pos,p,c[num_coche].datos.cid);
        if (not cam.empty()) {
            move(c[num_coche].datos.cid,cam.front());
            c[num_coche].objetivo = p;
            return true;
        }
        return false;
    }
    bool Ir_water(int num_coche) {
        int id_compa = 0;
        if(num_coche == id_compa) id_compa = 1;
        Pos obj_compa = c[id_compa].objetivo;
        Pos p = c[num_coche].Posiciones_water_bonus.top().second;
        if (first(p) == first(obj_compa) and second(p) == second(obj_compa)) {
            c[num_coche].Posiciones_water_bonus.pop();
            p = c[num_coche].Posiciones_water_bonus.top().second;
        }
        list<Dir> cam = Camino_rapido(c[num_coche].datos.pos,p,c[num_coche].datos.cid);
        if (not cam.empty()) {
            move(c[num_coche].datos.cid,cam.front());
            c[num_coche].objetivo = p;
            return true;
        }
        return false;
    }
    bool Ir_missile_bonus(int num_coche) {
        int id_compa = 0;
        if(num_coche == id_compa) id_compa = 1;
        Pos obj_compa = c[id_compa].objetivo;
        Pos p = c[num_coche].Posiciones_missil_bonus.top().second;
        if (first(p) == first(obj_compa) and second(p) == second(obj_compa)) {
            c[num_coche].Posiciones_missil_bonus.pop();
            p = c[num_coche].Posiciones_missil_bonus.top().second;
        }
        list<Dir> cam = Camino_rapido(c[num_coche].datos.pos,p,c[num_coche].datos.cid);
        if (not cam.empty()) {
            move(c[num_coche].datos.cid,cam.front());
            c[num_coche].objetivo = p;
            return true;
        }
        return false;
    }
    
    void Tomar_decision(int nc) {
        if(c[nc].datos.gas < 30) {
            if (Ir_gas(nc)) return;
            else if (c[nc].datos.nb_miss <= 3) {
                if (Ir_missile_bonus(nc)) return;
                else if (Ir_water(nc)) return;
                else if (c[nc].datos.nb_miss > 0) {
                    shoot(c[nc].datos.cid);
                    return;
                }
                else if (evadir(nc)) return;
                else shoot(c[nc].datos.cid);
            }
            else if (Ir_water(nc)) return;
            else if (Ir_missile_bonus(nc)) return;
            else if (c[nc].datos.nb_miss > 0) {
                    shoot(c[nc].datos.cid);
                    return;
                }
            else if (evadir(nc)) return;
            else shoot(c[nc].datos.cid);
        }
        else if (c[nc].datos.nb_miss <= 3) {
            if (Ir_missile_bonus(nc)) return;
            else if (Ir_water(nc)) return;
            else if (Ir_gas(nc)) return;
            else if (c[nc].datos.nb_miss > 0) {
                    shoot(c[nc].datos.cid);
                    return;
                }
            else if (evadir(nc)) return;
            else shoot(c[nc].datos.cid);
        }
        else {
            if (Ir_water(nc)) return;
            else if (Ir_gas(nc)) return;
            else if (Ir_missile_bonus(nc)) return;
            else if (c[nc].datos.nb_miss > 0) {
                    shoot(c[nc].datos.cid);
                    return;
                }
            else if (evadir(nc)) return;
            else shoot(c[nc].datos.cid);
        }

        shoot(c[nc].datos.cid);
    }

    // Llamada 1 vez por ronda
    void Update() {
        c = vector<Coche>(0);
        for (Car_Id cid = begin(me()); cid != end(me()); ++cid) {
            Car cr = car(cid);
            Coche cc;
            cc.datos = cr;
            cc.objetivo = Pos(17,17);
            c.push_back(cc);
        }
        Registrar_board();
        
        if(round() == 0) {
            shoot(c[0].datos.cid);
            Tomar_decision(1);
        }
        else if(round()> 1) {
            Tomar_decision(0);
            Tomar_decision(1);
        }
        else if(round() <= 1) {
            Tomar_decision(1);
        }
    }

    virtual void play () {
        Update();
    }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

