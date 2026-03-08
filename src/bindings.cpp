#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <tuple>
#include <cstdint>
#include <array>

#include "calsht_dw.hpp"
#include "shoupai.hpp"
#include "mianzi.hpp"
#include "hupai.hpp"
#include "hule.hpp"
#include "action.hpp"

namespace py = pybind11;

namespace {
constexpr int SELF_OPT_TSUMO = 1 << 0;
constexpr int SELF_OPT_RIICHI = 1 << 1;
constexpr int SELF_OPT_ANKAN = 1 << 2;
constexpr int SELF_OPT_KAKAN = 1 << 3;
constexpr int SELF_OPT_KYUSHUKYUHAI = 1 << 4;
constexpr int SELF_OPT_PENUKI = 1 << 5;

constexpr int REACT_OPT_RON = 1 << 0;
constexpr int REACT_OPT_CHI = 1 << 1;
constexpr int REACT_OPT_PON = 1 << 2;
constexpr int REACT_OPT_MINKAN = 1 << 3;

uint64_t bitset34_to_mask(const std::bitset<34>& bits) {
    uint64_t mask = 0;
    for (int i = 0; i < 34; ++i) {
        if (bits.test(i)) {
            mask |= (uint64_t(1) << i);
        }
    }
    return mask;
}

std::bitset<34> mask_to_bitset34(uint64_t mask) {
    std::bitset<34> bits;
    for (int i = 0; i < 34; ++i) {
        if ((mask >> i) & uint64_t(1)) {
            bits.set(i);
        }
    }
    return bits;
}

uint32_t bitset3_to_mask(const std::bitset<3>& bits) {
    uint32_t mask = 0;
    for (int i = 0; i < 3; ++i) {
        if (bits.test(i)) {
            mask |= (uint32_t(1) << i);
        }
    }
    return mask;
}

std::bitset<3> mask_to_bitset3(uint32_t mask) {
    std::bitset<3> bits;
    for (int i = 0; i < 3; ++i) {
        if ((mask >> i) & uint32_t(1)) {
            bits.set(i);
        }
    }
    return bits;
}

std::vector<Mianzi> encode_melds(const std::vector<std::pair<int, int>>& melds) {
    std::vector<Mianzi> fulu;
    fulu.reserve(melds.size());
    for (const auto& [meld_type, pai_34] : melds) {
        switch (meld_type) {
        case 0:
            fulu.emplace_back(Mianzi::chi, pai_34);
            break;
        case 1:
            fulu.emplace_back(Mianzi::peng, pai_34);
            break;
        case 2:
            fulu.emplace_back(Mianzi::minggang, pai_34);
            break;
        case 3:
            fulu.emplace_back(Mianzi::angang, pai_34);
            break;
        default:
            throw std::runtime_error("invalid meld type");
        }
    }
    return fulu;
}

bool has_hupai_impl_ex(const std::array<int, 34>& hand,
                       const std::vector<std::pair<int, int>>& melds,
                       int win_tile,
                       bool is_tsumo,
                       bool is_menqian,
                       bool is_riichi,
                       int zhuangfeng,
                       int lunban,
                       bool is_haidi,
                       bool is_lingshang,
                       bool is_qianggang);

bool has_hupai_shoupai_impl(const Shoupai& base,
                            int win_tile,
                            bool is_tsumo,
                            bool is_menqian,
                            bool is_riichi,
                            int zhuangfeng,
                            int lunban,
                            bool is_haidi,
                            bool is_lingshang,
                            bool is_qianggang) {
    Shoupai shoupai = base;
    HuleOption option(zhuangfeng, lunban);
    option.is_menqian = is_menqian;
    option.is_lizhi = is_riichi;
    option.is_haidi = is_haidi;
    option.is_lingshang = is_lingshang;
    option.is_qianggang = is_qianggang;
    Action action(is_tsumo ? Action::zimohu : Action::ronghu, win_tile);
    Hule hule(shoupai, action, option);
    return hule.has_hupai;
}

uint64_t wait_mask_impl(const std::array<int, 34>& hand, int meld_count, bool three_player = false) {
    static CalshtDW xiangting_calculator;
    auto [x, _mode, _disc, wait] = xiangting_calculator(hand, 4 - meld_count, 7, false, three_player);
    if (x != 0) return uint64_t(0);
    return static_cast<uint64_t>(wait);
}

bool has_riichi_discard_impl(const std::array<int, 34>& hand, int meld_count, bool three_player = false) {
    static CalshtDW xiangting_calculator;
    auto base = hand;
    for (int i = 0; i < 34; ++i) {
        if (three_player && i > 0 && i < 8) continue;
        if (base[i] == 0) continue;
        --base[i];
        auto [x, _mode, _disc, _wait] = xiangting_calculator(base, 4 - meld_count, 7, false, three_player);
        ++base[i];
        if (x == 0) return true;
    }
    return false;
}

bool can_chi_impl(const std::array<int, 34>& hand, int tile_idx) {
    if (tile_idx >= 27) return false;
    int suit_base = (tile_idx / 9) * 9;
    int n = tile_idx % 9 + 1;
    const std::array<std::pair<int, int>, 3> patterns = {{
        {n - 2, n - 1},
        {n - 1, n + 1},
        {n + 1, n + 2},
    }};
    for (const auto& [a, b] : patterns) {
        if (a < 1 || b > 9) continue;
        int ai = suit_base + (a - 1);
        int bi = suit_base + (b - 1);
        if (hand[ai] > 0 && hand[bi] > 0) return true;
    }
    return false;
}

bool has_legal_post_call_discard_impl(
    const std::array<int, 34>& hand,
    const std::vector<int>& forbidden_tiles
) {
    std::array<bool, 34> forbidden{};
    for (int tile : forbidden_tiles) {
        if (tile >= 0 && tile < 34) forbidden[tile] = true;
    }
    for (int tile = 0; tile < 34; ++tile) {
        if (hand[tile] <= 0) continue;
        if (!forbidden[tile]) return true;
    }
    return false;
}

bool has_legal_post_call_discard_impl(
    const std::array<int, 34>& hand,
    int forbidden_a,
    int forbidden_b = -1
) {
    for (int tile = 0; tile < 34; ++tile) {
        if (hand[tile] <= 0) continue;
        if (tile == forbidden_a || tile == forbidden_b) continue;
        return true;
    }
    return false;
}

bool can_chi_tenhou_impl(const std::array<int, 34>& hand, int tile_idx) {
    if (tile_idx >= 27) return false;
    int suit_base = (tile_idx / 9) * 9;
    int n = tile_idx % 9 + 1;
    const std::array<std::pair<int, int>, 3> patterns = {{
        {n - 2, n - 1},
        {n - 1, n + 1},
        {n + 1, n + 2},
    }};
    for (const auto& [a, b] : patterns) {
        if (a < 1 || b > 9) continue;
        int ai = suit_base + (a - 1);
        int bi = suit_base + (b - 1);
        if (hand[ai] <= 0 || hand[bi] <= 0) continue;
        auto remaining = hand;
        --remaining[ai];
        --remaining[bi];
        int seq_low = std::min({ai, bi, tile_idx});
        int seq_high = std::max({ai, bi, tile_idx});
        if (tile_idx == seq_low && seq_high + 1 < suit_base + 9) {
            if (has_legal_post_call_discard_impl(remaining, tile_idx, seq_high + 1)) return true;
            continue;
        }
        if (tile_idx == seq_high && seq_low - 1 >= suit_base) {
            if (has_legal_post_call_discard_impl(remaining, tile_idx, seq_low - 1)) return true;
            continue;
        }
        if (has_legal_post_call_discard_impl(remaining, tile_idx)) return true;
    }
    return false;
}

bool can_pon_tenhou_impl(const std::array<int, 34>& hand, int tile_idx) {
    if (tile_idx < 0 || tile_idx >= 34 || hand[tile_idx] < 2) return false;
    auto remaining = hand;
    remaining[tile_idx] -= 2;
    return has_legal_post_call_discard_impl(remaining, tile_idx);
}

bool can_ankan_impl(
    const std::array<int, 34>& hand,
    int meld_count,
    bool is_riichi,
    int drawn_tile,
    bool three_player
) {
    bool any_candidate = false;
    for (int tile = 0; tile < 34; ++tile) {
        if (hand[tile] < 4) continue;
        any_candidate = true;
        if (!is_riichi) return true;
        if (tile != drawn_tile) continue;
        auto pre_draw = hand;
        if (pre_draw[drawn_tile] <= 0) continue;
        --pre_draw[drawn_tile];
        uint64_t waits_before = wait_mask_impl(pre_draw, meld_count, three_player);
        if (waits_before == 0) continue;
        auto after = hand;
        after[tile] -= 4;
        uint64_t waits_after = wait_mask_impl(after, meld_count + 1, three_player);
        if (waits_after != 0 && waits_after == waits_before) return true;
    }
    return !is_riichi && any_candidate;
}

bool can_kyushukyuhai_impl(
    const std::array<int, 34>& hand,
    bool is_first_turn,
    bool first_turn_open_calls_seen,
    int open_melds,
    int closed_kans
) {
    if (!is_first_turn || first_turn_open_calls_seen || open_melds > 0 || closed_kans > 0) return false;
    constexpr std::array<int, 13> yaochu = {{0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33}};
    int uniq = 0;
    for (int idx : yaochu) {
        if (hand[idx] > 0) ++uniq;
    }
    return uniq >= 9;
}

bool is_kokushi_agari_shape_impl(const std::array<int, 34>& hand, int meld_count) {
    if (meld_count != 0) return false;
    int total = 0;
    for (int x : hand) total += x;
    if (total != 14) return false;
    bool pair_found = false;
    for (int i = 0; i < 34; ++i) {
        bool is_yaochu = i == 0 || i == 8 || i == 9 || i == 17 || i == 18 || i == 26 || i >= 27;
        int c = hand[i];
        if (is_yaochu) {
            if (c == 0) return false;
            if (c == 1) continue;
            if (c == 2 && !pair_found) {
                pair_found = true;
                continue;
            }
            return false;
        }
        if (c != 0) return false;
    }
    return pair_found;
}

int compute_self_option_mask_impl(
    const std::array<int, 34>& hand,
    const std::vector<std::pair<int, int>>& melds,
    int drawn_tile,
    bool is_riichi,
    int score,
    int zhuangfeng,
    int lunban,
    int live_draws_left,
    int closed_kans,
    int open_melds,
    const std::vector<int>& open_pon_tiles,
    bool is_first_turn,
    bool first_turn_open_calls_seen,
    bool is_gangzimo,
    bool three_player
) {
    int mask = 0;
    bool can_riichi = !is_riichi && open_melds == 0 && score >= 1000 && live_draws_left >= 4;
    bool can_tsumo = has_hupai_impl_ex(
        hand,
        melds,
        drawn_tile,
        true,
        open_melds == 0,
        is_riichi,
        zhuangfeng,
        lunban,
        live_draws_left == 0 && !is_gangzimo,
        is_gangzimo,
        false);
    if (can_tsumo) mask |= SELF_OPT_TSUMO;
    if (can_riichi && has_riichi_discard_impl(hand, closed_kans, three_player)) mask |= SELF_OPT_RIICHI;
    if (can_ankan_impl(hand, static_cast<int>(melds.size()), is_riichi, drawn_tile, three_player)) mask |= SELF_OPT_ANKAN;
    for (int tile : open_pon_tiles) {
        if (tile >= 0 && tile < 34 && hand[tile] > 0) {
            mask |= SELF_OPT_KAKAN;
            break;
        }
    }
    if (can_kyushukyuhai_impl(hand, is_first_turn, first_turn_open_calls_seen, open_melds, closed_kans)) {
        mask |= SELF_OPT_KYUSHUKYUHAI;
    }
    if (three_player && hand[30] > 0) {
        mask |= SELF_OPT_PENUKI;
    }
    return mask;
}

using ReactionPlayerInput = std::tuple<
    std::array<int, 34>,
    std::vector<std::pair<int, int>>,
    bool,
    bool,
    bool,
    uint64_t,
    int
>;

using ReactionPlayerCachedInput = std::tuple<
    std::array<int, 34>,
    std::vector<std::pair<int, int>>,
    bool,
    bool,
    bool,
    uint64_t,
    int,
    uint64_t
>;

using ReactionShoupaiInput = std::tuple<
    Shoupai,
    bool,
    bool,
    bool,
    uint64_t,
    int
>;

std::vector<std::pair<int, int>> compute_reaction_option_masks_impl(
    const std::vector<ReactionPlayerInput>& players,
    int discarder,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    int live_draws_left,
    bool last_draw_was_gangzimo,
    bool three_player
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(3);
    bool is_haidi = live_draws_left == 0 && !last_draw_was_gangzimo;
    int seat_count = static_cast<int>(players.size());
    for (int offset = 1; offset < seat_count; ++offset) {
        int seat = (discarder + offset) % seat_count;
        const auto& [hand, melds, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds] = players[seat];
        int option_mask = 0;
        int meld_count = static_cast<int>(melds.size());
        uint64_t waits = wait_mask_impl(hand, meld_count, three_player);
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (!temporary_furiten && !riichi_furiten && !permanent_furiten && ((waits >> tile_idx) & 1ULL)) {
            auto ron_hand = hand;
            ++ron_hand[tile_idx];
            bool can_ron = has_hupai_impl_ex(
                ron_hand,
                melds,
                tile_idx,
                false,
                open_melds == 0,
                is_riichi,
                zhuangfeng,
                (seat - dealer_seat + seat_count) % seat_count,
                is_haidi,
                false,
                false);
            if (can_ron) option_mask |= REACT_OPT_RON;
        }
        if (!is_riichi && live_draws_left > 0) {
            if (!three_player && offset == 1 && can_chi_tenhou_impl(hand, tile_idx)) option_mask |= REACT_OPT_CHI;
            if (can_pon_tenhou_impl(hand, tile_idx)) option_mask |= REACT_OPT_PON;
            if (hand[tile_idx] >= 3) option_mask |= REACT_OPT_MINKAN;
        }
        if (option_mask != 0) out.emplace_back(seat, option_mask);
    }
    return out;
}

std::vector<std::pair<int, int>> compute_reaction_option_masks_cached_impl(
    const std::vector<ReactionPlayerCachedInput>& players,
    int discarder,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    int live_draws_left,
    bool last_draw_was_gangzimo
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(players.size() > 0 ? players.size() - 1 : 0);
    bool is_haidi = live_draws_left == 0 && !last_draw_was_gangzimo;
    int seat_count = static_cast<int>(players.size());
    for (int offset = 1; offset < seat_count; ++offset) {
        int seat = (discarder + offset) % seat_count;
        const auto& [hand, melds, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds, waits] = players[seat];
        int option_mask = 0;
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (!temporary_furiten && !riichi_furiten && !permanent_furiten && ((waits >> tile_idx) & 1ULL)) {
            auto ron_hand = hand;
            ++ron_hand[tile_idx];
            bool can_ron = has_hupai_impl_ex(
                ron_hand,
                melds,
                tile_idx,
                false,
                open_melds == 0,
                is_riichi,
                zhuangfeng,
                (seat - dealer_seat + seat_count) % seat_count,
                is_haidi,
                false,
                false);
            if (can_ron) option_mask |= REACT_OPT_RON;
        }
        if (!is_riichi && live_draws_left > 0) {
            if (can_pon_tenhou_impl(hand, tile_idx)) option_mask |= REACT_OPT_PON;
            if (hand[tile_idx] >= 3) option_mask |= REACT_OPT_MINKAN;
        }
        if (option_mask != 0) out.emplace_back(seat, option_mask);
    }
    return out;
}

std::vector<std::pair<int, int>> compute_rob_kan_option_masks_impl(
    const std::vector<ReactionPlayerInput>& players,
    int actor,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    bool require_kokushi,
    bool three_player
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(3);
    int seat_count = static_cast<int>(players.size());
    for (int seat = 0; seat < seat_count; ++seat) {
        if (seat == actor) continue;
        const auto& [hand, melds, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds] = players[seat];
        int meld_count = static_cast<int>(melds.size());
        uint64_t waits = wait_mask_impl(hand, meld_count, three_player);
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (temporary_furiten || riichi_furiten || permanent_furiten || ((waits >> tile_idx) & 1ULL) == 0) {
            continue;
        }
        auto ron_hand = hand;
        ++ron_hand[tile_idx];
        if (require_kokushi && !is_kokushi_agari_shape_impl(ron_hand, meld_count)) continue;
        bool can_ron = has_hupai_impl_ex(
            ron_hand,
            melds,
            tile_idx,
            false,
            open_melds == 0,
            is_riichi,
            zhuangfeng,
            (seat - dealer_seat + seat_count) % seat_count,
            false,
            false,
            true);
        if (can_ron) out.emplace_back(seat, REACT_OPT_RON);
    }
    return out;
}

std::vector<std::pair<int, int>> compute_rob_kan_option_masks_cached_impl(
    const std::vector<ReactionPlayerCachedInput>& players,
    int actor,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    bool require_kokushi
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(players.size() > 0 ? players.size() - 1 : 0);
    int seat_count = static_cast<int>(players.size());
    for (int seat = 0; seat < seat_count; ++seat) {
        if (seat == actor) continue;
        const auto& [hand, melds, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds, waits] = players[seat];
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (temporary_furiten || riichi_furiten || permanent_furiten || ((waits >> tile_idx) & 1ULL) == 0) {
            continue;
        }
        auto ron_hand = hand;
        ++ron_hand[tile_idx];
        if (require_kokushi && !is_kokushi_agari_shape_impl(ron_hand, static_cast<int>(melds.size()))) continue;
        bool can_ron = has_hupai_impl_ex(
            ron_hand,
            melds,
            tile_idx,
            false,
            open_melds == 0,
            is_riichi,
            zhuangfeng,
            (seat - dealer_seat + seat_count) % seat_count,
            false,
            false,
            true);
        if (can_ron) out.emplace_back(seat, REACT_OPT_RON);
    }
    return out;
}

int compute_self_option_mask_shoupai_impl(
    const Shoupai& shoupai,
    int drawn_tile,
    bool is_riichi,
    int score,
    int zhuangfeng,
    int lunban,
    int live_draws_left,
    int closed_kans,
    int open_melds,
    const std::vector<int>& open_pon_tiles,
    bool is_first_turn,
    bool first_turn_open_calls_seen,
    bool is_gangzimo,
    bool three_player
) {
    int mask = 0;
    bool can_riichi = !is_riichi && open_melds == 0 && score >= 1000 && live_draws_left >= 4;
    bool can_tsumo = has_hupai_shoupai_impl(
        shoupai,
        drawn_tile,
        true,
        open_melds == 0,
        is_riichi,
        zhuangfeng,
        lunban,
        live_draws_left == 0 && !is_gangzimo,
        is_gangzimo,
        false);
    if (can_tsumo) mask |= SELF_OPT_TSUMO;
    if (can_riichi && has_riichi_discard_impl(shoupai.bing, closed_kans, three_player)) mask |= SELF_OPT_RIICHI;
    if (can_ankan_impl(
            shoupai.bing,
            static_cast<int>(shoupai.fulu.size()),
            is_riichi,
            drawn_tile,
            three_player)) {
        mask |= SELF_OPT_ANKAN;
    }
    for (int tile : open_pon_tiles) {
        if (tile >= 0 && tile < 34 && shoupai.bing[tile] > 0) {
            mask |= SELF_OPT_KAKAN;
            break;
        }
    }
    if (can_kyushukyuhai_impl(shoupai.bing, is_first_turn, first_turn_open_calls_seen, open_melds, closed_kans)) {
        mask |= SELF_OPT_KYUSHUKYUHAI;
    }
    if (three_player && shoupai.bing[30] > 0) {
        mask |= SELF_OPT_PENUKI;
    }
    return mask;
}

std::vector<std::pair<int, int>> compute_reaction_option_masks_shoupai_impl(
    const std::vector<ReactionShoupaiInput>& players,
    int discarder,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    int live_draws_left,
    bool last_draw_was_gangzimo,
    bool three_player
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(3);
    bool is_haidi = live_draws_left == 0 && !last_draw_was_gangzimo;
    int seat_count = static_cast<int>(players.size());
    for (int offset = 1; offset < seat_count; ++offset) {
        int seat = (discarder + offset) % seat_count;
        const auto& [shoupai, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds] = players[seat];
        int option_mask = 0;
        uint64_t waits = wait_mask_impl(shoupai.bing, static_cast<int>(shoupai.fulu.size()), three_player);
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (!temporary_furiten && !riichi_furiten && !permanent_furiten && ((waits >> tile_idx) & 1ULL)) {
            Shoupai ron_shoupai = shoupai;
            ++ron_shoupai.bing[tile_idx];
            bool can_ron = has_hupai_shoupai_impl(
                ron_shoupai,
                tile_idx,
                false,
                open_melds == 0,
                is_riichi,
                zhuangfeng,
                (seat - dealer_seat + seat_count) % seat_count,
                is_haidi,
                false,
                false);
            if (can_ron) option_mask |= REACT_OPT_RON;
        }
        if (!is_riichi && live_draws_left > 0) {
            if (!three_player && offset == 1 && can_chi_tenhou_impl(shoupai.bing, tile_idx)) option_mask |= REACT_OPT_CHI;
            if (can_pon_tenhou_impl(shoupai.bing, tile_idx)) option_mask |= REACT_OPT_PON;
            if (shoupai.bing[tile_idx] >= 3) option_mask |= REACT_OPT_MINKAN;
        }
        if (option_mask != 0) out.emplace_back(seat, option_mask);
    }
    return out;
}

std::vector<std::pair<int, int>> compute_rob_kan_option_masks_shoupai_impl(
    const std::vector<ReactionShoupaiInput>& players,
    int actor,
    int tile_idx,
    int zhuangfeng,
    int dealer_seat,
    bool require_kokushi,
    bool three_player
) {
    std::vector<std::pair<int, int>> out;
    out.reserve(3);
    int seat_count = static_cast<int>(players.size());
    for (int seat = 0; seat < seat_count; ++seat) {
        if (seat == actor) continue;
        const auto& [shoupai, is_riichi, temporary_furiten, riichi_furiten, furiten_mask, open_melds] = players[seat];
        uint64_t waits = wait_mask_impl(shoupai.bing, static_cast<int>(shoupai.fulu.size()), three_player);
        bool permanent_furiten = (waits & furiten_mask) != 0;
        if (temporary_furiten || riichi_furiten || permanent_furiten || ((waits >> tile_idx) & 1ULL) == 0) {
            continue;
        }
        Shoupai ron_shoupai = shoupai;
        ++ron_shoupai.bing[tile_idx];
        if (require_kokushi && !is_kokushi_agari_shape_impl(ron_shoupai.bing, static_cast<int>(ron_shoupai.fulu.size()))) {
            continue;
        }
        bool can_ron = has_hupai_shoupai_impl(
            ron_shoupai,
            tile_idx,
            false,
            open_melds == 0,
            is_riichi,
            zhuangfeng,
            (seat - dealer_seat + seat_count) % seat_count,
            false,
            false,
            true);
        if (can_ron) out.emplace_back(seat, REACT_OPT_RON);
    }
    return out;
}

bool has_hupai_impl(const std::array<int, 34>& hand,
                    const std::vector<std::pair<int, int>>& melds,
                    int win_tile,
                    bool is_tsumo,
                    bool is_menqian,
                    bool is_riichi,
                    int zhuangfeng,
                    int lunban) {
    auto fulu = encode_melds(melds);
    Shoupai shoupai = fulu.empty() ? Shoupai(hand) : Shoupai(hand, fulu);
    HuleOption option(zhuangfeng, lunban);
    option.is_menqian = is_menqian;
    option.is_lizhi = is_riichi;
    Action action(is_tsumo ? Action::zimohu : Action::ronghu, win_tile);
    Hule hule(shoupai, action, option);
    return hule.has_hupai;
}

bool has_hupai_impl_ex(const std::array<int, 34>& hand,
                       const std::vector<std::pair<int, int>>& melds,
                       int win_tile,
                       bool is_tsumo,
                       bool is_menqian,
                       bool is_riichi,
                       int zhuangfeng,
                       int lunban,
                       bool is_haidi,
                       bool is_lingshang,
                       bool is_qianggang) {
    auto fulu = encode_melds(melds);
    Shoupai shoupai = fulu.empty() ? Shoupai(hand) : Shoupai(hand, fulu);
    HuleOption option(zhuangfeng, lunban);
    option.is_menqian = is_menqian;
    option.is_lizhi = is_riichi;
    option.is_haidi = is_haidi;
    option.is_lingshang = is_lingshang;
    option.is_qianggang = is_qianggang;
    Action action(is_tsumo ? Action::zimohu : Action::ronghu, win_tile);
    Hule hule(shoupai, action, option);
    return hule.has_hupai;
}
}  // namespace

PYBIND11_MODULE(pymahjong, m) {
    m.doc() = "pymahjong: mahjong calculation library written in c++ with python bindings";

    // Xiangting (CalshtDW) のバインディング
    py::class_<CalshtDW>(m, "Xiangting")
        .def(py::init<>())
        .def("calculate", &CalshtDW::operator(),
             py::arg("hand"), py::arg("size"), py::arg("mode"),
             py::arg("check_hand") = false, py::arg("three_player") = false);

    // Mianzi の enum バインディング
    py::enum_<Mianzi::Type>(m, "MianziType")
        .value("shunzi", Mianzi::Type::shunzi)
        .value("kezi", Mianzi::Type::kezi)
        .value("duizi", Mianzi::Type::duizi)
        .export_values();

    py::enum_<Mianzi::FuluType>(m, "FuluType")
        .value("none", Mianzi::FuluType::none)
        .value("chi", Mianzi::FuluType::chi)
        .value("peng", Mianzi::FuluType::peng)
        .value("minggang", Mianzi::FuluType::minggang)
        .value("angang", Mianzi::FuluType::angang)
        .export_values();

    // Mianzi クラスのバインディング
    py::class_<Mianzi>(m, "Mianzi")
        .def(py::init<Mianzi::Type, int>(), py::arg("type"), py::arg("pai_34"))
        .def(py::init<Mianzi::FuluType, int>(), py::arg("fulu_type"), py::arg("pai_34"))
        .def("__repr__", &Mianzi::to_string)
        .def_readwrite("type", &Mianzi::type)
        .def_readwrite("fulu_type", &Mianzi::fulu_type)
        .def_readwrite("pai_34", &Mianzi::pai_34);

    // Hupai クラスのバインディング
    py::class_<Hupai>(m, "Hupai")
        .def(py::init<>())
        .def("sum", &Hupai::sum)
        .def("tolist", &Hupai::tolist)
        .def_readwrite("立直", &Hupai::立直)
        .def_readwrite("一発", &Hupai::一発)
        .def_readwrite("門前清自摸和", &Hupai::門前清自摸和)
        .def_readwrite("平和", &Hupai::平和)
        .def_readwrite("一盃口", &Hupai::一盃口)
        .def_readwrite("断么九", &Hupai::断么九)
        .def_readwrite("場風_東", &Hupai::場風_東)
        .def_readwrite("場風_南", &Hupai::場風_南)
        .def_readwrite("場風_西", &Hupai::場風_西)
        .def_readwrite("場風_北", &Hupai::場風_北)
        .def_readwrite("自風_東", &Hupai::自風_東)
        .def_readwrite("自風_南", &Hupai::自風_南)
        .def_readwrite("自風_西", &Hupai::自風_西)
        .def_readwrite("自風_北", &Hupai::自風_北)
        .def_readwrite("役牌_白", &Hupai::役牌_白)
        .def_readwrite("役牌_發", &Hupai::役牌_發)
        .def_readwrite("役牌_中", &Hupai::役牌_中)
        .def_readwrite("槍槓", &Hupai::槍槓)
        .def_readwrite("嶺上開花", &Hupai::嶺上開花)
        .def_readwrite("海底摸月", &Hupai::海底摸月)
        .def_readwrite("河底撈魚", &Hupai::河底撈魚)
        .def_readwrite("両立直", &Hupai::両立直)
        .def_readwrite("七対子", &Hupai::七対子)
        .def_readwrite("混全帯么九", &Hupai::混全帯么九)
        .def_readwrite("一気通貫", &Hupai::一気通貫)
        .def_readwrite("三色同順", &Hupai::三色同順)
        .def_readwrite("三色同刻", &Hupai::三色同刻)
        .def_readwrite("三槓子", &Hupai::三槓子)
        .def_readwrite("対々和", &Hupai::対々和)
        .def_readwrite("三暗刻", &Hupai::三暗刻)
        .def_readwrite("小三元", &Hupai::小三元)
        .def_readwrite("混老頭", &Hupai::混老頭)
        .def_readwrite("純全帯么九", &Hupai::純全帯么九)
        .def_readwrite("二盃口", &Hupai::二盃口)
        .def_readwrite("混一色", &Hupai::混一色)
        .def_readwrite("清一色", &Hupai::清一色)
        .def_readwrite("天和", &Hupai::天和)
        .def_readwrite("地和", &Hupai::地和)
        .def_readwrite("大三元", &Hupai::大三元)
        .def_readwrite("四暗刻", &Hupai::四暗刻)
        .def_readwrite("四暗刻単騎", &Hupai::四暗刻単騎)
        .def_readwrite("字一色", &Hupai::字一色)
        .def_readwrite("緑一色", &Hupai::緑一色)
        .def_readwrite("清老頭", &Hupai::清老頭)
        .def_readwrite("国士無双", &Hupai::国士無双)
        .def_readwrite("国士無双１３面", &Hupai::国士無双１３面)
        .def_readwrite("小四喜", &Hupai::小四喜)
        .def_readwrite("大四喜", &Hupai::大四喜)
        .def_readwrite("四槓子", &Hupai::四槓子)
        .def_readwrite("九蓮宝燈", &Hupai::九蓮宝燈)
        .def_readwrite("純正九蓮宝燈", &Hupai::純正九蓮宝燈);

    // HuleOption のバインディング
    py::class_<HuleOption>(m, "HuleOption")
        .def(py::init<int, int>(), py::arg("zhuangfeng"), py::arg("lunban"))
        .def_readwrite("is_menqian", &HuleOption::is_menqian)
        .def_readwrite("is_lizhi", &HuleOption::is_lizhi)
        .def_readwrite("is_shuanglizhi", &HuleOption::is_shuanglizhi)
        .def_readwrite("is_yifa", &HuleOption::is_yifa)
        .def_readwrite("is_haidi", &HuleOption::is_haidi)
        .def_readwrite("is_lingshang", &HuleOption::is_lingshang)
        .def_readwrite("is_qianggang", &HuleOption::is_qianggang)
        .def_readwrite("is_init_turn_and_no_call", &HuleOption::is_init_turn_and_no_call)
        .def_readwrite("zhuangfeng", &HuleOption::zhuangfeng)
        .def_readwrite("lunban", &HuleOption::lunban);

    // Hule クラスのバインディング
    py::class_<Hule>(m, "Hule")
        .def(py::init<Shoupai&, const Action&, const HuleOption&>(),
             py::arg("shoupai"), py::arg("action"), py::arg("option"))
        .def_readwrite("hand", &Hule::hand)
        .def_readwrite("shoupai", &Hule::shoupai)
        .def_readwrite("fu", &Hule::fu)
        .def_readwrite("fanshu", &Hule::fanshu)
        .def_readwrite("damanguan", &Hule::damanguan)
        .def_readwrite("option", &Hule::option)
        .def_readwrite("hupai", &Hule::hupai)
        .def_readwrite("has_hupai", &Hule::has_hupai)
        .def_readwrite("hule_pai", &Hule::hule_pai)
        .def_readwrite("is_zimohu", &Hule::is_zimohu);

    // Shoupai クラスのバインディング
    py::class_<Shoupai>(m, "Shoupai")
        .def(py::init<>())
        .def(py::init<const std::array<int, 34>&>(), py::arg("bing"))
        .def(py::init<const std::array<int, 34>&, const std::vector<Mianzi>&>(),
             py::arg("bing"), py::arg("fulu"))
        .def_readwrite("bing", &Shoupai::bing)
        .def_readwrite("fulu", &Shoupai::fulu)
        .def_readwrite("xiangting", &Shoupai::xiangting)
        .def_readwrite("mode", &Shoupai::mode)
        .def_property(
            "tingpai",
            [](const Shoupai& shoupai) {
                return bitset34_to_mask(shoupai.tingpai);
            },
            [](Shoupai& shoupai, uint64_t mask) {
                shoupai.tingpai = mask_to_bitset34(mask);
            })
        .def_property(
            "red",
            [](const Shoupai& shoupai) {
                return bitset3_to_mask(shoupai.red);
            },
            [](Shoupai& shoupai, uint32_t mask) {
                shoupai.red = mask_to_bitset3(mask);
            })
        .def("apply", &Shoupai::apply)
        .def("update", &Shoupai::update)
        .def("tingpai_mask", [](const Shoupai& shoupai) {
            return bitset34_to_mask(shoupai.tingpai);
        })
        .def("tingpai_list", [](const Shoupai& shoupai) {
            std::vector<int> out;
            out.reserve(34);
            for (int i = 0; i < 34; ++i) {
                if (shoupai.tingpai.test(i)) out.push_back(i);
            }
            return out;
        })
        .def("__repr__", &Shoupai::to_string);

    m.def("wait_mask", [](const std::array<int, 34>& hand, int meld_count, bool three_player) {
        return wait_mask_impl(hand, meld_count, three_player);
    }, py::arg("hand"), py::arg("meld_count"), py::arg("three_player") = false);

    m.def("has_riichi_discard", [](const std::array<int, 34>& hand, int meld_count, bool three_player) {
        return has_riichi_discard_impl(hand, meld_count, three_player);
    }, py::arg("hand"), py::arg("meld_count"), py::arg("three_player") = false);

    m.def("has_hupai",
          [](const std::array<int, 34>& hand,
             const std::vector<std::pair<int, int>>& melds,
             int win_tile,
             bool is_tsumo,
             bool is_menqian,
             bool is_riichi,
             int zhuangfeng,
             int lunban,
             bool is_haidi,
             bool is_lingshang,
             bool is_qianggang) {
              return has_hupai_impl_ex(
                  hand,
                  melds,
                  win_tile,
                  is_tsumo,
                  is_menqian,
                  is_riichi,
                  zhuangfeng,
                  lunban,
                  is_haidi,
                  is_lingshang,
                  is_qianggang);
          },
          py::arg("hand"),
          py::arg("melds"),
          py::arg("win_tile"),
          py::arg("is_tsumo"),
          py::arg("is_menqian"),
          py::arg("is_riichi"),
          py::arg("zhuangfeng"),
          py::arg("lunban"),
          py::arg("is_haidi"),
          py::arg("is_lingshang"),
          py::arg("is_qianggang"));

    m.def("has_hupai_multi",
          [](const std::vector<std::tuple<
                 std::array<int, 34>,
                 std::vector<std::pair<int, int>>,
                 int,
                 bool,
                 bool,
                 bool,
                 int,
                 int,
                 bool,
                 bool,
                 bool>>& cases) {
              std::vector<bool> out;
              out.reserve(cases.size());
              for (const auto& c : cases) {
                  out.push_back(has_hupai_impl_ex(
                      std::get<0>(c),
                      std::get<1>(c),
                      std::get<2>(c),
                      std::get<3>(c),
                      std::get<4>(c),
                      std::get<5>(c),
                      std::get<6>(c),
                      std::get<7>(c),
                      std::get<8>(c),
                      std::get<9>(c),
                      std::get<10>(c)));
              }
              return out;
          },
          py::arg("cases"));

    m.def("evaluate_draw",
          [](const std::array<int, 34>& hand,
             const std::vector<std::pair<int, int>>& melds,
             int win_tile,
             bool is_menqian,
             bool is_riichi,
             int zhuangfeng,
             int lunban,
             int closed_kans,
             bool check_riichi_discard,
             bool is_haidi,
             bool is_lingshang,
             bool three_player) {
              bool can_tsumo = has_hupai_impl_ex(
                  hand,
                  melds,
                  win_tile,
                  true,
                  is_menqian,
                  is_riichi,
                  zhuangfeng,
                  lunban,
                  is_haidi,
                  is_lingshang,
                  false);
              bool can_riichi_discard = false;

              if (check_riichi_discard) {
                  can_riichi_discard = has_riichi_discard_impl(hand, closed_kans, three_player);
              }

              return py::make_tuple(can_tsumo, can_riichi_discard);
          },
          py::arg("hand"),
          py::arg("melds"),
          py::arg("win_tile"),
          py::arg("is_menqian"),
          py::arg("is_riichi"),
          py::arg("zhuangfeng"),
          py::arg("lunban"),
          py::arg("closed_kans"),
          py::arg("check_riichi_discard"),
          py::arg("is_haidi"),
          py::arg("is_lingshang"),
          py::arg("three_player") = false);

    m.attr("SELF_OPT_TSUMO") = SELF_OPT_TSUMO;
    m.attr("SELF_OPT_RIICHI") = SELF_OPT_RIICHI;
    m.attr("SELF_OPT_ANKAN") = SELF_OPT_ANKAN;
    m.attr("SELF_OPT_KAKAN") = SELF_OPT_KAKAN;
    m.attr("SELF_OPT_KYUSHUKYUHAI") = SELF_OPT_KYUSHUKYUHAI;
    m.attr("SELF_OPT_PENUKI") = SELF_OPT_PENUKI;
    m.attr("REACT_OPT_RON") = REACT_OPT_RON;
    m.attr("REACT_OPT_CHI") = REACT_OPT_CHI;
    m.attr("REACT_OPT_PON") = REACT_OPT_PON;
    m.attr("REACT_OPT_MINKAN") = REACT_OPT_MINKAN;

    m.def("compute_self_option_mask",
          &compute_self_option_mask_impl,
          py::arg("hand"),
          py::arg("melds"),
          py::arg("drawn_tile"),
          py::arg("is_riichi"),
          py::arg("score"),
          py::arg("zhuangfeng"),
          py::arg("lunban"),
          py::arg("live_draws_left"),
          py::arg("closed_kans"),
          py::arg("open_melds"),
          py::arg("open_pon_tiles"),
          py::arg("is_first_turn"),
          py::arg("first_turn_open_calls_seen"),
          py::arg("is_gangzimo"),
          py::arg("three_player") = false);

    m.def("compute_reaction_option_masks",
          &compute_reaction_option_masks_impl,
          py::arg("players"),
          py::arg("discarder"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("live_draws_left"),
          py::arg("last_draw_was_gangzimo"),
          py::arg("three_player") = false);

    m.def("compute_reaction_option_masks_cached",
          &compute_reaction_option_masks_cached_impl,
          py::arg("players"),
          py::arg("discarder"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("live_draws_left"),
          py::arg("last_draw_was_gangzimo"));

    m.def("compute_rob_kan_option_masks",
          &compute_rob_kan_option_masks_impl,
          py::arg("players"),
          py::arg("actor"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("require_kokushi"),
          py::arg("three_player") = false);

    m.def("compute_rob_kan_option_masks_cached",
          &compute_rob_kan_option_masks_cached_impl,
          py::arg("players"),
          py::arg("actor"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("require_kokushi"));

    m.def("compute_self_option_mask_shoupai",
          &compute_self_option_mask_shoupai_impl,
          py::arg("shoupai"),
          py::arg("drawn_tile"),
          py::arg("is_riichi"),
          py::arg("score"),
          py::arg("zhuangfeng"),
          py::arg("lunban"),
          py::arg("live_draws_left"),
          py::arg("closed_kans"),
          py::arg("open_melds"),
          py::arg("open_pon_tiles"),
          py::arg("is_first_turn"),
          py::arg("first_turn_open_calls_seen"),
          py::arg("is_gangzimo"),
          py::arg("three_player") = false);

    m.def("compute_reaction_option_masks_shoupai",
          &compute_reaction_option_masks_shoupai_impl,
          py::arg("players"),
          py::arg("discarder"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("live_draws_left"),
          py::arg("last_draw_was_gangzimo"),
          py::arg("three_player") = false);

    m.def("compute_rob_kan_option_masks_shoupai",
          &compute_rob_kan_option_masks_shoupai_impl,
          py::arg("players"),
          py::arg("actor"),
          py::arg("tile_idx"),
          py::arg("zhuangfeng"),
          py::arg("dealer_seat"),
          py::arg("require_kokushi"),
          py::arg("three_player") = false);

    // Action の enum バインディング
    py::enum_<Action::Type>(m, "ActionType")
    	.value("zimo", Action::zimo)
    	.value("dapai", Action::dapai)
    	.value("lizhi", Action::lizhi)
    	.value("chi", Action::chi)
    	.value("peng", Action::peng)
    	.value("minggang", Action::minggang)
    	.value("angang", Action::angang)
    	.value("jiagang", Action::jiagang)
    	.value("zimohu", Action::zimohu)
    	.value("ronghu", Action::ronghu)
    	.value("pingju", Action::pingju)
    	.export_values();

	// Action クラスのバインディング
	py::class_<Action>(m, "Action")
    	.def(py::init<Action::Type, int>(), py::arg("type"), py::arg("pai_34"))
    	.def(py::init<Action::Type, int, bool>(), py::arg("type"), py::arg("pai_34"), py::arg("red"))
    	.def(py::init<Action::Type, int, bool, int>(), py::arg("type"), py::arg("pai_34"), py::arg("red"), py::arg("bias"))
    	.def_readwrite("type", &Action::type)
    	.def_readwrite("pai_34", &Action::pai_34)
    	.def_readwrite("red", &Action::red)
    	.def_readwrite("bias", &Action::bias);
}
