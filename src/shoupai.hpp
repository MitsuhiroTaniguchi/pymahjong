#ifndef PYMAHJONG_SHOUPAI_HPP
#define PYMAHJONG_SHOUPAI_HPP

#include <vector>
#include <array>
#include <stdexcept>
#include <bitset>
#include "calsht_dw.hpp"
#include "action.hpp"
#include "mianzi.hpp"

struct Shoupai {
    std::array<int, 34> bing;
    std::vector<Mianzi> fulu;

    int xiangting, mode;
    std::bitset<34> tingpai;

    std::bitset<3> red;

    Shoupai() : bing{} {}
    explicit Shoupai(const std::array<int, 34>& bing) : bing{bing} {}
    Shoupai(const std::array<int, 34>& bing, const std::vector<Mianzi>& fulu) : bing{bing}, fulu{fulu} {}

    void apply(const Action& action) {
        int p = action.pai_34;
        switch (action.type) {
        case Action::zimo:
            ++bing[p];
            if (action.red) red.set(p / 9);
            return;
        case Action::dapai:
        case Action::lizhi:
            --bing[p];
            if (action.red) red.reset(p / 9);
            return;
        case Action::chi: {
            int anchor = p - action.bias.value();
            for (int i = anchor; i <= anchor + 2; ++i) {
                if (i != p) --bing[i];
            }
            if (action.red) red.reset(p / 9);
            fulu.emplace_back(Mianzi::chi, anchor);
            return;
        }
        case Action::peng:
            bing[p] -= 2;
            if (action.red) red.reset(p / 9);
            fulu.emplace_back(Mianzi::peng, p);
            return;
        case Action::minggang:
            bing[p] -= 3;
            if (p < 27 && p % 9 == 4) red.reset(p / 9);
            fulu.emplace_back(Mianzi::minggang, p);
            return;
        case Action::angang:
            bing[p] -= 4;
            if (p < 27 && p % 9 == 4) red.reset(p / 9);
            fulu.emplace_back(Mianzi::angang, p);
            return;
        case Action::jiagang:
            for (auto& mianzi : fulu) {
                if (mianzi.fulu_type == Mianzi::peng && mianzi.pai_34 == p) {
                    mianzi.fulu_type = Mianzi::minggang;
                    return;
                }
            }
        default:;
        }
    }

    void update() {
        auto [x, m, d, w] = _x_cal()(bing, 4 - fulu.size(), 7);
        xiangting = x;
        mode = m;
        tingpai = std::bitset<34>(w);
    }

    std::string to_string() const {
        std::string s;
        for (int i = 0; i < 4; ++i) {
            s += "mpsz"[i];
            for (int j = 0; j < (i < 3 ? 9 : 7); ++j) {
                int p = i * 9 + j;
                for (int k = 0; k < bing[p]; ++k) s += std::to_string(j);
            }
        }
        for (auto& mianzi : fulu) {
            s += ",";
            s += mianzi.to_string();
        }
        return s;
    }

private:
    static CalshtDW& _x_cal() {
        static CalshtDW xiangting_calculator;
        return xiangting_calculator;
    }
};

#endif //PYMAHJONG_SHOUPAI_HPP
