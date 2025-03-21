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
    std::optional<bool> red;
    std::optional<int> bias;

    Action(Type type, int pai_136) : type(type), pai_34(pai_136) {}
    Action(Type type, int pai_136, bool red) : type(type), pai_34(pai_136), red(red) {}
    Action(Type type, int pai_136, bool red, int bias) : type(type), pai_34(pai_136), red(red), bias(bias) {}
};

#endif //PYMAHJONG_ACTION_HPP
