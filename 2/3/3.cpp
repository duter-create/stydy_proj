#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    int n;
    std::cin >> n;
    std::vector<long long> b(n - 1), c(n - 1), a(n);

    // 读入 b 数组
    for (int i = 0; i < n - 1; ++i) {
        std::cin >> b[i];
    }

    // 读入 c 数组
    for (int i = 0; i < n - 1; ++i) {
        std::cin >> c[i];
    }

    // 对 b 和 c数组排序
    std::sort(b.begin(), b.end());
    std::sort(c.begin(), c.end());

    // 找出缺失的两个值
    long long sum_b = 0, sum_c = 0;
    for (int i = 0; i < n - 2; ++i) {
        sum_b += b[i];
        sum_c += c[i];
    }

    // 最后剩下的两位可能是最后一个元素，也可能是被删除的元素。
    // 找出较小的那个，其他元素即为 a[i] = b[i] 或 c[i] - sum_b 或 sum_c
    long long last_b = b[n - 2];
    long long last_c = c[n - 2];
    long long total_sum = sum_b + sum_c + last_b + last_c;
    long long a_last = (total_sum + last_b - last_c) / 2;
    long long a_last_minus_one = (total_sum - a_last) / 2;

    // 还原数组 a 的元素值
    a[n - 1] = a_last;
    a[n - 2] = std::max(last_b, last_c) - a_last_minus_one;
    for (int i = n - 3; i >= 0; --i) {
        a[i] = b[i] - b[i + 1] + a[i + 1];
    }

    // 打印结果
    for (auto value : a) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    return 0;
}