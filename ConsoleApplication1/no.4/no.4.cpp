#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

struct Player {
    int id;
    int teamId;
    pair<int, int> position;
    string status;
    bool hasSoulflower;
    int shadeValue;
};

struct Team {
    int teamId;
    vector<Player> players;
    bool eliminated;
};

map<int, Team> teams;

void processKO(int A, int B) {
    // 检查击倒规则
    auto& playerA = teams[A / 3].players[A % 3];
    auto& playerB = teams[B / 3].players[B % 3];

    if (playerB.hasSoulflower) {
        playerB.hasSoulflower = false; // 失去返魂花
        playerB.status = "Ghost"; // 进入幽灵状态
    }
    else {
        playerB.status = "Downed"; // 倒地
    }
}

void processUP(int A, int B) {
    // A玩家拉起B玩家
    auto& playerB = teams[B / 3].players[B % 3];
    playerB.status = "Alive"; // 玩家B进入存活状态
    playerB.shadeValue = 0; // 清除暗域值
}

void processRespawn(int A, int X, int Y) {
    // A玩家在X,Y重生
    auto& playerA = teams[A / 3].players[A % 3];
    playerA.status = "Alive"; // 重生为存活状态
    playerA.position = make_pair(X, Y); // 更新位置
}

void processMove(int A, int X, int Y) {
    // A玩家移动到X,Y
    auto& playerA = teams[A / 3].players[A % 3];
    playerA.position = make_pair(X, Y); // 更新位置
}

void processShrink() {
    // 暗域缩小
}

void processQuit(int A) {
    // A队伍退出
    for (auto& player : teams[A].players) {
        player.status = "Downed"; // 队伍中的玩家全部倒地
    }
    teams[A].eliminated = true;
}

vector<int> getDowned() {
    // 获取所有倒地的玩家id
    vector<int> downed;
    for (auto& team : teams) {
        for (auto& player : team.second.players) {
            if (player.status == "Downed") {
                downed.push_back(player.id);
            }
        }
    }

    sort(downed.begin(), downed.end());
    return downed;
}

vector<int> getEliminated() {
    // 获取所有淘汰的队伍id
    vector<int> eliminated;
    for (auto& team : teams) {
        if (team.second.eliminated) {
            eliminated.push_back(team.first);
        }
    }

    sort(eliminated.begin(), eliminated.end());
    return eliminated;
}

int main() {
    int n, m;

    cin >> n;
    for (int i = 0; i < n; i++) {
        Team team;
        team.eliminated = false;
        for (int j = 0; j < 3; j++) {
            Player player;
            int id;
            cin >> id;
            player.id = id;
            player.teamId = i;
            player.hasSoulflower = true;
            player.shadeValue = 0;
            player.status = "Alive";
            team.players.push_back(player);
        }
        int x, y;
        cin >> x >> y;
        for (Player& p : team.players) {
            p.position = make_pair(x, y);
        }
        teams[i] = team;
    }

    cin >> m;
    for (int i = 0; i < m; i++) {
        string event;
        cin >> event;
        if (event == "KO") {
            int A, B;
            cin >> A >> B;
            processKO(A, B);
        }
        else if (event == "UP") {
            int A, B;
            cin >> A >> B;
            processUP(A, B);
        }
        else if (event == "Respawn") {
            int A, X, Y;
            cin >> A >> X >> Y;
            processRespawn(A, X, Y);
        }
        else if (event == "Move") {
            int A, X, Y;
            cin >> A >> X >> Y;
            processMove(A, X, Y);
        }
        else if (event == "Shrink") {
            processShrink();
        }
        else if (event == "Quit") {
            int A;
            cin >> A;
            processQuit(A);
        }

        vector<int> downed = getDowned();
        if (!downed.empty()) {
            cout << "Fall";
            for (int id : downed) {
                cout << " " << id;
            }
            cout << endl;
        }

        vector<int> eliminated = getEliminated();
        if (!eliminated.empty()) {
            cout << "Exit";
            for (int id : eliminated) {
                cout << " " << id;
            }
            cout << endl;
        }

        int a = 0, b = 0;
        for (auto& team : teams) {
            if (!team.second.eliminated) {
                b++;
                for (auto& player : team.second.players) {
                    if (player.status == "Alive")
                        a++;
                }
            }
        }
        cout << a << " " << b << endl;

        if (b <= 1) {
            for (auto& team : teams) {
                if (!team.second.eliminated) {
                    cout << "Winner " << team.first << endl;
                    return 0;
                }
            }
        }
    }

    return 0;
}