#include <iostream>
#include <string>
#include <sstream>
#include <limits>
using namespace std;

int main() {
    int n;
    cin >> n; // 读取复数的数量

    // 跳过当前行剩余的输入并准备读取下一行
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string line;
    getline(cin, line); // 读取包含所有复数的行

    stringstream ss(line);
    string complex_number;
    int real_count = 0;

    while (ss >> complex_number) { // 逐个提取复数
        // 检查字符串是否包含 'i' 字符
        size_t found_i = complex_number.find('i');
        if (found_i != string::npos) {
            // 如果找到 'i'，检查 'i' 前是否存在数字并且是否为 '0'
            // 确保 '+' 或 '-' 后面至少有一个数字
            if ((found_i > 1) && (complex_number[found_i - 1] == '0') &&
                ((complex_number[found_i - 2] >= '0' && complex_number[found_i - 2] <= '9') ||
                    (found_i == 2 && (complex_number[0] == '+' || complex_number[0] == '-')) ||
                    (found_i > 2 && (complex_number[found_i - 3] == '+' || complex_number[found_i - 3] == '-')))) {
                real_count++;
            }
        }
        else {
            // 没有 'i' 字符，这是一个实数
            real_count++;
        }
    }

    cout << real_count << endl; // 输出实数的数量
    return 0;
}