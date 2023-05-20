
#include "Player.hh"
#include<queue>
#include<algorithm>
#include<cmath>
#include<list>
#include<set>

using namespace std;


/**
 * Write the name of your player and save this file
 * as AI<name>.cc
 */
#define PLAYER_NAME Maikel

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

    #define MAX_PASOS 100
    
    typedef pair<double,Pos> dist_and_p;
    struct Node_Astar {
        // Distacia del node inicial
        double G_cost = INFINITY;
        // Distacia del node final
        double H_cost = INFINITY;
        double F_cost() {
            return G_cost + H_cost;
        }
        Pos p;
        Node_Astar* parent = nullptr;
    };
    struct cmp_dist_p {
        bool operator()(const dist_and_p& a, const dist_and_p& b) const {
        // Comparación basada en la magnitud del número complejo
        return a.first > b.first;
        }
    };
    struct cmp_dist_c {
        bool operator()(const pair<double,Car>& a, const pair<double,Car>& b) const {
        // Comparación basada en la magnitud del número complejo
        return a.first > b.first;
        }
    };
    struct cmp_a_star {
        bool operator()(const Node_Astar& a, const Node_Astar& b) const {
        // Comparación basada en la magnitud del número complejo
        return (a.G_cost + a.H_cost) > (b.G_cost + b.H_cost);
        }
    };
    typedef priority_queue<dist_and_p, vector<dist_and_p>, cmp_dist_p> priority_queue_dist_and_p;
    typedef priority_queue<pair<double,Car>, vector<pair<double,Car>>, cmp_dist_c> priority_queue_dist_and_c;
    typedef enum Accion {
        ILDE,
        IR_A_GAS,
        IR_A_BONUS,
        IR_A_MISSIL,
        ATACAR,
        EVADIR,
        SHOOT,
        MOVE
    } Accion;
    typedef struct Cotxe {
        Car datos;
        Accion accion;
        priority_queue_dist_and_p Posiciones_water_bonus;
        priority_queue_dist_and_p Posiciones_gas_bonus;
        priority_queue_dist_and_p Posiciones_missil_bonus;
        priority_queue_dist_and_p Posiciones_missil;
        priority_queue_dist_and_c Posiciones_coches_enemigos;
    }Cotxe;
    vector<Cotxe> c;
    /**
     * Play method.
     * 
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     */
    bool usage(string s) {
        cerr << "fatal error: " << s << endl;
        exit(EXIT_FAILURE);
    }
    void add_elem(priority_queue_dist_and_p& v, dist_and_p elem) {
        v.push(elem);
    }
    void add_elem(priority_queue_dist_and_c& v, pair<double,Car> elem) {
        v.push(elem);
    }
    bool comprobar_si_rango(Pos pos1, Pos pos2) {
        int xf = second(pos2);
        int xi = second(pos1);
        int yf = first(pos2);
        int yi = first(pos1);

        return true;
    }
    bool Comprovar_Cell(Pos p, CType tipo) {
        return cell(p).type == tipo;
    }
    bool Es_Cell_circulable (Pos p) {
        if(cell(p).type == TYRE or not within_window(p,round())) return false;
        return true;
    }
    void Imprimir_posicion(Pos p) {
        cerr << first(p) << ',' << second(p) << endl;
    }
    Dir ir_a_pos(Pos ini, Pos fin) {
        int xf = second(fin);
        int xi = second(ini);
        int yf = first(fin);
        int yi = first(ini);
        if (xi > xf) xf += number_universe_columns()-1;
        int delta_x = xf - xi;
        int delta_y = yf - yi;
        if (delta_x == 0) {
            if (delta_y == 0) return SLOW;
            if (delta_y == -1) return SLOW_UP;
            if (delta_y == 1) return SLOW_DOWN;
            else usage("Intentas hacer un movimiento Y ilegal");

        }
        if (delta_x == 1) {
            if (delta_y == 0) return DEFAULT;
            if (delta_y == -1) return UP;
            if (delta_y == 1) return DOWN;
            else usage("Intentas hacer un movimiento Y ilegal");
        }
        if (delta_x == 2) {
            if (delta_y == 0) return FAST;
            if (delta_y == -1) return FAST_UP;
            if (delta_y == 1) return FAST_DOWN;
            else usage("Intentas hacer un movimiento Y ilegal");
        }
        else usage("Intentas hacer un movimiento X ilegal");
        return DEFAULT;
    }

    bool Es_accion_segura(Dir d, int num_coche) {
        Pos pp = c[num_coche].datos.pos + d;
        if(not Es_Cell_circulable(pp)) return false;
        else if (cell(pp-DEFAULT).type == MISSILE) return false;
        return true;
    }
    bool Es_accion_segura(int num_coche) {
        return Es_accion_segura(DEFAULT,num_coche);
    }

    pair<Dir,bool> Direccion_segura(int num_coche) {
        for (int i = 0; i < 2; i++) {
            for (int j = -1; j < 2; j++)
            {
                if(i == 0 and j == 0) continue;
                else {
                    Pos pn = {first(c[num_coche].datos.pos) + j, second(c[num_coche].datos.pos) + i};
                    if(within_window(pn,round()+1) and cell(pn).type != TYRE and cell(pn - DEFAULT).type != MISSILE){
                        return pair<Dir,bool> {ir_a_pos(c[num_coche].datos.pos,pn),true};
                    }
                }
            }
        }
        return pair<Dir,bool>{DEFAULT,false};
    }

    double Calcular_distancia(Pos ini, Pos fin) {
        int xf = second(fin);
        int xi = second(ini);
        int yf = first(fin);
        int yi = first(ini);
        double d = INFINITY;
        if(not comprobar_si_rango(ini, fin)) return d;
        if (xi > xf) xf += number_universe_columns()-1;
        int delta_x = abs(xf - xi);
        int delta_y = abs(yf - yi);

        if(delta_x >= delta_y) return 12*delta_y + 10*(delta_x-delta_y);
        else return 20*delta_x + 10*(delta_y-delta_x);
    }
    
    //void Tratar_muerte_coche(int numero_coche);

    void Registrar_coche(Car_Id id, Pos p) {
        for(Cotxe cn:c) {
            if(cn.datos.cid == id) return;
        }
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            pair<double,Car> cot_1;
            double dist_1 = Calcular_distancia(c[i].datos.pos,p);
            cot_1 = make_pair(dist_1, car(id));
            add_elem(c[i].Posiciones_coches_enemigos, cot_1);
        }
    }
    void Registrar_gas_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Calcular_distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_gas_bonus, dp);
        }        
    }
    void Registrar_water_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Calcular_distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_water_bonus, dp);
        }
    }
    void Registrar_missile_bonus(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Calcular_distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_missil_bonus, dp);
        }
    }
    void Registrar_missil(Pos p) {
        int size_c = c.size();
        for (int i = 0; i < size_c; i++) {
            dist_and_p dp = make_pair(Calcular_distancia(c[i].datos.pos,p),p);
            add_elem(c[i].Posiciones_missil, dp);
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


    void ilde(int num_coche) {
        Ejecutar_Accion(MOVE,DEFAULT,num_coche);
    }
    void ir_a_gas_bonus(int num_coche) {
        Dir d_m = ir_a_pos(c[num_coche].datos.pos, algorith_Astar_np(c[num_coche].datos.pos, c[num_coche].Posiciones_gas_bonus.top().second));
        Ejecutar_Accion(MOVE,d_m,num_coche);
    }
    void ir_a_missile_bonus(int num_coche) {
        Dir d_m = ir_a_pos(c[num_coche].datos.pos, algorith_Astar_np(c[num_coche].datos.pos, c[num_coche].Posiciones_missil_bonus.top().second));
        Ejecutar_Accion(MOVE,d_m,num_coche);
    }
    void ir_a_water_bonus(int num_coche) {
        Dir d_m = ir_a_pos(c[num_coche].datos.pos, algorith_Astar_np(c[num_coche].datos.pos, c[num_coche].Posiciones_water_bonus.top().second));
        Ejecutar_Accion(MOVE,d_m,num_coche);
    }
    //void evadir(int num_coche);
    void atacar(int num_coche) {
        Ejecutar_Accion(SHOOT,num_coche);
    }


    list<Node_Astar> Get_neighbours (Node_Astar node) {
        list<Node_Astar> nl;
        for (int i = 0; i < 3; i++) {
            for (int j = -1; j < 2; j++)
            {
                if(i == 0 and j == 0) continue;
                else {
                    Pos pn = {first(node.p) + j, second(node.p) + i};
                    if (within_window(pn,round())) {
                        Node_Astar nn;
                        nn.p = pn;
                        nn.G_cost = INFINITY;
                        nn.H_cost = INFINITY;
                        if (Es_Cell_circulable(pn)) {
                            nl.push_back(nn);
                        }
                    }
                }
            }
            
        }
        return nl;
        
    }
    bool son_pos_iguales(Pos p1, Pos p2) {
        return first(p1) == first(p2) and second(p1) == second(p2);
    }
    bool Is_in_Node_list(list<Node_Astar>& l, Node_Astar& elem) {
        list<Node_Astar>::iterator it = l.begin();
        while (it != l.end()) {
            if (son_pos_iguales((*it).p,elem.p)) return true;
            ++it;
        }
        return false;
        
    }
    Node_Astar Primer_nodo_del_camino(Node_Astar& n, Pos co) {
        if (son_pos_iguales(n.parent->p,co)) return n;
        else if (n.parent == nullptr) return n;
        else return Primer_nodo_del_camino(*(n.parent),co);
    }
    Pos algorith_Astar_np(Pos co, Pos cf, int Pasos_maximos) {
        list<Node_Astar> Open_set(1);
        list<Node_Astar> Close_set;
        (*Open_set.begin()).p = co;
        (*Open_set.begin()).G_cost = 0;
        (*Open_set.begin()).H_cost = INFINITY;
        while (not Open_set.empty()) {
            list<Node_Astar>::iterator current_node = Open_set.begin();
            list<Node_Astar>::iterator it = Open_set.begin();
            while (it != Open_set.end()) {
                if ((*it).F_cost() < (*current_node).F_cost()) {
                    current_node = it;
                }
                else if ((*it).F_cost() == (*current_node).F_cost() and (*it).H_cost < (*current_node).H_cost) {
                    current_node = it;
                }
                ++it;
            }
            it = Close_set.insert(Close_set.end(),*current_node);
            Open_set.erase(current_node);
            current_node = it;

            if (son_pos_iguales((*current_node).p,cf)) {
                return Primer_nodo_del_camino((*current_node),co).p;
            }
            
            for(Node_Astar neighbour:Get_neighbours((*current_node))) {
                if(not Es_Cell_circulable(neighbour.p) or Is_in_Node_list(Close_set,neighbour)) continue;
                double nMoveCosttoNeigh = (*current_node).G_cost + Calcular_distancia((*current_node).p, neighbour.p);
                if (not Is_in_Node_list(Open_set,neighbour) or nMoveCosttoNeigh < neighbour.G_cost) {
                    neighbour.G_cost = nMoveCosttoNeigh;
                    neighbour.H_cost = Calcular_distancia(neighbour.p, cf);
                    neighbour.parent = &(*current_node);
                    if(not Is_in_Node_list(Open_set,neighbour)) Open_set.push_back(neighbour);
                }
            }
        }
        return co+DEFAULT;
    }
    Pos algorith_Astar_np(Pos co, Pos cf) {
        return algorith_Astar_np(co,cf,MAX_PASOS);
    }

    void Tomar_decision() {
        if(c[0].Posiciones_gas_bonus.empty()) ilde(0);
        else ir_a_gas_bonus(0);
    }
    void Ejecutar_Accion(Accion a, int num_coche) {
        if(a == SHOOT){
            if (Es_accion_segura(num_coche)) shoot(c[num_coche].datos.cid);
            else {
                pair<Dir,bool> Dir_seg = Direccion_segura(num_coche);
                if (Dir_seg.second) move(c[num_coche].datos.cid,Dir_seg.second);
                else if (c[num_coche].datos.nb_miss > 0) shoot(c[num_coche].datos.cid);
                else shoot(c[num_coche].datos.cid);
            }
        }
        else usage("Accion incorrecta (accion ejecutada SHOOT)");
    }
    void Ejecutar_Accion(Accion a, Dir d, int num_coche) {
        if(a == MOVE) {
            if (Es_accion_segura(d,num_coche)) move(c[num_coche].datos.cid,d);
            else {
                pair<Dir,bool> Dir_seg = Direccion_segura(num_coche);
                if (Dir_seg.second) move(c[num_coche].datos.cid,Dir_seg.second);
                else if (c[num_coche].datos.nb_miss > 0) shoot(c[num_coche].datos.cid);
                else move(c[num_coche].datos.cid,d);
            }
        }
        else usage("Accion incorrecta (accion ejecutada MOVE)");
    }

    //Llamado solo al inicio de la partida
    void Start() {

    }
    // Llamada 1 vez por ronda
    void Update() {
        c = vector<Cotxe>(0);
        for (Car_Id cid = begin(me()); cid != end(me()); ++cid) {
            Car cr = car(cid);
            Cotxe nc;
            nc.datos = cr;
            c.push_back(nc);
        }
        cerr << c.size()<< endl;
        Registrar_board();
        Tomar_decision();

    }

    virtual void play () {

        if(round() == 0) Start();
        
        
        Update();
    }

    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

