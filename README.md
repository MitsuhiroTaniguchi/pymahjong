# pymahjong

[English](README.md) | [日本語](README.ja.md)

`pymahjong` is a low-level Python binding for mahjong hand logic implemented in C++. It focuses on fast primitives for hand state, shanten calculation, win evaluation, and action-option queries.

This repository is a good fit when you are building:

- a mahjong AI or simulator
- a rules engine
- tile-efficiency or hand-analysis tooling

## What This Library Does

- Represent a hand with closed tiles, open melds, and red-tile state
- Compute shanten and wait masks
- Evaluate a win and return fu, han, yakuman, and yaku names
- Answer "what can the current player do now?"
- Answer "who can react to this discard or kan?"
- Support both four-player and three-player paths

## Before You Read the API

Most APIs use the same conventions.

### Tile indices

Tiles use a 34-index layout:

- `0-8`: `m1-m9`
- `9-17`: `p1-p9`
- `18-26`: `s1-s9`
- `27-33`: `east, south, west, north, white, green, red`

### Winds

- `zhuangfeng`: round wind
- `lunban`: seat wind of the acting or winning player

Wind encoding is:

- `0 = east`
- `1 = south`
- `2 = west`
- `3 = north`

### Bit masks

Bit masks use bit `n` for tile index `n`.

Examples:

- a wait mask tells you which draws complete a tenpai hand
- `SELF_OPT_*` flags tell you what the current player can do
- `REACT_OPT_*` flags tell you how another player can react

## Installation

Build requirements:

- C++17-capable compiler
- CMake
- `pybind11`

Install:

```bash
python3 -m pip install .
```

Editable install with test dependencies:

```bash
python3 -m pip install -e .[test]
```

## Typical Workflows

### 1. Work with a mutable hand: `Shoupai`

Use `Shoupai` when you want an object you can update over time.

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = 1
counts[1] = 1
counts[2] = 1
counts[9] = 2

hand = pm.Shoupai(tuple(counts))
hand.update()

print(hand.xiangting)
print(hand.tingpai_list())
```

Use this path if your application keeps a live game state.

### 2. Get shanten quickly: `Xiangting`

Use `Xiangting` when you want a fast stateless solver call on raw tile counts.

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = counts[1] = counts[2] = 1
counts[9] = counts[10] = counts[11] = 1
counts[18] = counts[19] = counts[20] = 1
counts[27] = 2
counts[31] = 2

x = pm.Xiangting()
shanten, mode, discard_mask, wait_mask = x.calculate(
    tuple(counts),
    4,
    7,
    False,
    False,
)
```

Returned values:

- `shanten`: current shanten number
- `mode`: internal solver mode
- `discard_mask`: candidate discard tiles
- `wait_mask`: effective draws after the best discard logic

### 3. Evaluate a win: `Hule`

Use `Hule` when you already know the winning tile and want scoring information.

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

result = pm.Hule(
    pm.Shoupai(tuple(counts)),
    pm.Action(pm.ActionType.ronghu, 10),
    option,
)

print(result.has_hupai)
print(result.fu, result.fanshu, result.damanguan)
print(result.hupai.tolist())
```

### 4. Ask what actions are available now

Use `compute_self_option_mask()` after a draw.

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

## How To Choose an API

- Use `Shoupai` if your program maintains a mutable hand object.
- Use `Xiangting.calculate()` if you only need fast shanten and waits from raw counts.
- Use `Hule` if you need fu, han, yaku, and yakuman details.
- Use `wait_mask()` if you only want waits for a tenpai hand.
- Use `has_hupai()` if you only need a yes or no win check.
- Use `compute_self_option_mask()` and `compute_reaction_option_masks()` for game-flow logic.

## Main Types

### `Shoupai`

Mutable hand state.

Constructor forms:

- `Shoupai(bing)`
- `Shoupai(bing, fulu)`

Important fields and methods:

- `bing`: closed tile counts as a length-34 array
- `fulu`: open melds as a list of `Mianzi`
- `xiangting`: cached shanten after `update()`
- `mode`: cached solver mode after `update()`
- `tingpai`: wait mask as an integer
- `red`: red-five state as a 3-bit integer for manzu, pinzu, souzu
- `apply(action)`: mutates the hand by applying an `Action`
- `update()`: recomputes `xiangting`, `mode`, and `tingpai`
- `tingpai_mask()`: returns the wait mask
- `tingpai_list()`: returns wait tile indices as a Python list

`Shoupai` is the right abstraction when you are simulating draws, discards, calls, and state transitions.

### `Xiangting`

Stateless shanten solver wrapper.

Primary method:

- `calculate(hand, size, mode, check_hand=False, three_player=False)`

Argument meaning:

- `hand`: length-34 tile counts
- `size`: number of meld slots still to complete
- `mode`: solver mode used by the native implementation
- `check_hand`: validates impossible tile totals before solving
- `three_player`: switches to the sanma logic path

For a standard hand, `size` is usually `4`. If you already have open melds, the effective remaining meld count is smaller.

### `HuleOption`

Round-context flags used by win evaluation.

Important fields:

- `is_menqian`: hand is closed
- `is_lizhi`: riichi declared
- `is_shuanglizhi`: double riichi declared
- `is_yifa`: ippatsu still active
- `is_haidi`: last-draw or last-discard win situation
- `is_lingshang`: rinshan win
- `is_qianggang`: chankan win
- `is_init_turn_and_no_call`: first uninterrupted go-around, used for tenhou/chiihou style checks
- `zhuangfeng`: round wind
- `lunban`: seat wind of the winner

This object describes table context, not hand shape.

### `Hule`

Win evaluation result.

Constructor:

- `Hule(shoupai, action, option)`

Important outputs:

- `has_hupai`: whether the hand is valid as a win
- `fu`: fu count
- `fanshu`: han count
- `damanguan`: yakuman multiplier count
- `hupai`: `Hupai` object with named yaku flags
- `hule_pai`: winning tile index
- `is_zimohu`: whether the result was evaluated as tsumo

Use `hupai.tolist()` when you want readable `(name, han)` output.

### `Mianzi`

Represents one meld or block.

Constructor forms:

- `Mianzi(MianziType, pai_34)`
- `Mianzi(FuluType, pai_34)`

Important fields:

- `type`: sequence, triplet, or pair
- `fulu_type`: chi, pon, open kan, closed kan, or none
- `pai_34`: base tile index

Meaning of `pai_34`:

- for a sequence, it is the lowest tile
- for a pair, pon, or kan, it is the repeated tile

### `Action`

Represents one action applied to a hand or one winning event.

Constructor forms:

- `Action(ActionType, pai_34)`
- `Action(ActionType, pai_34, red)`
- `Action(ActionType, pai_34, red, bias)`

Common action types:

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

Important fields:

- `pai_34`: tile touched by the action
- `red`: whether that tile is a red five
- `bias`: sequence offset used for `chi`

`bias` matters mainly for `chi`. It tells the hand which position the called tile occupies inside the sequence.

## Main Helper Functions

### Hand-state helpers

- `wait_mask(hand, meld_count, three_player=False)`
  Returns the wait mask for a tenpai hand. Non-tenpai shapes return `0`.
- `has_riichi_discard(hand, meld_count, three_player=False)`
  Returns whether at least one discard leaves the hand in tenpai.
- `has_hupai(...)`
  Returns whether a specific win setup is valid.
- `evaluate_draw(...)`
  Returns `(can_tsumo, can_riichi_discard)` for a draw event.

### Game-flow helpers

- `compute_self_option_mask(...)`
  Returns what the current player can do after drawing.
- `compute_reaction_option_masks(...)`
  Returns `(seat, mask)` pairs for reactions to a discard.
- `compute_rob_kan_option_masks(...)`
  Returns `(seat, mask)` pairs for reactions to a kan.

Inputs that often confuse readers:

- `meld_count`: current number of melds already formed in the hand
- `open_melds`: number of open melds; used for menzen and riichi checks
- `closed_kans`: number of concealed kans already present
- `open_pon_tiles`: tile indices of pon melds that may be upgraded to kakan
- `is_first_turn`: whether the hand is still in the first uninterrupted round of turns
- `first_turn_open_calls_seen`: whether any open call has already broken that first-turn state
- `dealer_seat`: absolute dealer seat index, used internally to derive each player's seat wind
- `players`: per-player tuples carrying hand state plus riichi and furiten context

## Constants

Self-action bits:

- `SELF_OPT_TSUMO`
- `SELF_OPT_RIICHI`
- `SELF_OPT_ANKAN`
- `SELF_OPT_KAKAN`
- `SELF_OPT_KYUSHUKYUHAI`
- `SELF_OPT_PENUKI`

Reaction bits:

- `REACT_OPT_RON`
- `REACT_OPT_CHI`
- `REACT_OPT_PON`
- `REACT_OPT_MINKAN`

## Repository Layout

- `src/bindings.cpp`: Python bindings and helper entry points
- `src/calsht_dw.cpp`: shanten logic
- `src/hule.hpp`: win evaluation and scoring
- `src/shoupai.hpp`: mutable hand representation
- `tests/test_regressions.py`: regression tests

## Development

Run tests:

```bash
python3 -m pytest
```

## Notes

- This is intentionally a low-level API. Most applications should build a small wrapper layer on top.
- The README documents conventions and usage, but not every internal solver mode.
