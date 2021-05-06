// Lecture 13 - Type casting
#include <iostream>
#include <tuple>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <unordered_map>

using namespace std;
using namespace std::chrono_literals;

constexpr int TIMEOUT = 400; // maximum number of milliseconds that a player is allowed to take
constexpr int gridSideSize = 15;

enum class Symbols
{
    s, S, p, P, r, R, M, f, F, empty
};

class Position
{
public:
    Position()
            :
            pos(-1, -1)
    {}

    Position(int row, int column)
        :
        pos(row, column)
    {}

    bool operator==(const Position& rhs) const
    {
        return get<0>(pos) == get<0>(rhs.pos)
               && get<1>(pos) == get<1>(rhs.pos);
    }

    Position& operator=(const Position& rhs)
    {
        if (this == &rhs) return *this;

        get<0>(pos) = get<0>(rhs.pos);
        get<1>(pos) = get<1>(rhs.pos);

        return *this;
    }

    [[nodiscard]] int getRow() const
    {
        return get<0>(pos);
    }

    [[nodiscard]] int getColumn() const
    {
        return get<1>(pos);
    }

private:
    std::tuple<int, int> pos;
};

class World {
public:
    vector<vector<shared_ptr<Symbols>>> board;
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

    vector<pair<int, int>> set0;
    vector<pair<int, int>> set1;

public:
    World()
        :
        board(gridSideSize, vector<shared_ptr<Symbols>>(gridSideSize, make_shared<Symbols>(Symbols::empty)))
    {
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 1; j < 6; ++j)
            {
                set0.emplace_back(i, j);
            }
        }

        for (int i = 9; i < 15; ++i)
        {
            for (int j = 9; j < 14; ++j)
            {
                set1.emplace_back(i, j);
            }
        }
    }

    void init()
    {
        // place flags
        board[0][0] = make_shared<Symbols>(Symbols::f);
        board[gridSideSize - 1][gridSideSize - 1] = make_shared<Symbols>(Symbols::F);

        // place set 0
        for (int i = 1; i < 6; ++i)
        {
            board[0][i] = make_shared<Symbols>(Symbols::r);
            board[1][i] = make_shared<Symbols>(Symbols::p);;
            board[2][i] = make_shared<Symbols>(Symbols::s);
            board[3][i] = make_shared<Symbols>(Symbols::r);
            board[4][i] = make_shared<Symbols>(Symbols::p);
            board[5][i] = make_shared<Symbols>(Symbols::s);
        }

        // place set 1
        for (int i = 9; i < 14; ++i)
        {
            board[9][i] = make_shared<Symbols>(Symbols::S);
            board[10][i] = make_shared<Symbols>(Symbols::P);
            board[11][i] = make_shared<Symbols>(Symbols::R);
            board[12][i] = make_shared<Symbols>(Symbols::S);
            board[13][i] = make_shared<Symbols>(Symbols::P);
            board[14][i] = make_shared<Symbols>(Symbols::R);
        }

        // place mountains
        shared_ptr<Symbols> m;
        for (const auto& tmp : mountainCoordinates)
        {
            board[tmp.first][tmp.second] = make_shared<Symbols>(Symbols::M);
        }
    }

    void show() const
    {
        auto showSymbol = [](const shared_ptr<Symbols>& symbolPtr)
        {
            switch (*symbolPtr)
            {
                case Symbols::s : { cout << 's'; break; }
                case Symbols::S : { cout << 'S'; break; }
                case Symbols::p : { cout << 'p'; break; }
                case Symbols::P : { cout << 'P'; break; }
                case Symbols::r : { cout << 'r'; break; }
                case Symbols::R : { cout << 'R'; break; }
                case Symbols::M : { cout << 'M'; break; }
                case Symbols::f : { cout << 'f'; break; }
                case Symbols::F : { cout << 'F'; break; }
                case Symbols::empty : { cout << ' '; break; }
            }
        };

        for (int i = 0; i < gridSideSize; ++i)
        {
            for (int j = 0; j < gridSideSize; ++j)
            {
                showSymbol(board[i][j]);
                cout << ' ';
            }
            cout << endl;
        }
    }
};

class Action {
public:
    shared_ptr<Position> from; // current row, column of the unit to be moved
    shared_ptr<Position> to; // position to where the unit must be moved
};

auto chooseSymbolRandomly(const vector<pair<int, int>>& set)
{
    return set[rand() % set.size()];
}

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
            action.from = make_shared<Position>(Position(row, column));
            action.to = make_shared<Position>(Position(row + 1, column)); // increase row
            cout << "coords: " << row << " " << column;
            cout << " to " << row + 1 << " " << column << endl;


            successfulMove = true;
        }
        else if (column + 1 != gridSideSize
                    && *world.board[row][column + 1] != Symbols::M
                    && *world.board[row][column + 1] != Symbols::r
                    && *world.board[row][column + 1] != Symbols::p
                    && *world.board[row][column + 1] != Symbols::s
                    && *world.board[row][column + 1] != Symbols::f)
        {
            action.from = make_shared<Position>(Position(row, column));
            action.to = make_shared<Position>(Position(row, column + 1));
            cout << "coords: " << row << " " << column;
            cout << " to " << row << " " << column + 1 << endl;

            successfulMove = true;
        }

        if (count++ == 100) {
            cout << "can't move :(" << endl;
            exit(0);
        }
    }

    return action;

}

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
        if (destRow >= 0 && destRow < gridSideSize && destColumn >= 0 && destColumn < gridSideSize
                && *world.board[destRow][destColumn] != Symbols::M
                && *world.board[destRow][destColumn] != Symbols::S
                && *world.board[destRow][destColumn] != Symbols::R
                && *world.board[destRow][destColumn] != Symbols::P
                && *world.board[destRow][destColumn] != Symbols::F)
        {
            action.from = make_shared<Position>(Position(row, column));
            action.to = make_shared<Position>(Position(destRow, destColumn));
            cout << "coords: " << row << " " << column;
            cout << " to " << destRow << " " << destColumn << endl;
            successfulMove = true;
        }

        if (count++ == 100) {
            cout << "can't move :(" << endl;
            exit(0);
        }
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
        return {action, true}; // player failed to answer in less than 400 ms
    else return {action, false};
}

std::tuple<bool, string> validateActions(const World& world, const Action& action0, const Action& action1)
{
    auto player0Dest = *action0.to;
    auto player1Dest = *action1.to;

    if (*world.board[player0Dest.getRow()][player0Dest.getColumn()] == Symbols::M
        || action0.to == action0.from
        || player0Dest.getRow() < 0 || player0Dest.getRow() >= gridSideSize
        || player0Dest.getColumn() < 0 || player0Dest.getColumn() >= gridSideSize)
    {
        return { true, "Player 0 made an illegal move. Player 1 won the game!"};
    }
    else if (*world.board[player1Dest.getRow()][player1Dest.getColumn()] == Symbols::M
             || action1.to == action1.from
             || player1Dest.getRow() < 0 || player1Dest.getRow() >= gridSideSize
             || player1Dest.getColumn() < 0 || player1Dest.getColumn() >= gridSideSize)
    {
        return { true, "Player 1 made an illegal move. Player 0 won the game!"};
    }
    else if (*world.board[player0Dest.getRow()][player0Dest.getColumn()] == Symbols::F)
    {
        return { true, "Player 0 captured the flag! Hooray!" };
    }
    else if (*world.board[player1Dest.getRow()][player1Dest.getColumn()] == Symbols::f)
    {
        return { true, "Player 1 captured the flag! Hooray!" };
    }
    else
    {
        return { false, "Nothing interesting yet" };
    }
}

int interaction(const Symbols& p0, const Symbols& p1)
{
    if (p0 == Symbols::s && p1 == Symbols::S
            || p0 == Symbols::r && p1 == Symbols::R
            || p0 == Symbols::p && p1 == Symbols::P
//            || p1 == Symbols::s && p0 == Symbols::S
//               || p1 == Symbols::r && p0 == Symbols::R
//               || p1 == Symbols::p && p0 == Symbols::P
               )
    {
        return 0;
    }
    else if (p0 == Symbols::s && p1 == Symbols::P
            || p0 == Symbols::r && p1 == Symbols::S
            || p0 == Symbols::p && p1 == Symbols::R)
    {
        return 1;
    }
    else if (p0 == Symbols::p && p1 == Symbols::S
             || p0 == Symbols::s && p1 == Symbols::R
             || p0 == Symbols::r && p1 == Symbols::P)
    {
        return 2;
    }

    return -1;
}

void handleInteraction(int code, World& world, Position& pos0, Position& pos0to, Position& pos1, Position& pos1to)
{
    switch (code) {
        case 0: // if two similar symbols met
        {
            // do nothing
            break;
        }
        case 1: // when player 0 kills player 1
        {
            cout << "killed: " << pos1.getRow() << " " << pos1.getColumn() << endl;

            world.board[pos1.getRow()][pos1.getColumn()] = make_shared<Symbols>(Symbols::empty); // just empty the killed symbol's cell
            auto it = find (world.set1.begin(), world.set1.end(), make_pair( pos1.getRow(), pos1.getColumn() ));
            if (it == world.set1.end()) {
                cout << "first not found at: " << pos0.getRow() << " " << pos0.getColumn() << endl;
                exit(0);
                break;
            }
            world.set1.erase(it);

            auto tmp = world.board[pos0.getRow()][pos0.getColumn()];
            world.board[pos0.getRow()][pos0.getColumn()] = make_shared<Symbols>(Symbols::empty);
            world.board[pos0to.getRow()][pos0to.getColumn()] = tmp;
            it = find (world.set0.begin(), world.set0.end(), make_pair( pos0.getRow(), pos0.getColumn() ));
            if (it == world.set0.end()) {
                cout << "other zero not found" << endl;
                exit(0);
                break;
            }
            it->first = pos0to.getRow();
            it->second = pos0to.getColumn();

            break;
        }
        case 2: // when player 1 kills player 0
        {
            cout << "killed: " << pos0.getRow() << " " << pos0.getColumn() << endl;
            world.board[pos0.getRow()][pos0.getColumn()] = make_shared<Symbols>(Symbols::empty); // just empty the killed symbol's cell
            auto it = find (world.set0.begin(), world.set0.end(), make_pair( pos0.getRow(), pos0.getColumn() ));
            if (it == world.set0.end()) {
                cout << "second not found at: " << pos0.getRow() << pos0.getColumn() << endl;
                exit(0);
                break;
            }
            world.set0.erase(it);
            cout << "erased: " << it->first << " " << it->second << endl;

            auto tmp = world.board[pos1.getRow()][pos1.getColumn()];
            world.board[pos1.getRow()][pos1.getColumn()] = make_shared<Symbols>(Symbols::empty);
            world.board[pos1to.getRow()][pos1to.getColumn()] = tmp;
            it = find (world.set1.begin(), world.set1.end(), make_pair( pos1.getRow(), pos1.getColumn() ));
            if (it == world.set1.end()) {
                cout << "other second not found" << endl;
                exit(0);
                break;
            }
            it->first = pos1to.getRow();
            it->second = pos1to.getColumn();

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

void updateWorld(World& world, Action& action0, Action& action1)
{
    auto playerMove = [](World& world, Action& action, vector<pair<int, int>>& set)
    {
        world.board[action.to->getRow()][action.to->getColumn()] = move(world.board[action.from->getRow()][action.from->getColumn()]);
        world.board[action.from->getRow()][action.from->getColumn()] = make_shared<Symbols>(Symbols::empty);
        auto it = find (set.begin(), set.end(), make_pair( action.from->getRow(), action.from->getColumn() ));
        if (it == set.end())
        {
            cout << "move 0 not found" << endl;
            exit(0);
        }
        it->first = action.to->getRow();
        it->second = action.to->getColumn();
    };

    if (*action0.to == *action1.from && *action0.from == *action1.to)
    {
        swap(world.board[action0.to->getRow()][action0.to->getColumn()], world.board[action1.to->getRow()][action1.to->getColumn()]);
        auto it0 = find (world.set0.begin(), world.set0.end(), make_pair( action0.from->getRow(), action0.from->getColumn() ));
        auto it1 = find (world.set1.begin(), world.set1.end(), make_pair( action1.from->getRow(), action1.from->getColumn() ));
        swap(*it0, *it1);

        return; // if they just swap - do nothing
    }
    else if (*action0.to == *action1.from)
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
    else if (*action1.to == *action0.from)
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
    else if (*action0.to == *action1.to)
    {
        int code = interaction(
                *world.board[action0.from->getRow()][action0.from->getColumn()],
                *world.board[action1.from->getRow()][action1.from->getColumn()]
        );
        cout << "case 3" << endl; // TODO check it if problems
        handleInteraction(code, world, *action0.from, *action0.to, *action1.from, *action1.to);
    }
    else
    {
        {
            int code = interaction(
                    *world.board[action0.from->getRow()][action0.from->getColumn()],
                    *world.board[action0.to->getRow()][action0.to->getColumn()]
            );
            cout << "case 1" << endl;
            cout << action0.from->getRow() << " " << action0.from->getColumn() << " to " << action0.to->getRow() << " " << action0.to->getColumn() << endl;
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
            cout << action1.from->getRow() << " " << action1.from->getColumn() << endl;
            cout << action1.to->getRow() << " " << action1.to->getColumn() << endl;
            cout << "case 2" << endl;
            if (code == -1)
                playerMove(world, action1, world.set1);
            else
                handleInteraction(code, world, *action1.to, *action1.to, *action1.from, *action1.to);
        }
    }
}

int main() {
    srand ( time(NULL) );
    while (true)
    {
        World world;
        world.init();
        world.show();
        bool endGame = false;
        while (!endGame)
        {
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
                world.show();
                for (const auto& tmp : world.set0)
                    cout << tmp.first << " " << tmp.second << "|";
                cout << endl;
                for (const auto& tmp : world.set1)
                    cout << tmp.first << " " << tmp.second << "|";
                cout << endl;

                if (endGame)
                {
                    cout << message << endl;
                }
            }
        }
    }



    return 0;
}