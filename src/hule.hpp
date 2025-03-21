#ifndef HULE_HPP
#define HULE_HPP

#include "mianzi.hpp"
#include "shoupai.hpp"
#include <algorithm>
#include <vector>
#include <cassert>
#include <ranges>

#include "action.hpp"
#include "hupai.hpp"

struct HuleOption {
    bool is_menqian=true;
    bool is_lizhi=false;
    bool is_shuanglizhi=false;
    bool is_yifa=false;
    bool is_haidi=false;
    bool is_lingshang=false;
    bool is_qianggang=false;
    bool is_init_turn_and_no_call=false;
    int zhuangfeng, lunban;

    HuleOption(const int zhuangfeng, const int lunban) : zhuangfeng{zhuangfeng}, lunban{lunban} {}
};

struct Hule {
    std::vector<Mianzi> hand;

    Shoupai shoupai;
    int fu, fanshu, damanguan;

    Action hule_action;
    HuleOption option;
    Hupai hupai;
    
    std::vector<std::vector<Mianzi>> _possible_hands;
    bool is_hule;

    void rec(int p, const bool head_unknown=true) {
        if (hand.size() == 5 - shoupai.fulu.size()) {
            std::copy(hand.begin(), hand.end(), std::back_inserter(_possible_hands.emplace_back()));
            std::copy(shoupai.fulu.begin(), shoupai.fulu.end(), std::back_inserter(_possible_hands.back()));
            return;
        }
        if (p >= 34) return;
        if (not shoupai.bing[p]) return rec(p + 1);
        if (p % 9 < 7 && p < 27 && shoupai.bing[p + 1] && shoupai.bing[p + 2]) {
            --shoupai.bing[p];
            --shoupai.bing[p + 1];
            --shoupai.bing[p + 2];
            hand.emplace_back(Mianzi::shunzi, p);
            rec(p);
            hand.pop_back();
            ++shoupai.bing[p];
            ++shoupai.bing[p + 1];
            ++shoupai.bing[p + 2];
        }
        if (shoupai.bing[p] == 3) {
            shoupai.bing[p] -= 3;
            hand.emplace_back(Mianzi::kezi, p);
            rec(p + 1);
            hand.pop_back();
            shoupai.bing[p] += 3;
        }
        if (head_unknown && shoupai.bing[p] == 2) {
            shoupai.bing[p] -= 2;
            hand.emplace_back(Mianzi::duizi, p);
            rec(p + 1, false);
            hand.pop_back();
            shoupai.bing[p] += 2;
        }
    }

    bool eval_possible_hands() {
        uint64_t max_rank = 0;
        for (auto& hand : _possible_hands) {
            Hupai h = {};
            int fu = 20;
            h.断么九 = h.混全帯么九 = h.混老頭 = h.対々和 = h.緑一色 = true;
            int duizi = 0, ankezi = 0, gangzi = 0, head;
            uint64_t shunzi = 0, kezi = 0;
            uint64_t yibeikou = 0;
            bool liangmian = false, qian = false, bian = false;
            uint8_t color = 0, z = 0;
            for (auto& mianzi : hand) {
                int p = mianzi.pai_34;
                color |= 1 << p / 9;
                if (mianzi.type == Mianzi::shunzi) {
                    if (shunzi >> p & 1) yibeikou |= 1 << p;
                    shunzi |= 1 << p;
                    if (mianzi.fulu_type == Mianzi::FuluType::none) {
                        switch (hule_action.pai_34 - p) {
                        case 0:
                            (p % 9 < 6 ? liangmian : bian) = true;
                            break;
                        case 1:
                            qian = true;
                            break;
                        case 2:
                            (p % 9 > 2 ? liangmian : bian) = true;
                            break;
                        default:;
                        }
                    }
                    bool is_19 = p % 9 == 0 || p % 9 == 6;
                    h.断么九 &= not is_19;
                    h.混全帯么九 &= is_19;
                    h.混老頭 = h.対々和 = false;
                    h.緑一色 &= p / 9 == 2 && p % 9 == 1;
                } else {
                    bool is_z = p >= 27;
                    bool is_19z = p % 9 == 0 || p % 9 == 8 || is_z;
                    h.断么九 &= not is_19z;
                    h.混全帯么九 &= is_19z;
                    h.混老頭 &= is_19z;
                    h.緑一色 &= p == 32 || (p / 9 == 2 && not (is_19z || p % 9 == 4 || p % 9 == 6));
                    if (is_z) z |= 1 << p - 27;
                    if (mianzi.type == Mianzi::kezi) {
                        kezi |= 1 << p;
                        switch (mianzi.fulu_type) {
                        case Mianzi::peng:
                            fu += 2 << is_19z;
                            break;
                        case Mianzi::FuluType::none:
                            fu += 4 << is_19z;
                            ++ankezi;
                            break;
                        case Mianzi::minggang:
                            fu += 8 << is_19z;
                            ++gangzi;
                            break;
                        case Mianzi::angang:
                            fu += 16 << is_19z;
                            ++ankezi;
                            ++gangzi;
                            break;
                        default: __builtin_unreachable();
                        }
                    } else {
                        ++duizi;
                        head = p;
                        fu += 2 * (p - 27 == option.zhuangfeng);
                        fu += 2 * (p - 27 == option.lunban);
                        fu += 2 * (p > 30);
                    }
                }
            }
            bool danyi = head == hule_action.pai_34;
            bool zimohu = hule_action.type == Action::zimohu;

            if ((h.七対子 = duizi == 7)) fu = 25;
            else {
                h.平和 = fu == 20 && shoupai.fulu.empty() && liangmian;
                if (not h.平和) fu += 2 * (qian || bian || danyi);
            }

            fu += 10 * option.is_menqian;
            fu += 2 * (hule_action.type == zimohu);

            fu = (fu + 9) / 10 * 10;
            if (fu == 20 && not option.is_menqian) fu = 30;

            if (option.is_init_turn_and_no_call && zimohu) {
                h.天和 = option.lunban == 0;
                h.地和 = option.lunban != 0;
            }

            h.立直 = option.is_lizhi;
            h.一発 = option.is_yifa;
            h.門前清自摸和 = zimohu && option.is_menqian;

            h.場風_東 = option.zhuangfeng == 0 && kezi >> 27 & 1;
            h.場風_南 = option.zhuangfeng == 1 && kezi >> 28 & 1;
            h.場風_西 = option.zhuangfeng == 2 && kezi >> 29 & 1;
            h.場風_北 = option.zhuangfeng == 3 && kezi >> 30 & 1;

            h.自風_東 = option.lunban == 0 && kezi >> 27 & 1;
            h.自風_南 = option.lunban == 1 && kezi >> 28 & 1;
            h.自風_西 = option.lunban == 2 && kezi >> 29 & 1;
            h.自風_北 = option.lunban == 3 && kezi >> 30 & 1;

            h.役牌_白 = kezi >> 31 & 1;
            h.役牌_發 = kezi >> 32 & 1;
            h.役牌_中 = kezi >> 33 & 1;

            h.槍槓 = option.is_qianggang;
            h.嶺上開花 = option.is_lingshang;

            h.海底摸月 = option.is_haidi && zimohu;
            h.河底撈魚 = option.is_haidi && not zimohu;

            h.一盃口 = option.is_menqian && yibeikou != 0;
            h.二盃口 = h.一盃口 && __builtin_popcount(yibeikou) == 2;

            h.一気通貫 = (shunzi & 0b1001001) == 0b1001001 || (shunzi >> 9 & 0b1001001) == 0b1001001 || (shunzi >> 18 & 0b1001001) == 0b1001001;

            h.三色同順 = (shunzi & 0x1ff & (shunzi >> 9 & 0x1ff) & (shunzi >> 18 & 0x1ff)) != 0;
            h.三色同刻 = (kezi & 0x1ff & (kezi >> 9 & 0x1ff) & (kezi >> 18 & 0x1ff)) != 0;

            h.混一色 = color == 0b1001 || color == 0b1010 || color == 0b1100;
            h.清一色 = color == 0b0001 || color == 0b0010 || color == 0b0100;
            h.字一色 = color == 0b1000;

            h.純全帯么九 = h.混全帯么九 && z == 0;
            h.清老頭 = h.混老頭 && z == 0;

            h.小四喜 = ((z | 1 << head - 27) & 0b1111) == 0b1111;
            h.大四喜 = (z & 0b1111) == 0b1111;

            h.小三元 = ((z | 1 << head - 27) & 0b1110000) == 0b1110000;
            h.大三元 = (z & 0b1110000) == 0b1110000;

            h.三暗刻 = ankezi == 4 || ankezi == 3 && (zimohu || liangmian || qian || bian || danyi);
            h.四暗刻 = ankezi == 4 && zimohu;
            h.四暗刻単騎 = ankezi == 4 && danyi;

            h.三槓子 = gangzi == 3;
            h.四槓子 = gangzi == 4;

            if (h.清一色 && shoupai.fulu.empty()) {
                int i = __builtin_ctz(color) * 9;
                h.九蓮宝燈 =
                    shoupai.bing[i] >= 3 &&
                    shoupai.bing[i + 1] &&
                    shoupai.bing[i + 2] &&
                    shoupai.bing[i + 3] &&
                    shoupai.bing[i + 4] &&
                    shoupai.bing[i + 5] &&
                    shoupai.bing[i + 6] &&
                    shoupai.bing[i + 7] &&
                    shoupai.bing[i + 8] >= 3;
            }
            h.純正九蓮宝燈 = h.九蓮宝燈 && head == hule_action.pai_34;

            h.preset(option.is_menqian);
            auto [fanshu, damanguan] = h.sum();
            uint64_t rank = damanguan << 32 | fanshu << 16 || fu;
            if (rank > max_rank) {
                max_rank = rank;
                this->fu = fu;
                this->fanshu = fanshu;
                this->damanguan = damanguan;
                this->hupai = h;
            }
        }
        return damanguan || fanshu;
    }

    Hule(const Shoupai& shoupai, const Action& action, const HuleOption& option) : shoupai{shoupai}, hule_action(action), option{option} {
        assert (shoupai.xiangting == -1);
        assert (action.type == Action::zimohu || action.type == Action::ronghu);
        hupai = {};
        if (shoupai.mode >> 2) {
            hupai.国士無双 = shoupai.bing[action.pai_34] == 1;
            hupai.国士無双１３面 = not hupai.国士無双;
            hupai.preset(true);
            fu = action.type == Action::zimohu ? 30 : 40;
            std::tie(fanshu, damanguan) = hupai.sum();
            is_hule = true;
            return;
        }
        rec(0);
        is_hule = eval_possible_hands();
    }
};

#endif //HULE_HPP
