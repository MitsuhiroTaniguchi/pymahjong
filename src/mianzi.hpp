#ifndef PYMAHJONG_MIANZI_HPP
#define PYMAHJONG_MIANZI_HPP

struct Mianzi {
    enum Type : uint8_t {
        shunzi, kezi, duizi
    } type;

    enum FuluType : uint8_t {
        none, chi, peng, minggang, angang
    } fulu_type{};

    int pai_34;

    Mianzi(Type type, int pai) : type(type), pai_34(pai) {}
    Mianzi(FuluType fulu_type, int pai) : type(static_cast<Type>(fulu_type != chi)), fulu_type(fulu_type), pai_34(pai) {}
};

#endif //PYMAHJONG_MIANZI_HPP
