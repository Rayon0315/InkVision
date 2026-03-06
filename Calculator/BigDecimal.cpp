#include <iostream>
#include <stdexcept>
#include "BigDecimal.h"

BigDecimal::BigDecimal()
    : head(nullptr), tail(nullptr), sgn(1), scale(0) {}

BigDecimal::~BigDecimal() {
    NodePtr cur = head;
    while (cur) {
        NodePtr tmp = cur;
        cur = cur->next;
        delete tmp;
    }
    head = tail = nullptr;
}

BigDecimal::BigDecimal(const BigDecimal& other)
    : head(nullptr), tail(nullptr), sgn(other.sgn), scale(other.scale) {
    for (NodePtr cur = other.head; cur; cur = cur->next) {
        this->pushBack(cur->value);
    }
}

BigDecimal& BigDecimal::operator = (const BigDecimal& other) {
    if (this == &other) return *this;

    while (head) popFront();

    sgn = other.sgn;
    scale = other.scale;

    for (NodePtr cur = other.head; cur; cur = cur->next) {
        this->pushBack(cur->value);
    }

    return *this;
}

void BigDecimal::popFront() {
    if (!head) {
        throw std::logic_error("popFront on empty BigDecimal");
    }

    if (head == tail) {
        delete head;
        head = tail = nullptr;
        return;
    }

    NodePtr tmp = head;
    head = head->next;
    head->prev = nullptr;
    delete tmp;
}

void BigDecimal::popBack() {
    if (!tail) {
        throw std::logic_error("popBack on empty BigDecimal");
    }

    if (head == tail) {
        delete tail;
        head = tail = nullptr;
        return;
    }

    NodePtr tmp = tail;
    tail = tail->prev;
    tail->next = nullptr;
    delete tmp;
}

void BigDecimal::pushFront(int value) {
    if (!head) {
        head = new Node(value);
        tail = head;
        return;
    }

    NodePtr ins = new Node(value);
    head->prev = ins;
    ins->next = head;
    head = ins;
}

void BigDecimal::pushBack(int value) {
    if (!head) {
        head = new Node(value);
        tail = head;
        return;
    }

    NodePtr ins = new Node(value);
    tail->next = ins;
    ins->prev = tail;
    tail = ins;
}

BigDecimal BigDecimal::stringToBigDecimal(std::string inp) {
    BigDecimal ret;
    for (int i = inp.size() - 1; i >= 0; i--) {
        ret.pushBack(inp[i] - '0');
    }
    return ret;
}

void BigDecimal::debugPrint() {

    std::cout << "sgn::::" << sgn << std::endl;
    std::cout << "scale::::" << scale << std::endl;

    NodePtr cur = tail;
    while (cur) {
        std::cout << cur->value;
        cur = cur->prev;
    }
    std::cout << std::endl;
}

void BigDecimal::removeFrontZero() {
    while (tail && tail->value == 0 && head != tail) {
        this->popBack();
    }
}

void BigDecimal::removeBackZero() {
    while (scale > 0 && head && head->value == 0) {
        this->popFront();
        scale--;
    }
}

void BigDecimal::normalize() {
    removeBackZero();
    removeFrontZero();
}

int BigDecimal::getLength(const BigDecimal &X) {
    int cnt = 0;
    NodePtr cur = X.head;
    while (cur) {
        cnt++;
        cur = cur->next;
    }
    return cnt;
}

void BigDecimal::toTargetLength(int target) {
    int cur = getLength(*this);

    if (cur > target) {
        throw std::logic_error("target length smaller than current length");
    }

    while (cur < target) {
        this->pushBack();
        cur++;
    }
}

void BigDecimal::align(BigDecimal &L, BigDecimal &R) {
    if (L.scale < R.scale) {
        while (L.scale < R.scale) {
            L.pushFront();
            L.scale++;
        }
    }

    if (L.scale > R.scale) {
        while (L.scale > R.scale) {
            R.pushFront();
            R.scale++;
        }
    }

    int lenL = getLength(L);
    int lenR = getLength(R);

    if (lenL < lenR) {
        while (lenL < lenR) {
            L.pushBack();
            lenL++;
        }
    }

    if (lenL > lenR) {
        while (lenL > lenR) {
            R.pushBack();
            lenR++;
        }
    }
}

void BigDecimal::mySwap(BigDecimal &L, BigDecimal &R) {
    BigDecimal tmp = L;
    L = R;
    R = tmp;
}

bool BigDecimal::absGeq(BigDecimal L, BigDecimal R) {
    align(L, R);

    int lenL = getLength(L);
    int lenR = getLength(R);

    if (lenL < lenR) {
        return false;
    }

    if (lenL > lenR) {
        return true;
    }

    NodePtr curL = L.tail;
    NodePtr curR = R.tail;
    while (curL && curR) {
        if (curL->value != curR->value) {
            return curL->value > curR->value;
        }
        curL = curL->prev;
        curR = curR->prev;
    }

    return true;
}

// 加法的实现让长度完全对齐，从而不需要考虑动态加点
BigDecimal BigDecimal::absAdd(BigDecimal L, BigDecimal R) {
    align(L, R);

    int len = getLength(L);

    L.toTargetLength(len + 1);
    R.toTargetLength(len + 1);

    BigDecimal ret;
    ret.toTargetLength(len + 1);
    ret.sgn = 1;
    ret.scale = L.scale; // L R 此时的scale相同

    NodePtr p = L.head, q = R.head;
    NodePtr cur = ret.head;

    int carry = 0;
    while (cur && p && q) {
        int value = p->value + q->value + carry;

        cur->value = value % 10;
        carry = value / 10;

        p = p->next;
        q = q->next;

        cur = cur->next;
    }

    ret.normalize();

    return ret;
}

// 默认 L >= R
BigDecimal BigDecimal::absSub(BigDecimal L, BigDecimal R) {
    align(L, R);

    BigDecimal ret;
    ret.sgn = 1;
    ret.scale = L.scale;

    int len = getLength(L);
    ret.toTargetLength(len);

    NodePtr p = L.head, q = R.head;
    NodePtr cur = ret.head;

    int borrow = 0;
    while (cur && p && q) {
        int diff = p->value - q->value - borrow;

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }

        cur->value = diff;

        p = p->next;
        q = q->next;

        cur = cur->next;
    }

    ret.normalize();
    return ret;
}

void BigDecimal::debugAdd(BigDecimal &L, BigDecimal &R) {
    absAdd(L, R).debugPrint();
}
void BigDecimal::debugSub(BigDecimal &L, BigDecimal &R) {
    absSub(L, R).debugPrint();
}

BigDecimal BigDecimal::convertFromString(std::string inp) {
    BigDecimal ret;

    ret.sgn = 1;
    if (inp[0] == '-') ret.sgn = -1;

    bool dotFinded = false;
    for (auto c : inp) {
        if (c == '.') {
            dotFinded = true;
            break;
        }
    }

    ret.scale = 0;
    if (dotFinded)
        for (int i = inp.size() - 1; i >= 0; i--) {
            if (inp[i] == '.') break;
            ret.scale++;
        }

    for (int i = inp.size() - 1; i >= 0; i--) {
        if (inp[i] >= '0' && inp[i] <= '9') {
            ret.pushBack(inp[i] - '0');
        }
    }

    ret.normalize();
    return ret;
}

std::string BigDecimal::convertToString() {
    this->normalize();

    if (head && head == tail && head->value == 0) {
        return "0";
    }

    std::string out = "";

    if (sgn < 0) out += '-';

    int totLen = getLength(*this);
    int intLen = totLen - scale;

    if (intLen <= 0) {
        out += "0.";
        for (int i = 0; i < -intLen; i++) {
            out += "0";
        }
    }

    NodePtr cur = tail;
    int cnt = 0;
    while (cur) {
        out += cur->value + '0';
        cnt++;

        if (cnt == intLen && scale > 0) out += '.';

        if (cnt < intLen && (intLen - cnt) % 3 == 0) out += ',';

        cur = cur->prev;
    }

    return out;
}

BigDecimal BigDecimal::operator + (const BigDecimal &other) {
    BigDecimal ret;

    if (this->sgn == other.sgn) {
        ret = absAdd(*this, other);
        ret.sgn = this->sgn;
        return ret;
    }

    if (absGeq(*this, other)) {
        ret = absSub(*this, other);
        ret.sgn = this->sgn;
    } else {
        ret = absSub(other, *this);
        ret.sgn = other.sgn;
    }

    return ret;
}

BigDecimal BigDecimal::operator - (const BigDecimal &other) {
    BigDecimal neg = other;
    neg.sgn = -neg.sgn;
    return *this + neg;
}

BigDecimal BigDecimal::mulByDigit(int d) {
    BigDecimal ret;
    ret.sgn = 1;
    ret.scale = 0;

    int carry = 0;
    for (NodePtr cur = this->head; cur; cur = cur->next) {
        int value = cur->value * d + carry;

        ret.pushBack(value % 10);
        carry = value / 10;
    }

    if (carry) ret.pushBack(carry);

    return ret;
}

BigDecimal BigDecimal::operator * (const BigDecimal &other) {
    BigDecimal A = *this;
    BigDecimal B = other;

    BigDecimal ret;
    int retSgn = A.sgn * B.sgn;
    int retScale = A.scale + B.scale;

    A.sgn = B.sgn = 1;
    A.scale = B.scale = 0;

    int shift = 0;
    for (NodePtr cur = B.head; cur; cur = cur->next) {
        BigDecimal tmp = A.mulByDigit(cur->value);

        for (int i = 0; i < shift; i++)
            tmp.pushFront();

        ret = absAdd(ret, tmp);
        shift++;
    }

    ret.sgn = retSgn;
    ret.scale = retScale;
    ret.normalize();
    return ret;
}

BigDecimal BigDecimal::divInt(BigDecimal A, BigDecimal B) {
    BigDecimal quotient;
    BigDecimal remainder;

    quotient.sgn = 1;
    quotient.scale = 0;

    remainder.sgn = 1;
    remainder.scale = 0;

    for (NodePtr cur = A.tail; cur; cur = cur->prev) {
        remainder.pushFront(cur->value);

        int d = 0;
        for (int k = 9; k >= 0; k--) {
            BigDecimal tmp = B.mulByDigit(k);

            if (absGeq(remainder, tmp)) {
                d = k;
                remainder = absSub(remainder, tmp);
                break;
            }
        }

        quotient.pushFront(d);
    }

    return quotient;
}

BigDecimal BigDecimal::operator/(const BigDecimal& other) {
    BigDecimal A = *this;
    BigDecimal B = other;

    int retSgn = A.sgn * B.sgn;
    A.sgn = B.sgn = 1;

    constexpr int TARGET_SCALE = 10;
    constexpr int WORK_SCALE = TARGET_SCALE + 1;

    int shift = WORK_SCALE + B.scale - A.scale;
    if (shift > 0) {
        for (int i = 0; i < shift; ++i)
            A.pushFront();
    }

    A.scale = B.scale = 0;

    BigDecimal Q = divInt(A, B);

    if (Q.head && Q.head->value >= 5) {
        BigDecimal inc = convertFromString("1");
        inc.pushFront();
        Q = absAdd(Q, inc);
    }

    Q.popFront();
    Q.scale = TARGET_SCALE;
    Q.sgn = retSgn;
    Q.normalize();

    return Q;
}
