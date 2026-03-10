# pymahjong

[English](README.md) | [日本語](README.ja.md)

`pymahjong` is a Python binding for fast mahjong hand processing, shanten calculation, win detection, and action-option evaluation. The core logic is implemented in C++ and exposed through `pybind11`.

The project is currently best suited as a low-level toolkit for mahjong AI, rules engines, and tile-efficiency experiments.

## Features

- Represent hands with open melds and red-tile state
- Compute shanten and effective waits
- Evaluate winning hands, fu, han, and yakuman
- Compute self-action option masks such as tsumo, riichi, ankan, kakan, and nine terminals abort
- Compute reaction masks for chi, pon, ron, and open kan
- Support both four-player and three-player rule paths

## Installation

The build requires a C++17-capable compiler, CMake, and `pybind11`.

```bash
python3 -m pip install .
```

For editable install with test dependencies:

```bash
python3 -m pip install -e .[test]
```

## Quick Start

### Build a hand

Hands are passed as a length-34 tile-count sequence.

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = 1   # 1m
counts[1] = 1   # 2m
counts[2] = 1   # 3m
counts[9] = 2   # pair of 1p

shoupai = pm.Shoupai(tuple(counts))
shoupai.update()

print(shoupai.xiangting)
print(shoupai.tingpai_list())
```

### Calculate shanten

`Xiangting.calculate()` returns `(shanten, mode, discard_mask, wait_mask)`.

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = counts[1] = counts[2] = 1
counts[9] = counts[10] = counts[11] = 1
counts[18] = counts[19] = counts[20] = 1
counts[27] += 2
counts[31] += 2

x = pm.Xiangting()
shanten, mode, discard_mask, wait_mask = x.calculate(
    tuple(counts),
    4,
    7,
    False,
    False,
)
```

Arguments:

- `hand`: length-34 tile counts
- `size`: number of meld slots to complete, typically `4` for a standard hand
- `mode`: search mode used by the solver
- `check_hand`: validate tile-count legality before solving
- `three_player`: enable three-player rules

### Evaluate a winning hand

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = 1
counts[1] = 1
counts[2] = 1
counts[10] = 3
counts[13] = 1
counts[14] = 1
counts[15] = 1
counts[17] = 2
counts[32] = 3

option = pm.HuleOption(0, 0)
option.is_menqian = True
option.is_lizhi = False

hule = pm.Hule(
    pm.Shoupai(tuple(counts)),
    pm.Action(pm.ActionType.ronghu, 10),
    option,
)

print(hule.has_hupai)
print(hule.fu, hule.fanshu)
print(hule.hupai.tolist())
```

### Compute self action options

```python
import pymahjong as pm

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

can_tsumo = bool(mask & pm.SELF_OPT_TSUMO)
can_riichi = bool(mask & pm.SELF_OPT_RIICHI)
```

## Main API

Conventions used throughout the API:

- Tile indices use a 34-tile layout: `0-8 = m1-m9`, `9-17 = p1-p9`, `18-26 = s1-s9`, `27-33 = east, south, west, north, white, green, red`.
- `zhuangfeng` is the round wind as `0=east`, `1=south`, `2=west`, `3=north`.
- `lunban` is the player's seat wind in the same encoding, not an absolute seat id.
- Bit masks use bit `n` to represent tile index `n`.

### `Shoupai`

Represents a hand state.

- `Shoupai(bing)`: construct from a length-34 closed-hand count array
- `Shoupai(bing, fulu)`: construct from counts plus a list of melds
- `bing`: tile counts
- `fulu`: open meld list as `Mianzi` objects
- `xiangting`: cached shanten value after `update()`
- `mode`: cached solver mode
- `tingpai`: 34-bit wait mask exposed as an integer
- `red`: 3-bit red-tile mask exposed as an integer, one bit per suit for red 5m, 5p, 5s
- `apply(action)`: mutate the hand by applying an `Action`
- `update()`: recompute cached shanten and waits
- `tingpai_mask()`: return wait mask as an integer
- `tingpai_list()`: return wait tiles as a Python list

Use this when you want a mutable object-oriented hand representation instead of passing raw tuples to stateless helpers.

### `Xiangting`

Shanten solver entry point.

- `calculate(hand, size, mode, check_hand=False, three_player=False)`

Returns a tuple of:

- `shanten`: current shanten number
- `mode`: internal resolution mode selected by the solver
- `discard_mask`: bit mask of tiles that are valid discard candidates
- `wait_mask`: bit mask of effective draws after the best discard logic

In practice, this is the low-level solver API. `size` is usually `4` for a standard concealed hand, and decreases implicitly when you already have open melds.

### `HuleOption`

Carries rule and situation flags for hand evaluation.

Important fields:

- `is_menqian`
- `is_lizhi`
- `is_shuanglizhi`
- `is_yifa`
- `is_haidi`
- `is_lingshang`
- `is_qianggang`
- `is_init_turn_and_no_call`
- `zhuangfeng`
- `lunban`

Field meaning:

- `is_menqian`: the hand is closed
- `is_lizhi`: riichi was declared
- `is_shuanglizhi`: double riichi was declared
- `is_yifa`: ippatsu is still active
- `is_haidi`: last live tile draw or last discard situation
- `is_lingshang`: the win is on a rinshan draw
- `is_qianggang`: the win is by robbing a kan
- `is_init_turn_and_no_call`: still the uninterrupted first go-around, used for tenhou/chiihou style checks
- `zhuangfeng`: round wind
- `lunban`: winner's seat wind

Set these before constructing `Hule`. This object carries table context rather than hand shape.

### `Hule`

Represents the result of a win evaluation.

- `Hule(shoupai, action, option)`
- `has_hupai`: whether the hand is actually winning
- `fu`: fu value
- `fanshu`: han value
- `damanguan`: yakuman multiplier count
- `hupai`: `Hupai` object containing named yaku flags
- `is_zimohu`: whether the evaluation was tsumo-based
- `hule_pai`: winning tile index

`Hule` evaluates the best available interpretation of the hand shape, including fu calculation and yaku selection. For readable yaku output, use `hule.hupai.tolist()`, which returns `(name, han)` pairs.

### `Mianzi`

Represents a meld or block.

- `Mianzi(MianziType, pai_34)`
- `Mianzi(FuluType, pai_34)`
- `type`: sequence, triplet, or pair type
- `fulu_type`: chi, pon, open kan, closed kan, or none
- `pai_34`: base tile index. For a sequence this is the lowest tile, for a pon/kan/pair it is the repeated tile

Useful when building open hands manually.

### `Action`

Represents a player action.

Constructors:

- `Action(ActionType, pai_34)`
- `Action(ActionType, pai_34, red)`
- `Action(ActionType, pai_34, red, bias)`

Common `ActionType` values:

- `zimo`
- `dapai`
- `lizhi`
- `chi`
- `peng`
- `minggang`
- `angang`
- `jiagang`
- `zimohu`
- `ronghu`

Field meaning:

- `pai_34`: tile index the action refers to
- `red`: whether the acted tile is a red five
- `bias`: sequence offset used by `chi`; if the called tile is the middle tile of a sequence this is typically `1`

Use `Action` with `Shoupai.apply()` and `Hule(...)`. In practice, `bias` only matters for `chi`.

### Stateless helper functions

- `wait_mask(hand, meld_count, three_player=False)`
  Returns the wait mask for the given hand shape. It only returns non-zero when the hand is already in tenpai.
- `has_riichi_discard(hand, meld_count, three_player=False)`
  Returns whether at least one discard leaves the hand in tenpai, which is the key precondition for riichi.
- `has_hupai(hand, melds, win_tile, is_tsumo, is_menqian, is_riichi, zhuangfeng, lunban, is_haidi, is_lingshang, is_qianggang)`
  Returns whether the specified win condition is valid.
- `evaluate_draw(...)`
  Returns `(can_tsumo, can_riichi_discard)` for a draw event.
- `compute_self_option_mask(...)`
  Returns a bit mask of actions available to the current player after drawing a tile.
- `compute_reaction_option_masks(players, discarder, tile_idx, zhuangfeng, dealer_seat, live_draws_left, last_draw_was_gangzimo, three_player=False)`
  Returns a list of `(seat, mask)` pairs for reactions to a discard.
- `compute_rob_kan_option_masks(...)`
  Returns reaction masks for robbing a kan.

Important inputs for the option-mask helpers:

- `meld_count`: number of already-open melds in the hand
- `open_melds`: number of open melds; used to decide whether riichi or menzen-only checks apply
- `closed_kans`: number of concealed kans already present
- `open_pon_tiles`: tile indices for pon melds that could potentially be upgraded to kakan
- `is_first_turn`: whether the table is still on the first uninterrupted turn
- `first_turn_open_calls_seen`: whether any open call has already broken the first-turn state
- `players` in reaction helpers: per-player tuples including hand, melds, riichi/furiten state, furiten mask, and open meld count
- `dealer_seat`: absolute seat index of the dealer, used to convert seats into `lunban` wind values during win checks

### Bit-mask constants

Self options:

- `SELF_OPT_TSUMO`
- `SELF_OPT_RIICHI`
- `SELF_OPT_ANKAN`
- `SELF_OPT_KAKAN`
- `SELF_OPT_KYUSHUKYUHAI`
- `SELF_OPT_PENUKI`

Reaction options:

- `REACT_OPT_RON`
- `REACT_OPT_CHI`
- `REACT_OPT_PON`
- `REACT_OPT_MINKAN`

## Development

Run tests with:

```bash
python3 -m pytest
```

Build configuration lives in `CMakeLists.txt` and `setup.py`. Package data includes `data/index_dw_s.bin` and `data/index_dw_h.bin`.

## Repository Guide

- `src/bindings.cpp`: Python bindings
- `src/calsht_dw.cpp`: shanten logic
- `pymahjong/__init__.py`: Python package entry point
- `tests/test_regressions.py`: regression tests
