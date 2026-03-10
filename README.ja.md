# pymahjong

[English](README.md) | [日本語](README.ja.md)

`pymahjong` は、麻雀の手牌処理、シャンテン計算、和了判定、行動可能判定を高速に行うための Python バインディングです。コアロジックは C++ で実装され、`pybind11` 経由で Python から利用できます。

現状の位置づけとしては、麻雀 AI、ルールエンジン、牌効率検証などで使う低レイヤ寄りのツールキットです。

## 主な機能

- 副露や赤牌状態を含む手牌表現
- シャンテン数と有効牌の計算
- 和了判定、符、翻、役満の評価
- ツモ時に選択可能な行動のビットマスク計算
- 打牌に対するチー、ポン、ロン、明槓などの反応可能判定
- 四麻と三麻の両方のルール分岐に対応

## インストール

ビルドには C++17 対応コンパイラ、CMake、`pybind11` が必要です。

```bash
python3 -m pip install .
```

テスト依存込みで editable install する場合:

```bash
python3 -m pip install -e .[test]
```

## クイックスタート

### 手牌を作る

手牌は長さ 34 の牌カウント配列で渡します。

```python
import pymahjong as pm

counts = [0] * 34
counts[0] = 1   # 1m
counts[1] = 1   # 2m
counts[2] = 1   # 3m
counts[9] = 2   # 1p の対子

shoupai = pm.Shoupai(tuple(counts))
shoupai.update()

print(shoupai.xiangting)
print(shoupai.tingpai_list())
```

### シャンテン数を計算する

`Xiangting.calculate()` は `(shanten, mode, discard_mask, wait_mask)` を返します。

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

引数:

- `hand`: 長さ 34 の牌カウント
- `size`: 完成させる面子数。通常手なら `4`
- `mode`: ソルバの探索モード
- `check_hand`: 手牌枚数の妥当性を事前検証するか
- `three_player`: 三麻ルールを使うか

### 和了判定をする

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

### ツモ時の行動可能判定を調べる

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

## 主要 API

API 全体で使う前提:

- 牌インデックスは 34 種表現です。`0-8 = m1-m9`, `9-17 = p1-p9`, `18-26 = s1-s9`, `27-33 = 東, 南, 西, 北, 白, 發, 中`。
- `zhuangfeng` は場風で、`0=東`, `1=南`, `2=西`, `3=北` です。
- `lunban` はプレイヤーの自風で、同じく `0=東`, `1=南`, `2=西`, `3=北` です。絶対 seat 番号そのものではありません。
- ビットマスクは bit `n` が牌インデックス `n` に対応します。

### `Shoupai`

手牌状態を表すクラスです。

- `Shoupai(bing)`: 閉じた手牌の 34 牌カウントから生成
- `Shoupai(bing, fulu)`: 牌カウントと副露リストから生成
- `bing`: 手牌カウント
- `fulu`: `Mianzi` オブジェクトの副露リスト
- `xiangting`: `update()` 後に保持されるシャンテン数
- `mode`: 更新時に使われたモード
- `tingpai`: 34 ビットの待ち牌マスクを整数として exposed
- `red`: 3 ビットの赤牌マスクを整数として exposed。各 bit は赤 5m, 赤 5p, 赤 5s に対応
- `apply(action)`: `Action` を適用して手牌を更新
- `update()`: シャンテン数と待ち牌情報を再計算
- `tingpai_mask()`: 待ち牌マスクを整数で返す
- `tingpai_list()`: 待ち牌インデックスを Python リストで返す

生のタプルを毎回関数に渡すより、ミュータブルな手牌オブジェクトとして扱いたいときに使います。

### `Xiangting`

シャンテン計算器です。

- `calculate(hand, size, mode, check_hand=False, three_player=False)`

返り値の内容:

- `shanten`: 現在のシャンテン数
- `mode`: ソルバ内部で選択された解決モード
- `discard_mask`: 打牌候補のビットマスク
- `wait_mask`: 最善打牌考慮後の有効牌ビットマスク

実質的には低レイヤのシャンテンソルバです。`size` は通常手なら `4` を使い、副露がある場合はその分だけ完成させる面子数が減る前提です。

### `HuleOption`

和了判定に必要な状況フラグを保持します。

主なフィールド:

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

各フィールドの意味:

- `is_menqian`: 門前か
- `is_lizhi`: 立直しているか
- `is_shuanglizhi`: ダブル立直か
- `is_yifa`: 一発が有効か
- `is_haidi`: 海底摸月または河底撈魚の局面か
- `is_lingshang`: 嶺上開花のツモか
- `is_qianggang`: 槍槓か
- `is_init_turn_and_no_call`: まだ誰も鳴いていない最初の一巡目か。天和・地和判定に使われます
- `zhuangfeng`: 場風
- `lunban`: 和了者の自風

`Hule` を作る前にこれらを設定します。手牌形そのものではなく、局面コンテキストを持つオブジェクトです。

### `Hule`

和了評価結果を表します。

- `Hule(shoupai, action, option)`
- `has_hupai`: 和了形として成立しているか
- `fu`: 符
- `fanshu`: 翻数
- `damanguan`: 役満倍率
- `hupai`: 役情報を持つ `Hupai`
- `is_zimohu`: ツモ和了として評価されたか
- `hule_pai`: 和了牌インデックス

`Hule` は手牌の分解候補を評価して、最も良い和了形を選びます。役を人間向けに確認したい場合は `hule.hupai.tolist()` が便利で、`(役名, 翻数)` の一覧を返します。

### `Mianzi`

面子や副露を表現します。

- `Mianzi(MianziType, pai_34)`
- `Mianzi(FuluType, pai_34)`
- `type`: 順子、刻子、対子の種類
- `fulu_type`: チー、ポン、明槓、暗槓、なし
- `pai_34`: 基準牌インデックス。順子なら下の牌、刻子・槓子・対子ならその牌自身

副露を含む手牌を手動で構築するときに使います。

### `Action`

プレイヤーの行動を表します。

コンストラクタ:

- `Action(ActionType, pai_34)`
- `Action(ActionType, pai_34, red)`
- `Action(ActionType, pai_34, red, bias)`

主な `ActionType`:

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

フィールドの意味:

- `pai_34`: その行動が対象にする牌インデックス
- `red`: 対象牌が赤 5 か
- `bias`: `chi` 時の並び位置を表すオフセット。鳴いた牌が順子の真ん中なら通常 `1`

`Shoupai.apply()` や `Hule(...)` と組み合わせて使います。実質的に `bias` が重要なのは `chi` のときです。

### ステートレス補助関数

- `wait_mask(hand, meld_count, three_player=False)`
  指定手牌形の待ち牌マスクを返します。テンパイしていない手牌では 0 になります。
- `has_riichi_discard(hand, meld_count, three_player=False)`
  どれか 1 つ打牌すればテンパイになるかを返します。立直可能判定の前提チェックです。
- `has_hupai(hand, melds, win_tile, is_tsumo, is_menqian, is_riichi, zhuangfeng, lunban, is_haidi, is_lingshang, is_qianggang)`
  指定条件で和了成立するかを返します。
- `evaluate_draw(...)`
  ツモ直後の `(can_tsumo, can_riichi_discard)` を返します。
- `compute_self_option_mask(...)`
  ツモ直後に現在手番プレイヤーが選べる行動のビットマスクを返します。
- `compute_reaction_option_masks(players, discarder, tile_idx, zhuangfeng, dealer_seat, live_draws_left, last_draw_was_gangzimo, three_player=False)`
  打牌に反応できる各プレイヤーの `(seat, mask)` 一覧を返します。
- `compute_rob_kan_option_masks(...)`
  槍槓可能判定の反応マスクを返します。

オプションマスク系で分かりにくい引数:

- `meld_count`: すでにある副露数
- `open_melds`: 開いている副露数。門前判定や立直可否に使います
- `closed_kans`: すでにある暗槓数
- `open_pon_tiles`: すでにポンしていて加槓候補になりうる牌インデックス列
- `is_first_turn`: まだ最初の一巡目か
- `first_turn_open_calls_seen`: その一巡目中に鳴きが入っていないか
- `players`: 各プレイヤーについて、手牌、面子、立直状態、振聴状態、振聴マスク、副露数などをまとめたタプル列
- `dealer_seat`: 親の絶対 seat 番号。これを基準に各 seat の自風 `lunban` が計算されます

### ビットマスク定数

自摸側の行動:

- `SELF_OPT_TSUMO`
- `SELF_OPT_RIICHI`
- `SELF_OPT_ANKAN`
- `SELF_OPT_KAKAN`
- `SELF_OPT_KYUSHUKYUHAI`
- `SELF_OPT_PENUKI`

反応側の行動:

- `REACT_OPT_RON`
- `REACT_OPT_CHI`
- `REACT_OPT_PON`
- `REACT_OPT_MINKAN`

## 開発

テスト実行:

```bash
python3 -m pytest
```

ビルド設定は `CMakeLists.txt` と `setup.py` にあります。パッケージデータとして `data/index_dw_s.bin` と `data/index_dw_h.bin` を含みます。

## リポジトリ内の主要ファイル

- `src/bindings.cpp`: Python バインディング
- `src/calsht_dw.cpp`: シャンテン計算ロジック
- `pymahjong/__init__.py`: Python パッケージのエントリポイント
- `tests/test_regressions.py`: 回帰テスト
