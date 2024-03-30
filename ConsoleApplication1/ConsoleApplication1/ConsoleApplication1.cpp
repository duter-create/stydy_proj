#include <iostream>
#include <vector>

int main() {
    int N;
    std::cin >> N;

    // 初始化为0，分别对应1年、3年、6年、10年积木需求量
    std::vector<int> blocksNeeded(4, 0);

    for (int i = 0; i < N; i++) {
        int year;
        std::cin >> year;

        // 如果员工达到了10年司龄，确认他们之前已领取过所有积木
        if (year == 10) {
            blocksNeeded[3]++;
            year -= 10;
        }

        // 如果员工达到或超过了6年司龄，在他们6年时应该已经领取过6周年积木
        if (year >= 6) {
            blocksNeeded[2]++;
            year -= 6;
        }

        // 如果员工达到或超过了3年司龄，在他们3年时应该已经领取过3周年积木
        if (year >= 3) {
            blocksNeeded[1]++;
            year -= 3;
        }

        // 每一位员工至少应该领取一次1周年积木
        blocksNeeded[0]++;
    }

    // 输出所需的积木数量
    for (int block : blocksNeeded) {
        std::cout << block << ' ';
    }
    std::cout << std::endl;
    return 0;
}