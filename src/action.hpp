#ifndef PYMAHJONG_ACTION_HPP
#define PYMAHJONG_ACTION_HPP

#include <optional>

struct Action {
    enum Type : uint8_t {
        zimo,
        dapai,
        lizhi,
        chi,
        peng,
        minggang,
        angang,
        jiagang,
        zimohu,
        ronghu,
        pingju,
    } type;

    int pai_34;
    bool red;
    std::optional<int> bias;

    Action(Type type, int pai_34) : type(type), pai_34(pai_34), red(false) {}
    Action(Type type, int pai_34, bool red) : type(type), pai_34(pai_34), red(red) {}
    Action(Type type, int pei_34, bool red, int bias) : type(type), pai_34(pei_34), red(red), bias(bias) {}
};

#endif //PYMAHJONG_ACTION_HPP
