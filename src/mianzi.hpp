#ifndef PYMAHJONG_MIANZI_HPP
#define PYMAHJONG_MIANZI_HPP

struct Mianzi {
    enum Type : uint8_t {
        shunzi, kezi, duizi
    } type;

    enum FuluType : uint8_t {
        none, chi, peng, minggang, angang
    } fulu_type = none;

    int pai_34;

    Mianzi(Type type, int pai) : type(type), pai_34(pai) {}
    Mianzi(FuluType fulu_type, int pai) : type(static_cast<Type>(fulu_type != chi)), fulu_type(fulu_type), pai_34(pai) {}

    std::string to_string() const {
        std::string s;
        int n = pai_34 % 9 + 1;
        switch (type) {
        case shunzi:
            s += fulu_type == none ? "shunzi: " : "chi: ";
            s += "mpsz"[pai_34 / 9];
            s += std::to_string(n);
            s += std::to_string(n + 1);
            s += std::to_string(n + 2);
            break;
        case kezi:
            switch (fulu_type) {
        case none:
            s += "kezi: ";
                break;
        case peng:
            s += "peng: ";
                break;
        case minggang:
            s += "minggang: ";
                break;
        case angang:
            s += "angang: ";
                break;
        default:
            __builtin_unreachable();
            }
            s += "mpsz"[pai_34 / 9];
            s += std::to_string(n);
            break;
        case duizi:
            s += "duizi: ";
            s += "mpsz"[pai_34 / 9];
            s += std::to_string(n);
            break;
        }
        return s;
    }
};

#endif //PYMAHJONG_MIANZI_HPP
