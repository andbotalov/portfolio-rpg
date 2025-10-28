#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <ctime>

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string PINK = "\033[35m";
const std::string CYAN = "\033[36m";

class Character {
protected:
    std::string name;
    int health, maxHealth;
    int attack, defense;
    double critChance, critMultiplier;
    int initiative;
    int expReward;
    bool defending;

public:
    Character(std::string n, int hp, int atk, int def, double critCh, double critMult, int init, int exp = 0)
        : name(std::move(n)), health(hp), maxHealth(hp), attack(atk), defense(def),
        critChance(critCh), critMultiplier(critMult), initiative(init), expReward(exp), defending(false) {
    }

    virtual ~Character() = default;

    const std::string& getName() const { return name; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getATK() const { return attack; }
    int getDEF() const { return defense; }
    double getCritChance() const { return critChance; }
    double getCritMultiplier() const { return critMultiplier; }
    int getInitiative() const { return initiative; }
    int getExpReward() const { return expReward; }

    bool isAlive() const { return health > 0; }

    void resetDefending() { defending = false; }

    virtual void takeTurn(std::vector<Character*>& enemies) = 0;

    void startDefending() { defending = true; }

    void takeDamage(int dmg) {
        int totalDefense = defense;
        if (defending) totalDefense += 25;
        if (totalDefense > 80) totalDefense = 80;

        int reduced = dmg - (dmg * totalDefense / 100);
        if (reduced < 0) reduced = 0;

        health -= reduced;
        if (health < 0) health = 0;

        std::cout << name << " takes " << reduced << " damage (HP: "
            << health << "/" << maxHealth << ")\n";
    }

    void attackTarget(Character& target) {
        static std::mt19937 rng((unsigned)time(nullptr));
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        int dmg = attack;
        bool isCrit = (dist(rng) < critChance);
        if (isCrit) {
            dmg = static_cast<int>(dmg * critMultiplier);
            std::cout << YELLOW << "Critical hit! " << RESET;
        }

        std::cout << name << " attacks " << target.getName()
            << " for " << dmg << " damage.\n";
        target.takeDamage(dmg);
    }

    virtual void printStats(bool colorName = false, bool isHero = false) const {
        if (isHero) std::cout << GREEN << name << RESET;
        else if (colorName) std::cout << RED << name << RESET;
        else std::cout << name;

        std::cout << " (HP: " << health << "/" << maxHealth
            << ", ATK: " << attack
            << ", DEF: " << defense << "%"
            << ", INIT: " << initiative
            << ", CRIT: " << critChance * 100 << "%"
            << ", CRITx: " << critMultiplier
            << ", EXP: " << expReward << ")";
    }
};

class Hero : public Character {
    int level;
    int exp;

public:
    Hero(std::string n, int lvl, int hp, int atk, int def, double critCh, double critMult, int init, int exp_)
        : Character(std::move(n), hp, atk, def, critCh, critMult, init),
        level(lvl), exp(exp_) {
    }

    void takeTurn(std::vector<Character*>& enemies) override {
        if (enemies.empty()) return;

        std::cout << "\nChoose target:\n";
        for (size_t i = 0; i < enemies.size(); i++) {
            std::cout << (i + 1) << ". " << enemies[i]->getName()
                << " (HP: " << enemies[i]->getHealth() << ")\n";
        }

        int targetIndex;
        std::cin >> targetIndex;
        if (targetIndex < 1 || targetIndex >(int)enemies.size()) return;
        Character* target = enemies[targetIndex - 1];

        std::cout << "\nChoose action:\n";
        std::cout << "1. " << PINK << "Attack" << RESET << "\n";
        std::cout << "2. " << CYAN << "Defend" << RESET << "\n";
        std::cout << "3. " << GREEN << "Retreat" << RESET << "\n";
        std::cout << "4. Skip turn\n";

        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1: attackTarget(*target); break;
        case 2: startDefending(); std::cout << CYAN << name << " braces for defense!\n" << RESET; break;
        case 3: std::cout << GREEN << name << " retreats from battle!\n" << RESET; health = maxHealth; for (auto& e : enemies) e->takeDamage(e->getHealth()); break;
        case 4: std::cout << name << " skips the turn.\n"; break;
        }
    }

    void addExp(int amount) {
        exp += amount;
        std::cout << GREEN << name << " gains " << amount << " EXP!\n" << RESET;

        int expToLevel = level * 50; // условие для апа
        while (exp >= expToLevel) {
            exp -= expToLevel;
            levelUp();
            expToLevel = level * 50;
        }
    }

    int getExp() const { return exp; }
    int getLevel() const { return level; }
    void levelUp() {
        level++;
        std::cout << YELLOW << "\n*** Level Up! ***\n" << RESET;

        // варианты апгрейдов
        struct Upgrade {
            std::string desc;
            std::function<void()> apply;
        };
        std::vector<Upgrade> all = {
            {"Increase HP (+5..+10)", [this] {
                int delta = 5 + rand() % 6;
                maxHealth += delta;
                health = maxHealth;
                std::cout << GREEN << "HP +" << delta << RESET << "\n";
            }},
            {"Increase ATK (+2..+4)", [this] {
                int delta = 2 + rand() % 3;
                attack += delta;
                std::cout << PINK << "ATK +" << delta << RESET << "\n";
            }},
            {"Increase DEF (+1..+3)", [this] {
                int delta = 1 + rand() % 3;
                defense += delta;
                std::cout << CYAN << "DEF +" << delta << RESET << "\n";
            }},
            {"Increase CRIT chance (+1..3%)", [this] {
                double delta = (1 + rand() % 3) / 100.0;
                critChance += delta;
                std::cout << YELLOW << "CRIT chance +" << delta * 100 << "%" << RESET << "\n";
            }},
            {"Increase CRIT multiplier (+0.05..0.1)", [this] {
                double delta = 0.05 + (rand() % 6) / 100.0;
                critMultiplier += delta;
                std::cout << YELLOW << "CRITx +" << delta << RESET << "\n";
            }},
        };

        // выбираем 3 уникальных случайных апгрейда
        std::shuffle(all.begin(), all.end(), std::mt19937(std::random_device{}()));
        std::vector<Upgrade> options(all.begin(), all.begin() + 3);

        std::cout << "Choose an upgrade:\n";
        for (int i = 0; i < 3; i++) {
            std::cout << (i + 1) << ". " << options[i].desc << "\n";
        }
        int choice;
        std::cin >> choice;
        if (choice >= 1 && choice <= 3) options[choice - 1].apply();
    }
};

class Enemy : public Character {
public:
    Enemy(std::string n, int hp, int atk, int def, double critCh, double critMult, int init, int exp)
        : Character(std::move(n), hp, atk, def, critCh, critMult, init, exp) {
    }

    void takeTurn(std::vector<Character*>& enemies) override {
        if (enemies.empty()) return;
        Character* target = enemies[0]; // атакуем героя
        std::cout << name << " attacks!\n";
        attackTarget(*target);
    }
};

struct EnemyTemplate {
    std::string name;
    int hp, atk, def, init, exp;
    double crit, critx;
};

std::vector<EnemyTemplate> loadEnemies(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<EnemyTemplate> db;
    std::string line;
    bool header = true;
    while (std::getline(file, line)) {
        if (header) { header = false; continue; }
        std::stringstream ss(line);
        std::string token;
        EnemyTemplate et;
        std::getline(ss, et.name, ';');
        std::getline(ss, token, ';'); et.hp = std::stoi(token);
        std::getline(ss, token, ';'); et.atk = std::stoi(token);
        std::getline(ss, token, ';'); et.def = std::stoi(token);
        std::getline(ss, token, ';'); et.crit = std::stod(token);
        std::getline(ss, token, ';'); et.critx = std::stod(token);
        std::getline(ss, token, ';'); et.init = std::stoi(token);
        std::getline(ss, token, ';'); et.exp = std::stoi(token);
        db.push_back(et);
    }
    return db;
}

Hero loadHero(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::string name;
        std::cout << "Enter hero name: ";
        std::cin >> name;
        return Hero(name, 1, 100, 10, 5, 0.1, 1.4, 15, 0);
    }

    std::string line;
    std::getline(file, line);
    std::getline(file, line);
    std::stringstream ss(line);
    std::string token, name;
    std::getline(ss, name, ';');
    std::getline(ss, token, ';'); int lvl = std::stoi(token);
    std::getline(ss, token, ';'); int hp = std::stoi(token);
    std::getline(ss, token, ';'); int atk = std::stoi(token);
    std::getline(ss, token, ';'); int def = std::stoi(token);
    std::getline(ss, token, ';'); double crit = std::stod(token);
    std::getline(ss, token, ';'); double critx = std::stod(token);
    std::getline(ss, token, ';'); int init = std::stoi(token);
    std::getline(ss, token, ';'); int exp = std::stoi(token);

    return Hero(name, lvl, hp, atk, def, crit, critx, init, exp);
}

void saveHero(const Hero& h, const std::string& filename) {
    std::ofstream file(filename);
    file << "HeroName;Level;HP;ATK;DEF;CRIT;CRITx;INIT;EXP\n";
    file << h.getName() << ";" << h.getLevel() << ";"
        << h.getMaxHealth() << ";" << h.getATK() << ";"
        << h.getDEF() << ";" << h.getCritChance() << ";"
        << h.getCritMultiplier() << ";" << h.getInitiative() << ";"
        << h.getExp() << "\n";
}

class Game {
    Hero hero;
    std::vector<EnemyTemplate> enemyDB;

public:
    Game(Hero h, std::vector<EnemyTemplate> db) : hero(std::move(h)), enemyDB(std::move(db)) {}

    void run() {
        while (hero.isAlive()) {
            saveHero(hero, "save.txt");
            std::vector<Enemy*> enemies;

            // создаем 1-3 врагов
            int enemyCount = 1 + rand() % 3;
            for (int i = 0; i < enemyCount; i++) {
                const auto& tmpl = enemyDB[rand() % enemyDB.size()];
                enemies.push_back(new Enemy(tmpl.name, tmpl.hp, tmpl.atk, tmpl.def, tmpl.crit, tmpl.critx, tmpl.init, tmpl.exp));
            }

            std::cout << "\nEnemies appear:\n";
            for (auto e : enemies) { e->printStats(true); std::cout << "\n"; }

            std::vector<Character*> turnOrder;
            turnOrder.push_back(&hero);
            for (auto e : enemies) turnOrder.push_back(e);

            std::sort(turnOrder.begin(), turnOrder.end(), [](Character* a, Character* b) { return a->getInitiative() > b->getInitiative(); });

            while (hero.isAlive() && std::any_of(enemies.begin(), enemies.end(), [](Enemy* e) {return e->isAlive(); })) {
                std::cout << "\n--- Turn Order ---\n";
                for (auto c : turnOrder) {
                    c->printStats(c != &hero, c == &hero);
                    std::cout << "\n";
                }

                for (auto c : turnOrder) {
                    if (!hero.isAlive()) break;
                    if (c->isAlive()) {
                        if (c == &hero) {
                            std::vector<Character*> enemyVec;
                            for (auto e : enemies) if (e->isAlive()) enemyVec.push_back(e);
                            c->takeTurn(enemyVec);
                        }
                        else {
                            std::vector<Character*> heroVec = { &hero };
                            c->takeTurn(heroVec);
                        }
                        c->resetDefending();
                    }
                }
            }

            if (hero.isAlive()) {
                int gainedExp = 0;
                for (auto e : enemies) if (!e->isAlive()) gainedExp += e->getExpReward();
                hero.addExp(gainedExp);
            }

            for (auto e : enemies) delete e;
            enemies.clear();

            saveHero(hero, "save.txt");
        }
    }
};

int main() {
    srand((unsigned)time(nullptr));
    auto enemyDB = loadEnemies("enemies_db.csv");
    Hero hero = loadHero("save.txt");
    if (std::ifstream("save.txt")) {
        std::cout << GREEN << "Hero loaded successfully!\n" << RESET;
        std::cout << "Continue with this hero? (y/n): ";
        char ans;
        std::cin >> ans;
        if (ans == 'n' || ans == 'N') {
            std::string newName;
            std::cout << "Enter new hero name: ";
            std::cin >> newName;
            hero = Hero(newName, 1, 100, 10, 5, 0.1, 1.4, 15, 0);
        }
    }
    Game game(hero, enemyDB);
    game.run();
}
