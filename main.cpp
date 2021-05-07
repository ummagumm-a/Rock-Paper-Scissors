// Lecture 13 - Type casting
#include <iostream>
#include <tuple>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;
using namespace std::chrono_literals;

constexpr int TIMEOUT = 400; // maximum number of milliseconds that a player is allowed to take
constexpr int gridSideSize = 15;

enum class Symbols // this enum class describes all possible symbols on the board and an empty cell
{
    s, S, p, P, r, R, M, f, F, empty
};

class Position
{
public:
    // ctor for creation of new position
    Position(int row, int column)
        :
        pos(row, column)
    {}

    // compare on equal operator
    bool operator==(const Position& rhs) const
    {
        return get<0>(pos) == get<0>(rhs.pos)
               && get<1>(pos) == get<1>(rhs.pos);
    }

    // copy assignment operator
    Position& operator=(const Position& rhs)
    {
        if (this == &rhs) return *this;

        get<0>(pos) = get<0>(rhs.pos);
        get<1>(pos) = get<1>(rhs.pos);

        return *this;
    }

    // returns the row of this position
    [[nodiscard]] int getRow() const
    {
        return get<0>(pos);
    }

    // returns the column of this position
    [[nodiscard]] int getColumn() const
    {
        return get<1>(pos);
    }

private:
    std::tuple<int, int> pos;
};

class World
{
public:
    // ctor for creating a world
    World()
    {
        for (int i = 0; i < gridSideSize; ++i)
        {
            vector<unique_ptr<Symbols>> tmp;
            board.push_back(move(tmp));
            for (int j = 0; j < gridSideSize; ++j)
            {
                board[i].push_back(make_unique<Symbols>(Symbols::empty));
            }
        }
    }

    void init()
    {
        // place flags
        board[0][0] = make_unique<Symbols>(Symbols::f);
        board[gridSideSize - 1][gridSideSize - 1] = make_unique<Symbols>(Symbols::F);

        // define the coordinates of the 0 player's units
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 1; j < 6; ++j)
            {
                set0.emplace_back(i, j);
            }
        }

        // define the coordinates of the 1 player's units
        for (int i = 9; i < 15; ++i)
        {
            for (int j = 9; j < 14; ++j)
            {
                set1.emplace_back(i, j);
            }
        }

        // place set 0
        for (int i = 1; i < 6; ++i)
        {
            board[0][i] = make_unique<Symbols>(Symbols::r);
            board[1][i] = make_unique<Symbols>(Symbols::p);;
            board[2][i] = make_unique<Symbols>(Symbols::s);
            board[3][i] = make_unique<Symbols>(Symbols::r);
            board[4][i] = make_unique<Symbols>(Symbols::p);
            board[5][i] = make_unique<Symbols>(Symbols::s);
        }

        // place set 1
        for (int i = 9; i < 14; ++i)
        {
            board[9][i] = make_unique<Symbols>(Symbols::S);
            board[10][i] = make_unique<Symbols>(Symbols::P);
            board[11][i] = make_unique<Symbols>(Symbols::R);
            board[12][i] = make_unique<Symbols>(Symbols::S);
            board[13][i] = make_unique<Symbols>(Symbols::P);
            board[14][i] = make_unique<Symbols>(Symbols::R);
        }

        // place mountains
        unique_ptr<Symbols> m;
        for (const auto& tmp : mountainCoordinates)
        {
            board[tmp.first][tmp.second] = make_unique<Symbols>(Symbols::M);
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const World& world);

public:
    // a grid representing a board
    // ITEM 3.b: contains unique pointers to Symbols, because
    vector<vector<unique_ptr<Symbols>>> board;
    // coordinates of mountains
    inline static vector<pair<int, int>> mountainCoordinates = {
            {10, 2},
            {11, 2},
            {12, 2},
            {10, 4},
            {11, 4},
            {12, 4},
            {12, 5},
            {10, 6},
            {11, 6},
            {12, 6},
            {7,  6},
            {7,  7},
            {7,  8},
            {2,  10},
            {2,  12},
            {4,  9},
            {4,  13},
            {5,  10},
            {5,  11},
            {5,  12}
    };

    // I don't want to use templates here because my program would not
    // benefit from their use.

    // units of player 0
    vector<pair<int, int>> set0;
    // units of player 1
    vector<pair<int, int>> set1;
};

std::ostream& operator<<(std::ostream& out, const World& world)
{
    // choose an appropriate char for a unit
    auto showSymbol = [&out](const unique_ptr<Symbols>& symbolPtr)
    {
        switch (*symbolPtr)
        {
            case Symbols::s : { out << 's'; break; }
            case Symbols::S : { out << 'S'; break; }
            case Symbols::p : { out << 'p'; break; }
            case Symbols::P : { out << 'P'; break; }
            case Symbols::r : { out << 'r'; break; }
            case Symbols::R : { out << 'R'; break; }
            case Symbols::M : { out << 'M'; break; }
            case Symbols::f : { out << 'f'; break; }
            case Symbols::F : { out << 'F'; break; }
            case Symbols::empty : { out << '_'; break; }
        }
    };

    // print all cells on the board
    for (int i = 0; i < gridSideSize; ++i)
    {
        for (int j = 0; j < gridSideSize; ++j)
        {
            showSymbol(world.board[i][j]);
            out << ' ';
        }
        out << endl;
    }

    return out;
}

class Action
{
public:
    // ctor for creating an action
    Action(Position& positionFrom, Position& positionTo)
    {
        from = make_unique<Position>(positionFrom);
        to = make_unique<Position>(positionTo);
    }

    Action() = default;

    unique_ptr<Position> from; // current row, column of the unit to be moved
    unique_ptr<Position> to; // position to where the unit must be moved
};

// return a random unit in a set
auto chooseSymbolRandomly(const vector<pair<int, int>>& set)
{
    return set[rand() % set.size()];
}

// chooses an action for the player 0
Action actionPlayerZero(const World& world)
{
    bool successfulMove = false;
    Action action;
    int count = 0;
    while (!successfulMove)
    {
        // randomly choose some symbol from the set && extract its coordinates
        auto[row, column] = chooseSymbolRandomly(world.set0);

        if (row < column // if needs to change row and if the move will be legal
                && row + 1 != gridSideSize
                    && *world.board[row + 1][column] != Symbols::M
                    && *world.board[row + 1][column] != Symbols::r
                    && *world.board[row + 1][column] != Symbols::p
                    && *world.board[row + 1][column] != Symbols::s
                    && *world.board[row + 1][column] != Symbols::f)
        {
            action.from = make_unique<Position>(Position(row, column)); // old coordinates
            action.to = make_unique<Position>(Position(row + 1, column)); // increase row

            successfulMove = true;
        }
        else if (column + 1 != gridSideSize // otherwise, try to change the column if possible
                    && *world.board[row][column + 1] != Symbols::M
                    && *world.board[row][column + 1] != Symbols::r
                    && *world.board[row][column + 1] != Symbols::p
                    && *world.board[row][column + 1] != Symbols::s
                    && *world.board[row][column + 1] != Symbols::f)
        {
            action.from = make_unique<Position>(Position(row, column)); // old coordinates
            action.to = make_unique<Position>(Position(row, column + 1)); // increase column

            successfulMove = true;
        }

        if (count++ == 100) {
            cout << "can't move :(" << endl;
            exit(0);
        } // if after 100 iterations haven't found a unit that can move - end the game
    }

    return action;
}

// chooses an action for the player 1
Action actionPlayerOne(const World& world) {
    bool successfulMove = false;
    Action action;
    int count = 0;
    while (!successfulMove)
    {
        // randomly choose some symbol from the set && extract its coordinates
        auto[row, column] = chooseSymbolRandomly(world.set1);
        // randomly choose direction of the move
        int randChoice = rand() % 4;
        vector<pair<int, int>> randMoves{
                {row - 1, column},
                {row + 1, column},
                {row, column - 1},
                {row, column + 1}
        };

        // specify which move it takes
        auto[destRow, destColumn] = randMoves[randChoice];
        // check whether the move is possible
        if (destRow >= 0 && destRow < gridSideSize && destColumn >= 0 && destColumn < gridSideSize
                && *world.board[destRow][destColumn] != Symbols::M
                && *world.board[destRow][destColumn] != Symbols::S
                && *world.board[destRow][destColumn] != Symbols::R
                && *world.board[destRow][destColumn] != Symbols::P
                && *world.board[destRow][destColumn] != Symbols::F)
        {
            action.from = make_unique<Position>(Position(row, column)); // old coordinates
            action.to = make_unique<Position>(Position(destRow, destColumn)); // new coordinates

            successfulMove = true;
        }

        if (count++ == 100) {
            cout << "can't move :(" << endl;
            exit(0);
        } // if after 100 iterations haven't found a unit that can move - end the game
    }

    return action;
}

/**
 * The return is a pair: action and a boolean whether a timeout happened
 */
std::tuple<Action, bool> waitPlayer(Action (*f)(const World&), const World& world) {
    auto start = std::chrono::high_resolution_clock::now();
    Action action = f(world);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    if (elapsed.count() > TIMEOUT) // if time > 0.4 s
        return { move(action), true }; // player failed to answer in less than 400 ms
    else return { move(action), false };
}

// validate action - return a boolean which shows whether we need to stop the game
// and string message in case of end of the game
std::tuple<bool, string> validateActions(const World& world, const Action& action0, const Action& action1)
{
    auto player0Dest = *action0.to; // destination of player 0
    auto player1Dest = *action1.to; // destination of player 1

    if (*world.board[player0Dest.getRow()][player0Dest.getColumn()] == Symbols::M
            || action0.to == action0.from
            || player0Dest.getRow() < 0 || player0Dest.getRow() >= gridSideSize
            || player0Dest.getColumn() < 0 || player0Dest.getColumn() >= gridSideSize) // if the player 0 made an illegal move
    {
        return { true, "Player 0 made an illegal move. Player 1 won the game!"};
    }
    else if (*world.board[player1Dest.getRow()][player1Dest.getColumn()] == Symbols::M
             || action1.to == action1.from
             || player1Dest.getRow() < 0 || player1Dest.getRow() >= gridSideSize
             || player1Dest.getColumn() < 0 || player1Dest.getColumn() >= gridSideSize) // if the player 1 made an illegal move
    {
        return { true, "Player 1 made an illegal move. Player 0 won the game!"};
    }
    else if (*world.board[player0Dest.getRow()][player0Dest.getColumn()] == Symbols::F) // if the player 0 captured the flag
    {
        return { true, "Player 0 captured the flag! Hooray!" };
    }
    else if (*world.board[player1Dest.getRow()][player1Dest.getColumn()] == Symbols::f) // if the player 1 captured the flag
    {
        return { true, "Player 1 captured the flag! Hooray!" };
    }
    else // if nothing remarkable happened yet
    {
        return { false, "Nothing interesting yet" };
    }
}

// if two different symbols met in one cell
// return a code which specifies the needed behaviour for game controller
int interaction(const Symbols& p0, const Symbols& p1)
{
    if (p0 == Symbols::s && p1 == Symbols::S
            || p0 == Symbols::r && p1 == Symbols::R
            || p0 == Symbols::p && p1 == Symbols::P // if two similar symbols met
        )
    {
        return 0;
    }
    else if (p0 == Symbols::s && p1 == Symbols::P
            || p0 == Symbols::r && p1 == Symbols::S
            || p0 == Symbols::p && p1 == Symbols::R) // if unit 0 kills unit 1
    {
        return 1;
    }
    else if (p0 == Symbols::p && p1 == Symbols::S
             || p0 == Symbols::s && p1 == Symbols::R
             || p0 == Symbols::r && p1 == Symbols::P) // if unit 1 kills unit 0
    {
        return 2;
    }

    return -1;
}


// change the position of the unit on the board
void moveUnitOnBoard(vector<vector<unique_ptr<Symbols>>>& board, Position& posFrom, Position& posTo)
{
    auto tmp = move(board[posFrom.getRow()][posFrom.getColumn()]);
    board[posFrom.getRow()][posFrom.getColumn()] = make_unique<Symbols>(Symbols::empty);
    board[posTo.getRow()][posTo.getColumn()] = move(tmp);
}

// change the position of the unit in the set of units
void changeCoordsInSet(vector<pair<int, int>>& set, Position& posFrom, Position& posTo)
{
    auto it = find (set.begin(), set.end(), make_pair( posFrom.getRow(), posFrom.getColumn() ));
    it->first = posTo.getRow();
    it->second = posTo.getColumn();
}

// logic for moving a unit
void playerMove(World& world, const Action& action, vector<pair<int, int>>& set)
{
    moveUnitOnBoard(world.board, *action.from, *action.to);
    changeCoordsInSet(set, *action.from, *action.to);
}

void handleInteraction(int code, World& world, Position& pos0, Position& pos0to, Position& pos1, Position& pos1to)
{
    // kill the unit
    auto killing = [](vector<vector<unique_ptr<Symbols>>>& board, Position& pos, vector<pair<int, int>>& set)
    {
        board[pos.getRow()][pos.getColumn()] = make_unique<Symbols>(Symbols::empty); // just empty the killed symbol's cell
        auto it = find (set.begin(), set.end(), make_pair( pos.getRow(), pos.getColumn() ));
        set.erase(it);
    };

    switch (code) {
        case 0: // if two similar symbols met
        {
            // the move is discarded
            break;
        }
        case 1: // when player 0 kills player 1
        {
            killing(world.board, pos1, world.set1);
            playerMove(world, Action(pos0, pos0to), world.set0);

            break;
        }
        case 2: // when player 1 kills player 0
        {
            killing(world.board, pos0, world.set0);
            playerMove(world, Action(pos1, pos1to), world.set1);

            break;
        }
        default:
        {
            cout << "problem" << endl;
            exit(0);
            break;
        }
    }
}

// take actions and move units, update the state of the world
void updateWorld(World& world, Action& action0, Action& action1)
{

    if (*action0.to == *action1.from && *action0.from == *action1.to) // in case the unit just swap places
    {
        swap(world.board[action0.to->getRow()][action0.to->getColumn()], world.board[action1.to->getRow()][action1.to->getColumn()]);
        auto it0 = find (world.set0.begin(), world.set0.end(), make_pair( action0.from->getRow(), action0.from->getColumn() ));
        auto it1 = find (world.set1.begin(), world.set1.end(), make_pair( action1.from->getRow(), action1.from->getColumn() ));
        swap(*it0, *it1);

        return;
    }
    else if (*action0.to == *action1.from) // if unit 0 goes to the initial place of unit 1
    {
        int code = interaction(
                *world.board[action1.to->getRow()][action1.to->getColumn()],
                *world.board[action1.from->getRow()][action1.from->getColumn()]
        );
        if (code == -1)
            playerMove(world, action1, world.set1);
        else
            handleInteraction(code, world, *action1.to, *action1.to, *action1.from, *action1.to);

        code = interaction(
                *world.board[action0.from->getRow()][action0.from->getColumn()],
                *world.board[action0.to->getRow()][action0.to->getColumn()]
        );
        if (code == -1)
            playerMove(world, action0, world.set0);
        else
            handleInteraction(code, world, *action0.from, *action0.to, *action0.to, *action0.to);
    }
    else if (*action1.to == *action0.from) // if unit 1 goes to the initial place of unit 0
    {
        int code = interaction(
                *world.board[action0.from->getRow()][action0.from->getColumn()],
                *world.board[action0.to->getRow()][action0.to->getColumn()]
        );
        if (code == -1)
            playerMove(world, action0, world.set0);
        else
            handleInteraction(code, world, *action0.from, *action0.to, *action0.to, *action0.to);

        code = interaction(
                *world.board[action1.to->getRow()][action1.to->getColumn()],
                *world.board[action1.from->getRow()][action1.from->getColumn()]
        );
        if (code == -1)
            playerMove(world, action1, world.set1);
        else
            handleInteraction(code, world, *action1.to, *action1.to, *action1.from, *action1.to);
    }
    else if (*action0.to == *action1.to) // if units 0 and 1 move to the same place
    {
        int code = interaction(
                *world.board[action0.from->getRow()][action0.from->getColumn()],
                *world.board[action1.from->getRow()][action1.from->getColumn()]
        );

        handleInteraction(code, world, *action0.from, *action0.to, *action1.from, *action1.to);
    }
    else // if moves of unit 0 and unit 1 are independent
    {
        {
            int code = interaction(
                    *world.board[action0.from->getRow()][action0.from->getColumn()],
                    *world.board[action0.to->getRow()][action0.to->getColumn()]
            );

            if (code == -1)
                playerMove(world, action0, world.set0);
            else
                handleInteraction(code, world, *action0.from, *action0.to, *action0.to, *action0.to);
        }

        {
            int code = interaction(
                    *world.board[action1.to->getRow()][action1.to->getColumn()],
                    *world.board[action1.from->getRow()][action1.from->getColumn()]
            );

            if (code == -1)
                playerMove(world, action1, world.set1);
            else
                handleInteraction(code, world, *action1.to, *action1.to, *action1.from, *action1.to);
        }
    }
}

// save the state of the world to the file
void saveProgress(const World& world)
{
    ofstream file("savefile.txt");
    if (file.is_open())
    {
        // save board
        file << "Board" << endl;
        file << world;

        // save set 0
        file << "Set 0" << endl;
        for (int i = 0; i < world.set0.size(); ++i)
        {
            file << world.set0[i].first << " " << world.set0[i].second << endl;
        }

        // save set 1
        file << "Set 1" << endl;
        for (int i = 0; i < world.set1.size(); ++i)
        {
            file << world.set1[i].first << " " << world.set1[i].second << endl;
        }
        file << "End" << endl;

        file.close();
    }
}


// parse the save file. Create the world
World startSavedGame()
{
    // choose an appropriate unit for a char
    auto chooseSymbol = [](char& symbol)
    {
        switch (symbol)
        {
            case 's' : { return make_unique<Symbols>(Symbols::s); }
            case 'S' : { return make_unique<Symbols>(Symbols::S); }
            case 'p' : { return make_unique<Symbols>(Symbols::p); }
            case 'P' : { return make_unique<Symbols>(Symbols::P); }
            case 'r' : { return make_unique<Symbols>(Symbols::r); }
            case 'R' : { return make_unique<Symbols>(Symbols::R); }
            case 'M' : { return make_unique<Symbols>(Symbols::M); }
            case 'f' : { return make_unique<Symbols>(Symbols::f); }
            case 'F' : { return make_unique<Symbols>(Symbols::F); }
            case '_' : { return make_unique<Symbols>(Symbols::empty); }
        }
    };

    World world;
    ifstream file("savefile.txt");

    if (file.is_open())
    {
        string fInput;
        if (file >> fInput, fInput != "Board") cout << "problem" << endl;

        char input = 0;
        int count = 0;

        // fill the board
        while (count != 225)
        {
            file >> input;
            world.board[count / 15][count % 15] = chooseSymbol(input);
            count++;
        }

        // fill Set 0
        string i, j;
        file >> i >> j;
        while (file >> i >> j, i != "Set")
        {
            world.set0.emplace_back( stoi(i), stoi(j) );
        }

        // fill Set 1
        while (file >> i >> j, i != "End")
        {
            world.set1.emplace_back( stoi(i), stoi(j) );
        }
    }

    return world;
}

// choose whether to start new game or to continue the saved one
void gameStart(World& world)
{
    string message;
    cout << "do you want to load the save game? Y/n" << endl;
    cin >> message;
    if (message == "Y")
    {
        world = startSavedGame();
    }
    else
    {
        world.init();
    }
}

// the main method of the program
int main() {
    srand ( time(NULL) );

    World world;
    gameStart(world);
    cout << world;
    int count = 0;

    bool endGame = false;
    while (!endGame) {
        auto[action0, timeout0] = waitPlayer(actionPlayerZero, world);
        auto[action1, timeout1] = waitPlayer(actionPlayerOne, world);

        if (timeout0 || timeout1)
        {
            endGame = true;
            if (timeout0) cout << "Player 1 won. Player 0, you are a slowpoke!" << endl;
            else cout << "Player 0 won. Player 1, you are a slowpoke!" << endl;
        }
        else
        {
            string message;
            tie(endGame, message) = validateActions(world, action0, action1);

            updateWorld(world, action0, action1);
            cout << world;

            // save the game after 50 iterations
            if (count++ == 50) saveProgress(world);
            if (endGame)
            {
                cout << message << endl;
                break;
            }
        }
    }
    return 0;
}