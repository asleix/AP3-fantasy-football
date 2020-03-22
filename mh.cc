#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <chrono>
#include <cmath>
#include <algorithm>
using namespace std;
using namespace std::chrono;


// Data structure of a player
// Includes comparison funct for sorting
struct Jugador {
    string name;
    string pos;
    int price;
    string team;
    int points;
};

bool compara_per_preu(const Jugador& a, const Jugador& b) {
    if(a.price!=b.price) return a.price < b.price;
    return a.points < b.points;
}

using Jugadors = vector<Jugador>;

//Data
vector<int> N(4); //restrictions
Jugadors players; //Data base

struct Team {
    Jugadors play = Jugadors(11);
    int price=0;
    int points=0;
};

// Print team: players, points and price
auto start = high_resolution_clock::now();

struct file_to_print {
    ofstream file;
    string oname;

    void print_position(const Team& best_team, const string& pos, ofstream& file) {
        bool first = true;
        for(Jugador j : best_team.play) if(j.pos==pos) {
            if(first) first = false;
            else file << ';';
            file << j.name;
        }
        file << endl;
    }

    void print_solution (const Team& best_team) {
        file.open(oname);

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        double time = double(duration.count())/1000;
        file << fixed << setprecision(1) << time << endl;

        file << "POR: ";
        print_position(best_team, "por", file);
        file << "DEF: ";
        print_position(best_team, "def", file);
        file << "MIG: ";
        print_position(best_team, "mig", file);
        file << "DAV: ";
        print_position(best_team, "dav", file);

        file << "Punts: " << best_team.points << endl;
        file << "Preu: " << best_team.price << endl;
        file.close();
    }
} f;


//Greedy algorithm to find
Team generateInitialSolution() {
    Team t;
    int p, i, j, k;
    p=i=j=k=0;
    int pos = 0;
    bool completed = false;
    while(not completed) {
        Jugador jug = players[pos];
        if(t.price + jug.price<=N[0]) {
            if(jug.pos == "por" and p < 1) {
                t.play[p+i+j+k]=jug;
                p++;
                t.price+=jug.price;
                t.points+=jug.points;
            }else if(jug.pos == "def" and i<N[1]) {
                t.play[p+i+j+k]=jug;
                i++;
                t.price+=jug.price;
                t.points+=jug.points;
            }else if(jug.pos == "mig" and j<N[2]) {
                t.play[p+i+j+k]=jug;
                j++;
                t.price+=jug.price;
                t.points+=jug.points;
            }else if(jug.pos == "dav" and k<N[3]) {
                t.play[p+i+j+k]=jug;
                k++;
                t.price+=jug.price;
                t.points+=jug.points;
            }
        }
        pos++;
        completed = (p+i+j+k == 11);
    }
    return t;
}

// Returns the index of the player with the max price(binary search)
int bsPrice(int maxPrice, int l, int r) {
    if(l == r) return l;
    int m = (l+r)/2;
    if(players[m].price <= maxPrice)
        return bsPrice(maxPrice, m+1, r);
    return bsPrice(maxPrice, l, m);
}

// Check if player is repeated
bool repeated_player(const Jugador& j, const Team& t) {
    for(const Jugador& jj : t.play){
        if(jj.price == j.price and jj.name == j.name) {
            return true;
        }
    }
    return false;
}

Team pickNeighbour(const Team& s) {
        Team ns = s;
        int num = rand()%11; //Selecting eliminated player
        int curprice = s.price;
        int elimPrice = s.play[num].price;
        int maxPrice = N[0]-(curprice-elimPrice); //the maximum price the new player can have

        int maxPlayer = bsPrice(maxPrice, 0, players.size()); //the last player with price <= maxPrice

        int newPlayer = rand()%maxPlayer; //New player
        while(players[newPlayer].pos != s.play[num].pos
              or repeated_player(players[newPlayer],s)) {
            newPlayer = rand()%maxPlayer; //New player
        }

        ns.play[num]=players[newPlayer];//Update
        ns.price = s.price - s.play[num].price + ns.play[num].price;
        ns.points = s.points - s.play[num].points + ns.play[num].points;
        return ns;
}

// Probability of changing
double p(double s, double ns, double t) {
    if(ns>s) return 1;
    else return exp(-(s-ns)/t);
}


Team metaheuristic() {
    sort(players.begin(), players.end(), compara_per_preu);
    Team s = generateInitialSolution(); //Greedy

    //double k=1, kmax=200000;
    double t=1;
    srand(time(NULL));
    double alpha = 1-1e-10;

    while (true) {
        t = t*alpha;
        //t = (1-k/kmax); //alpha*t_last
        Team NewS = pickNeighbour(s);
        if (double(rand())/RAND_MAX <= p(s.points, NewS.points, t)){
            s=NewS;
            f.print_solution(s);
        }
        //k++;
    }
    return s;
}


/**
 * Reads football player data base file and creates vector to access players
 * J: Maximum price of a player
   returns: vector of Jugador containing all players in "data_base.txt"
 * */
Jugadors read_database(const int J, char* database){
    ifstream db(database);
    Jugadors data;

    while (not db.eof()) {
        string name, position, club;
        int points, price;
        getline(db,name,';');    if (name == "") break;
        getline(db,position,';');
        db >> price;
        char aux; db >> aux;
        getline(db,club,';');
        db >> points;
        string aux2;
        getline(db,aux2);
        Jugador j = {name, position, price, club, points};
        if(j.price <= J) {
            data.push_back(j);
        }
    }
    db.close();
    return data;
}


int main (int argc, char** argv){

    /*Lectura de dades*/
    if (argc != 4) {
        cout << "Syntax: " << argv[0] << " data-base.txt public_benchs/example.txt out.txt" << endl;
        exit(1);
    }
    int n1, n2, n3, T, J;
    ifstream restrictions(argv[2]); // Read restrictions
    restrictions >> n1 >> n2 >> n3 >> T >> J;
    N[1] = n1; N[2]= n2; N[3] = n3; N[0]=T;
    players = read_database(J, argv[1]); //Read data base

    /*Metaheuristic*/
    start = high_resolution_clock::now();
    string name(argv[3]);
    f.oname = name;

    Team solution = metaheuristic();

    f.print_solution(solution);
}
