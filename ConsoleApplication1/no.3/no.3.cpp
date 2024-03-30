#include <iostream>
#include <vector>

using namespace std;

int main() {
    int p, n, m;
    cin >> p >> n >> m;

    // 队列存储怪物房间的怪物的战斗力
    vector<int> monsters(n);
    for (int i = 0; i < n; ++i) {
        cin >> monsters[i];
    }

    // 存储每个boss的战斗力
    vector<int> bosses(m);
    for (int i = 0; i < m; ++i) {
        cin >> bosses[i];
    }

    // 对于每个boss，计算玩家需要进入的房间数量
    for (int i = 0; i < m; ++i) {
        int currentPower = p;
        int roomCount = 0;
        int index = 0;

        // 当战斗力不足以打败boss时，尝试战斗或进入奖励房间
        while (currentPower <= bosses[i]) {
            // 如果还有怪物房间，并且当前战斗力大于等于怪物，则进入
            if (index < n&& currentPower >= monsters[index]) {
                currentPower += monsters[index];
                ++index; // 进入了一个怪物房
            }
            else {
                // 如果战斗力不够且已无怪物房间，只能进入奖励房间
                int increment = currentPower / 10;
                // 确保每次进入奖励房间至少增加1点战斗力，直到战斗力大于等于boss
                currentPower += increment > 0 ? increment : 1;
            }
            roomCount++; // 无论进入怪物房还是奖励房，房间数+1
        }

        // 输出玩家进入房间的总数
        cout << roomCount << endl;
    }

    return 0;
}