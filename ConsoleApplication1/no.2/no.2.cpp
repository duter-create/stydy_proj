#include <iostream>
#include <vector>

int main() {
    int N;
    std::cin >> N;

    // 分别对应一周年、三周年、六周年、十周年积木的数量
    std::vector<int> blocksNeeded(4, 0);

    for (int i = 0; i < N; ++i) {
        int seniority;
        std::cin >> seniority;

        // 判断并计算各个积木的需要量
        if (seniority >= 1) blocksNeeded[0]++; // 一周年
        if (seniority >= 3) blocksNeeded[1]++; // 三周年
        if (seniority >= 6) blocksNeeded[2]++; // 六周年
        if (seniority >= 10) blocksNeeded[3]++; // 十周年
    }

    // 输出每款积木所需的数量
    for (int i = 0; i < 4; ++i) {
        std::cout << blocksNeeded[i] << (i < 3 ? " " : "\n");
    }

    return 0;
}