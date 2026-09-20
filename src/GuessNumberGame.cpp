#include <iostream>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <cctype>
#include <string>
#include <thread>
#include <chrono>
using namespace std;

// 颜色宏
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// 清屏
void clearScreen() {
    cout << "\033[2J\033[H";
}

// 延时（毫秒）
void sleepMs(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

class GuessNumberGame {
public:
    void run() {
        srand(static_cast<unsigned>(time(0)));
        clearScreen();
        cout << BOLD << CYAN << "========================================\n";
        cout << "         猜 数 字 游 戏\n";
        cout << "========================================\n" << RESET;
        cout << "想玩一局吗？(y/n): ";
        string t;
        cin >> t;
        clearInput();

        if (t == "y" || t == "Y") {
            bool flag = true;
            while (flag) {
                bool valid = false;
                while (!valid) {
                    cout << "\n选择难度：" << GREEN << "easy" << RESET
                         << " / " << YELLOW << "medium" << RESET
                         << " / " << RED << "hard" << RESET
                         << " / sd（自定义）：";
                    cin >> t;
                    clearInput();
                    valid = setDiff(t);
                }
                playOneRound();
                flag = askAgain();
            }
        }
        cout << "\n" << CYAN << "再见！" << RESET << endl;
    }

private:
    int maxnum = 500;
    int maxtry = 10;
    int target = 0;
    int tries = 0;

    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void setNandT() {
        cout << "输入最大数字和最大次数：";
        if(!(cin >> maxnum >> maxtry)||maxnum<1||maxtry<1){
            cout<<"输入数据不合理，使用默认数据，maxnum = 500,maxtry = 10"<<endl;
            maxnum = 500;
            maxtry = 10;
        };
        clearInput();
    }

    bool setDiff(string s) {
        for (char &c : s) c = tolower(static_cast<unsigned char>(c));
        if (s == "e" || s == "easy") {
            maxnum = 50;  maxtry = 10;
        } else if (s == "m" || s == "medium") {
            maxnum = 100; maxtry = 7;
        } else if (s == "h" || s == "hard") {
            maxnum = 500; maxtry = 6;
        } else if (s == "sd") {
            setNandT();
        } else {
            cout << RED << "没有这个模式！" << RESET << endl;
            return false;
        }
        return true;
    }

    void playOneRound() {
        tries = 0;
        target = rand() % maxnum + 1;
        bool win = false;

        clearScreen();
        cout << BOLD << "游戏开始！" << RESET
             << " 范围 " << GREEN << "1 ~ " << maxnum << RESET
             << "，共 " << YELLOW << maxtry << RESET << " 次机会\n\n";

        while (tries < maxtry) {
            // 进度条
            cout << "机会：";
            for (int i = 0; i < maxtry; ++i) {
                if (i < maxtry - tries) cout << GREEN << "●" << RESET;
                else cout << RED << "○" << RESET;
            }
            cout << "  剩余 " << maxtry - tries << " 次\n";

            cout << "请输入你的猜测：";
            int n;
            cin >> n;
            clearInput();

            if (n < 1 || n > maxnum) {
                cout << RED << "超出范围！请输入 1~" << maxnum << RESET << "\n\n";
                continue;
            }

            ++tries;

            if (n < target) {
                cout << YELLOW << "太小了！" << RESET;
                if (target - n <= 5) cout << "（很接近了）";
                cout << "\n\n";
            } else if (n > target) {
                cout << YELLOW << "太大了！" << RESET;
                if (n - target <= 5) cout << "（很接近了）";
                cout << "\n\n";
            } else {
                cout << "\n" << GREEN << BOLD
                     << "🎉 恭喜！你用 " << tries << " 次猜中了 " << target << "！"
                     << RESET << "\n\n";
                win = true;
                sleepMs(1500);
                break;
            }
        }

        if (!win) {
            cout << RED << BOLD
                 << "😢 机会用完了，正确答案是 " << target
                 << RESET << "\n\n";
            sleepMs(1500);
        }
    }

    bool askAgain() {
        cout << "再来一局？(y/n): ";
        string s;
        cin >> s;
        clearInput();
        return (s == "y" || s == "Y");
    }
};

int main() {
    GuessNumberGame g;
    g.run();
    return 0;
}