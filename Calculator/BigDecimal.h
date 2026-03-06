#pragma once

#include <string>

class BigDecimal {
private:
    struct Node {
        int value;

        Node* next;
        Node* prev;

        Node (int VALUE) : value(VALUE), next(nullptr), prev(nullptr) {}
    };

    typedef Node* NodePtr;

    NodePtr head, tail;

    int sgn; // 符号，存储正数/负数
    int scale;
    /*
        供小数使用，X = A * 10^scale
        同时整数中scale = 0
        可以一开始就以小数为主体设计
    */

public:

    static int getLength(const BigDecimal &X);
    static void align(BigDecimal &L, BigDecimal &R);
    static bool absGeq(BigDecimal L, BigDecimal R);
    static void mySwap(BigDecimal &L, BigDecimal &R);
    static BigDecimal absAdd(BigDecimal L, BigDecimal R);
    static BigDecimal absSub(BigDecimal L, BigDecimal R);


    void toTargetLength(int target);

    BigDecimal();
    ~BigDecimal();

    // 保证拷贝是深拷贝，防止内存管理出问题
    BigDecimal(const BigDecimal& other);
    BigDecimal& operator = (const BigDecimal& other);

    void popFront();
    void popBack();

    void pushFront(int value = 0);
    void pushBack(int value = 0);

    static BigDecimal stringToBigDecimal(std::string inp);
    void debugPrint();
    static void debugAdd(BigDecimal &L, BigDecimal &R);
    static void debugSub(BigDecimal &L, BigDecimal &R);

    void removeFrontZero(); // 前导零：从tail开始删
    void removeBackZero(); //后导零：从head开始删，并且需要考虑scale
    void normalize();

    static BigDecimal convertFromString(std::string inp);
    std::string convertToString();

    BigDecimal operator + (const BigDecimal &other);
    BigDecimal operator - (const BigDecimal &other);

    BigDecimal mulByDigit(int d);
    BigDecimal operator * (const BigDecimal &other);

    BigDecimal divInt(BigDecimal A, BigDecimal B);
    BigDecimal operator / (const BigDecimal &other);
};
