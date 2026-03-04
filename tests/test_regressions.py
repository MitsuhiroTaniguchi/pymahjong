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
