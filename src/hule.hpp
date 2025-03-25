#ifndef HULE_HPP
#define HULE_HPP

#include "mianzi.hpp"
#include "shoupai.hpp"
#include <algorithm>
#include <vector>

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
    bool has_hupai;
    int fu = 0, fanshu = 0, damanguan = 0;

    int hule_pai; bool is_zimohu;
    HuleOption option;
    Hupai hupai;

    std::vector<std::vector<Mianzi>> _possible_hands;
    int _argmax_possible_hands = -1;

    void search_normal_form(int p, const bool head_unknown=true) {
        if (hand.size() == 5 - shoupai.fulu.size()) {
            auto tail = std::back_inserter(_possible_hands.emplace_back());
            std::copy(hand.begin(), hand.end(), tail);
            std::copy(shoupai.fulu.begin(), shoupai.fulu.end(), tail);
            return;
        }
        if (p >= 34) return;
        if (not shoupai.bing[p]) return search_normal_form(p + 1);
        if (p % 9 < 7 && p < 27 && shoupai.bing[p + 1] && shoupai.bing[p + 2]) {
            --shoupai.bing[p];
            --shoupai.bing[p + 1];
            --shoupai.bing[p + 2];
            hand.emplace_back(Mianzi::shunzi, p);
            search_normal_form(p);
            hand.pop_back();
            ++shoupai.bing[p];
            ++shoupai.bing[p + 1];
            ++shoupai.bing[p + 2];
        }
        if (shoupai.bing[p] == 3) {
            shoupai.bing[p] -= 3;
            hand.emplace_back(Mianzi::kezi, p);
            search_normal_form(p + 1);
            hand.pop_back();
            shoupai.bing[p] += 3;
        }
        if (head_unknown && shoupai.bing[p] == 2) {
            shoupai.bing[p] -= 2;
            hand.emplace_back(Mianzi::duizi, p);
            search_normal_form(p + 1, false);
            hand.pop_back();
            shoupai.bing[p] += 2;
        }
    }

    void search_7duizi_form() {
        for (int i = 0; i < 34; ++i) {
            if (shoupai.bing[i] == 2) hand.emplace_back(Mianzi::duizi, i);
        }
        if (hand.size() == 7) std::copy(hand.begin(), hand.end(), std::back_inserter(_possible_hands.emplace_back()));
        hand.clear();
    }

    bool eval_possible_hands() {
        uint64_t max_rank = 0;
        int index = 0;
        for (auto& hand : _possible_hands) {
            Hupai h = {}; h.is_menqian = option.is_menqian;
            int fu = 20;
            int ankezi = 0, gangzi = 0, head;
            uint64_t shunzi = 0, kezi = 0, duizi = 0;
            uint64_t yibeikou = 0;
            bool liangmian = false, qian = false, bian = false;
            uint8_t color = 0;
            for (auto& mianzi : hand) {
                int p = mianzi.pai_34;
                color |= 1 << p / 9;
                if (mianzi.type == Mianzi::shunzi) {
                    if (shunzi >> p & 1) yibeikou |= 1 << p;
                    shunzi |= 1 << p;
                    if (mianzi.fulu_type == Mianzi::FuluType::none) {
                        switch (hule_pai - p) {
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
                } else {
                    bool is_z = p >= 27;
                    bool is_19z = p % 9 == 0 || p % 9 == 8 || is_z;
                    if (mianzi.type == Mianzi::kezi) {
                        kezi |= 1uLL << p;
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
                        duizi |= 1uLL << p;
                        head = p;
                        fu += 2 * (p - 27 == option.zhuangfeng);
                        fu += 2 * (p - 27 == option.lunban);
                        fu += 2 * (p > 30);
                    }
                }
            }

            bool danyi = head == hule_pai;

            if ((h.七対子 = __builtin_popcount(duizi) == 7)) fu = 25;
            else {
                h.平和 = fu == 20 && shoupai.fulu.empty() && liangmian;
                if (not h.平和) fu += 2 * (qian || bian || danyi);
            }

            fu += 10 * (not is_zimohu && option.is_menqian);
            fu += 2 * is_zimohu;

            fu = (fu + 9) / 10 * 10;
            if (fu == 20 && not option.is_menqian) fu = 30;

            h.対々和 = shunzi == 0 && not h.七対子;

            if (option.is_init_turn_and_no_call && is_zimohu) {
                h.天和 = option.lunban == 0;
                h.地和 = option.lunban != 0;
            }

            h.立直 = option.is_lizhi; h.両立直 = option.is_shuanglizhi;
            h.一発 = option.is_yifa;
            h.門前清自摸和 = is_zimohu && option.is_menqian;

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

            h.海底摸月 = option.is_haidi && is_zimohu;
            h.河底撈魚 = option.is_haidi && not is_zimohu;

            h.一盃口 = option.is_menqian && yibeikou != 0;
            h.二盃口 = h.一盃口 && __builtin_popcount(yibeikou) == 2;

            h.一気通貫 = (shunzi & 0b1001001) == 0b1001001 || (shunzi >> 9 & 0b1001001) == 0b1001001 || (shunzi >> 18 & 0b1001001) == 0b1001001;

            h.三色同順 = (shunzi & 0x1ff & (shunzi >> 9 & 0x1ff) & (shunzi >> 18 & 0x1ff)) != 0;
            h.三色同刻 = (kezi & 0x1ff & (kezi >> 9 & 0x1ff) & (kezi >> 18 & 0x1ff)) != 0;

            h.混一色 = color == 0b1001 || color == 0b1010 || color == 0b1100;
            h.清一色 = color == 0b0001 || color == 0b0010 || color == 0b0100;
            h.字一色 = color == 0b1000;

            uint64_t kd = kezi | duizi;

            h.緑一色 = color == 0b1100 && (shunzi >> 18 & 0b111111101) == 0 && (kd >> 18 & 0b1011111101010001) == 0;

            h.断么九 = color < 0b1000 && (shunzi & 0b001000001001000001001000001 | kd & 0b100000001100000001100000001) == 0;

            h.混全帯么九 = (shunzi & 0b110111110110111110110111110 | kd & 0b011111110011111110011111110) == 0;
            h.純全帯么九 = h.混全帯么九 && color < 0b1000;

            h.混老頭 = (h.対々和 || h.七対子) && h.混全帯么九;
            h.清老頭 = h.混老頭 && h.純全帯么九;

            h.小四喜 = ((kezi | 1uLL << head) >> 27 & 0b1111) == 0b1111;
            h.大四喜 = (kezi >> 27 & 0b1111) == 0b1111;

            h.小三元 = ((kezi | 1uLL << head) >> 31 & 0b111) == 0b111;
            h.大三元 = (kezi >> 31 & 0b111) == 0b111;

            h.三暗刻 = ankezi == 4 || ankezi == 3 && (is_zimohu || liangmian || qian || bian || danyi);
            h.四暗刻 = ankezi == 4 && is_zimohu;
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
            h.純正九蓮宝燈 = h.九蓮宝燈 && danyi;

            auto [fanshu, damanguan] = h.sum();
            uint64_t rank = damanguan << 32 | fanshu << 16 || fu;
            if (rank > max_rank) {
                max_rank = rank;
                this->fu = fu;
                this->fanshu = fanshu;
                this->damanguan = damanguan;
                this->hupai = h;
                this->_argmax_possible_hands = index;
            }
            ++index;
        }
        return fanshu || damanguan;
    }

    Hule(Shoupai& shoupai, const Action& action, const HuleOption& option) : shoupai{shoupai}, option{option}, hupai{} {
        shoupai.update();
        if (shoupai.xiangting != -1) return;
        hule_pai = action.pai_34;
        is_zimohu = action.type == Action::zimohu;
        if (shoupai.mode & 4) {
            hupai.国士無双 = shoupai.bing[action.pai_34] == 1;
            hupai.国士無双１３面 = not hupai.国士無双;
            hupai.is_menqian = true;
            fu = action.type == Action::zimohu ? 30 : 40;
            std::tie(fanshu, damanguan) = hupai.sum();
            has_hupai = true;
            return;
        }
        search_normal_form(0);
        if (shoupai.mode & 2) search_7duizi_form();
        if ((has_hupai = eval_possible_hands())) hand = _possible_hands[_argmax_possible_hands];
    }
};

#endif //HULE_HPP
