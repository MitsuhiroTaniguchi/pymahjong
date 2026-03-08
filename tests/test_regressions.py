import pymahjong as pm
import pytest


def _hule(counts, win_tile, *, menqian=True, lizhi=False):
    option = pm.HuleOption(0, 0)
    option.is_menqian = menqian
    option.is_lizhi = lizhi
    return pm.Hule(pm.Shoupai(tuple(counts)), pm.Action(pm.ActionType.ronghu, win_tile), option)


def _hule_open(counts, melds, win_tile):
    option = pm.HuleOption(0, 0)
    option.is_menqian = False
    shoupai = pm.Shoupai(tuple(counts), melds)
    return pm.Hule(shoupai, pm.Action(pm.ActionType.ronghu, win_tile), option)


def test_shoupai_bitset_properties_are_python_accessible():
    s = pm.Shoupai(tuple([0] * 34))
    s.update()

    assert isinstance(s.tingpai, int)
    assert isinstance(s.red, int)

    s.tingpai = 0b101
    s.red = 0b011
    assert s.tingpai == 0b101
    assert s.red == 0b011


def test_jiagang_consumes_concealed_tile():
    counts = [0] * 34
    counts[0] = 3
    shoupai = pm.Shoupai(tuple(counts))

    shoupai.apply(pm.Action(pm.ActionType.peng, 0))
    assert shoupai.bing[0] == 1

    shoupai.apply(pm.Action(pm.ActionType.jiagang, 0))
    assert shoupai.bing[0] == 0
    assert len(shoupai.fulu) == 1
    assert shoupai.fulu[0].fulu_type == pm.FuluType.minggang


def test_jiagang_red_tile_clears_red_flag():
    # Build 5m x3 with one red tile, call peng without consuming red,
    # then upgrade to kakan using the red tile.
    p = 4  # 5m
    s = pm.Shoupai(tuple([0] * 34))
    s.apply(pm.Action(pm.ActionType.zimo, p, True))   # red 5m
    s.apply(pm.Action(pm.ActionType.zimo, p, False))
    s.apply(pm.Action(pm.ActionType.zimo, p, False))
    assert s.red == 0b001

    s.apply(pm.Action(pm.ActionType.peng, p, False))
    assert s.bing[p] == 1
    assert s.red == 0b001

    s.apply(pm.Action(pm.ActionType.jiagang, p, True))
    assert s.bing[p] == 0
    assert s.red == 0


def test_shanpon_ron_uses_open_triplet_fu():
    # m234 p222 p567 p99 z666, ron on p2
    counts = [0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0]
    h = _hule(counts, 10, menqian=True, lizhi=False)
    assert h.has_hupai
    assert h.fanshu == 1
    assert h.fu == 40


def test_chiitoitsu_detects_high_honor_bits():
    # Includes 發(32) so 64-bit popcount correctness matters.
    counts = [0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0]
    h = _hule(counts, 5, menqian=True, lizhi=False)
    assert h.has_hupai
    assert ("七対子", 2) in h.hupai.tolist()
    assert h.fu == 25


def test_double_iipeikou_with_four_identical_sequences():
    # m777788889999 + p55, ron on m7
    counts = [0] * 34
    counts[6] = 4
    counts[7] = 4
    counts[8] = 4
    counts[13] = 2
    h = _hule(counts, 6, menqian=True, lizhi=True)
    assert h.has_hupai
    assert ("二盃口", 3) in h.hupai.tolist()
    assert h.fanshu == 4  # riichi + ryanpeikou


def test_ryuuiisou_without_hatsu():
    # s22333344466688 (all green tiles in souzu only)
    counts = [0] * 34
    counts[19] = 4  # s2
    counts[20] = 4  # s3
    counts[21] = 1  # s4
    counts[23] = 3  # s6
    counts[25] = 2  # s8
    h = _hule(counts, 19, menqian=True, lizhi=False)
    assert h.has_hupai
    assert ("緑一色", 1) in h.hupai.tolist()
    assert h.damanguan == 1


def test_open_ron_minimum_30_fu():
    # Open tanyao hand with no additional fu should still be 30 fu on ron.
    # Concealed: m22 s234 s456 s456 + open meld: p234
    counts = [0] * 34
    counts[1] = 2   # m2 pair
    counts[19] = 1  # s2
    counts[20] = 1  # s3
    counts[21] = 3  # s4
    counts[22] = 2  # s5
    counts[23] = 2  # s6
    melds = [pm.Mianzi(pm.FuluType.chi, 10)]  # p234
    h = _hule_open(counts, melds, 21)  # ron on s4, choose ryanmen decomposition
    assert h.has_hupai
    assert ("断么九", 1) in h.hupai.tolist()
    assert h.fu == 30


def test_check_hand_rejects_invalid_total_tiles():
    x = pm.Xiangting()
    with pytest.raises(ValueError):
        x.calculate(tuple([4] * 34), 4, 7, True, False)


def test_compute_self_option_mask_exposes_tsumo_riichi_and_kyushukyuhai():
    counts = [0] * 34
    for idx in [0, 0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33]:
        counts[idx] += 1
    mask = pm.compute_self_option_mask(
        tuple(counts),
        [],
        33,
        False,
        25000,
        0,
        0,
        10,
        0,
        0,
        [],
        True,
        False,
        False,
    )
    assert mask & pm.SELF_OPT_TSUMO
    assert mask & pm.SELF_OPT_RIICHI
    assert mask & pm.SELF_OPT_KYUSHUKYUHAI


def test_compute_self_option_mask_exposes_penuki_in_three_player() -> None:
    counts = [0] * 34
    for idx in [0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33]:
        counts[idx] += 1
    counts[30] += 1
    mask = pm.compute_self_option_mask(
        tuple(counts),
        [],
        30,
        False,
        35000,
        0,
        0,
        10,
        0,
        0,
        [],
        True,
        False,
        False,
        True,
    )
    assert mask & pm.SELF_OPT_PENUKI


def test_compute_reaction_option_masks_reports_chi_pon_and_ron():
    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {0, 2, 4} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 1 else 1 if i == 3 else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 1 else 1 if i == 4 else 0 for i in range(34)]), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 1, 0, 0, 10, False))
    assert options[1] & pm.REACT_OPT_CHI
    assert options[2] & pm.REACT_OPT_PON
    assert options[3] & pm.REACT_OPT_PON


def test_compute_reaction_option_masks_three_player_disables_chi_and_uses_three_seats() -> None:
    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {9, 11} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 10 else 1 if i == 12 else 0 for i in range(34)]), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 10, 0, 0, 10, False, True))
    assert 1 not in options
    assert options[2] & pm.REACT_OPT_PON


def test_compute_reaction_option_masks_cached_matches_three_player_stateless() -> None:
    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {9, 11} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 10 else 1 if i == 12 else 0 for i in range(34)]), [], False, False, False, 0, 0),
    ]
    cached_players = [
        (*player, pm.wait_mask(player[0], len(player[1]), True))
        for player in players
    ]
    assert pm.compute_reaction_option_masks_cached(cached_players, 0, 10, 0, 0, 10, False) == pm.compute_reaction_option_masks(
        players, 0, 10, 0, 0, 10, False, True
    )


def test_compute_reaction_option_masks_applies_tenhou_kuikae_to_chi_and_pon():
    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {0, 1, 2} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 1 else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 3, 0, 0, 10, False))
    assert 1 not in options

    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {1, 2, 4} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([2 if i == 1 else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 3, 0, 0, 10, False))
    assert options[1] & pm.REACT_OPT_CHI

    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([2 if i == 1 else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 1, 0, 0, 10, False))
    assert 1 not in options

    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {1, 2, 4} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 0, 0, 0, 10, False))
    assert options[1] & pm.REACT_OPT_CHI

    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {1, 2, 3} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks(players, 0, 0, 0, 0, 10, False))
    assert 1 not in options


def test_compute_reaction_option_masks_shoupai_applies_tenhou_kuikae_to_chi_and_pon():
    players = [
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
        (pm.Shoupai(tuple([1 if i in {0, 1, 2} else 0 for i in range(34)])), False, False, False, 0, 0),
        (pm.Shoupai(tuple([2 if i == 1 else 0 for i in range(34)])), False, False, False, 0, 0),
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks_shoupai(players, 0, 3, 0, 0, 10, False))
    assert 1 not in options

    players = [
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
        (pm.Shoupai(tuple([1 if i in {1, 2, 4} else 0 for i in range(34)])), False, False, False, 0, 0),
        (pm.Shoupai(tuple([2 if i == 1 else 0 for i in range(34)])), False, False, False, 0, 0),
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks_shoupai(players, 0, 3, 0, 0, 10, False))
    assert options[1] & pm.REACT_OPT_CHI

    players = [
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
        (pm.Shoupai(tuple([2 if i == 1 else 0 for i in range(34)])), False, False, False, 0, 0),
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
        (pm.Shoupai(tuple([0] * 34)), False, False, False, 0, 0),
    ]
    options = dict(pm.compute_reaction_option_masks_shoupai(players, 0, 1, 0, 0, 10, False))
    assert 1 not in options


def test_compute_rob_kan_option_masks_can_require_kokushi():
    players = [
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([1 if i in {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33} else 0 for i in range(34)]), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
        (tuple([0] * 34), [], False, False, False, 0, 0),
    ]
    options = dict(pm.compute_rob_kan_option_masks(players, 0, 33, 0, 0, True))
    assert options[1] & pm.REACT_OPT_RON
