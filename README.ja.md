# pymahjong

[English](README.md) | [日本語](README.ja.md)

`pymahjong` は、麻雀の手牌ロジックを C++ で実装し、Python から使えるようにした低レイヤのバインディングです。主な対象は、手牌状態、シャンテン計算、和了判定、行動可能判定です。

向いている用途:

- 麻雀 AI やシミュレータ
- ルールエンジン
- 牌効率や手牌解析ツール

## 何ができるか

- 閉じた手牌、副露、赤牌状態をまとめて表現する
- シャンテン数と待ち牌マスクを計算する
- 和了の成立、符、翻、役満、役一覧を評価する
- 今の手番プレイヤーが何を選べるかを判定する
- 打牌や槓に対して誰がどう反応できるかを判定する
- 四麻と三麻の両方に対応する

## API を読む前の前提

ほとんどの API は同じ前提で動きます。

### 牌インデックス

牌は 34 種インデックスで表現します。

- `0-8`: `m1-m9`
- `9-17`: `p1-p9`
- `18-26`: `s1-s9`
- `27-33`: `東, 南, 西, 北, 白, 發, 中`

### 風の表現

- `zhuangfeng`: 場風
- `lunban`: 行動者または和了者の自風

エンコードは次の通りです。

- `0 = 東`
- `1 = 南`
- `2 = 西`
- `3 = 北`

### ビットマスク

ビットマスクでは bit `n` が牌インデックス `n` を表します。

例:

- 待ち牌マスクは、どの牌を引けばテンパイ手が和了するかを示します
- `SELF_OPT_*` は手番プレイヤーの選択肢を示します
- `REACT_OPT_*` は他家の反応可能行動を示します

## インストール

ビルドに必要なもの:

- C++17 対応コンパイラ
- CMake
- `pybind11`

インストール:

```bash
python3 -m pip install .
```

テスト依存込みの editable install:

```bash
python3 -m pip install -e .[test]
```

## 典型的な使い方

### 1. ミュータブルな手牌を持つ: `Shoupai`

ゲーム進行に合わせて手牌オブジェクトを更新したいなら `Shoupai` を使います。

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

### 2. シャンテン数だけ素早く引く: `Xiangting`

生の牌カウントから高速にシャンテン計算したいなら `Xiangting` を使います。

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

返り値:

- `shanten`: 現在のシャンテン数
- `mode`: ソルバ内部モード
- `discard_mask`: 打牌候補のマスク
- `wait_mask`: 最善打牌考慮後の有効牌マスク

### 3. 和了を評価する: `Hule`

和了牌が分かっていて、符・翻・役まで見たいなら `Hule` を使います。

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

### 4. 今の選択肢を調べる

ツモ直後に何ができるかを知りたいなら `compute_self_option_mask()` を使います。

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

## どの API を使うべきか

- 手番進行に合わせて手牌を更新したいなら `Shoupai`
- 生の牌カウントからシャンテン数だけ欲しいなら `Xiangting.calculate()`
- 符・翻・役まで欲しいなら `Hule`
- テンパイ手の待ちだけ欲しいなら `wait_mask()`
- 和了成立の真偽だけ欲しいなら `has_hupai()`
- 対局進行の選択肢判定をしたいなら `compute_self_option_mask()` と `compute_reaction_option_masks()`

## 主な型

### `Shoupai`

ミュータブルな手牌状態です。

コンストラクタ:

- `Shoupai(bing)`
- `Shoupai(bing, fulu)`

主なフィールドとメソッド:

- `bing`: 閉じた手牌の 34 牌カウント
- `fulu`: `Mianzi` の副露リスト
- `xiangting`: `update()` 後のシャンテン数
- `mode`: `update()` 後のソルバモード
- `tingpai`: 待ち牌マスク
- `red`: 赤 5m, 赤 5p, 赤 5s を表す 3 ビット整数
- `apply(action)`: `Action` を適用して手牌を更新
- `update()`: `xiangting`, `mode`, `tingpai` を再計算
- `tingpai_mask()`: 待ち牌マスクを返す
- `tingpai_list()`: 待ち牌インデックスのリストを返す

局進行の中でツモ、打牌、鳴き、加槓などを反映しながら手牌を持ち回したいときの中心になります。

### `Xiangting`

ステートレスなシャンテンソルバの入口です。

主なメソッド:

- `calculate(hand, size, mode, check_hand=False, three_player=False)`

引数の意味:

- `hand`: 長さ 34 の牌カウント
- `size`: これから完成させる面子数
- `mode`: ネイティブ実装側のソルバモード
- `check_hand`: ありえない牌枚数を事前検証するか
- `three_player`: 三麻ロジックに切り替えるか

通常手なら `size` はだいたい `4` です。副露済みの面子があるなら、その分だけ残り面子数は小さくなります。

### `HuleOption`

和了判定に必要な局面コンテキストです。

主なフィールド:

- `is_menqian`: 門前か
- `is_lizhi`: 立直しているか
- `is_shuanglizhi`: ダブル立直か
- `is_yifa`: 一発が有効か
- `is_haidi`: 海底または河底の局面か
- `is_lingshang`: 嶺上開花か
- `is_qianggang`: 槍槓か
- `is_init_turn_and_no_call`: 鳴きの入っていない最初の一巡目か。天和・地和系判定に使う
- `zhuangfeng`: 場風
- `lunban`: 和了者の自風

これは手牌形ではなく、卓上状況を表すオブジェクトです。

### `Hule`

和了評価結果です。

コンストラクタ:

- `Hule(shoupai, action, option)`

主な出力:

- `has_hupai`: 和了として成立しているか
- `fu`: 符数
- `fanshu`: 翻数
- `damanguan`: 役満倍率
- `hupai`: 役フラグを持つ `Hupai`
- `hule_pai`: 和了牌インデックス
- `is_zimohu`: ツモとして評価されたか

役を読みやすく出したいなら `hupai.tolist()` を使います。

### `Mianzi`

1 つの面子またはブロックを表します。

コンストラクタ:

- `Mianzi(MianziType, pai_34)`
- `Mianzi(FuluType, pai_34)`

主なフィールド:

- `type`: 順子、刻子、対子
- `fulu_type`: チー、ポン、明槓、暗槓、なし
- `pai_34`: 基準牌インデックス

`pai_34` の意味:

- 順子なら一番下の牌
- 対子、ポン、槓なら重なっている牌そのもの

### `Action`

手牌に適用する行動、または和了イベントを表します。

コンストラクタ:

- `Action(ActionType, pai_34)`
- `Action(ActionType, pai_34, red)`
- `Action(ActionType, pai_34, red, bias)`

よく使う `ActionType`:

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

主なフィールド:

- `pai_34`: 対象牌インデックス
- `red`: その牌が赤 5 か
- `bias`: `chi` のときに鳴いた牌が順子のどこに入るかを示すオフセット

`bias` が実質的に重要なのは `chi` のときです。

## 主な補助関数

### 手牌状態向け

- `wait_mask(hand, meld_count, three_player=False)`
  テンパイ手の待ち牌マスクを返します。テンパイしていなければ `0` です。
- `has_riichi_discard(hand, meld_count, three_player=False)`
  どれか 1 牌切ればテンパイになるかを返します。
- `has_hupai(...)`
  指定条件で和了成立するかを返します。
- `evaluate_draw(...)`
  ツモ直後の `(can_tsumo, can_riichi_discard)` を返します。

### 対局進行向け

- `compute_self_option_mask(...)`
  ツモ直後に手番プレイヤーが取れる行動を返します。
- `compute_reaction_option_masks(...)`
  打牌に対して各プレイヤーが取れる反応を `(seat, mask)` で返します。
- `compute_rob_kan_option_masks(...)`
  槍槓に対する反応を `(seat, mask)` で返します。

分かりにくい引数:

- `meld_count`: 現在できている面子数
- `open_melds`: 開いている副露数。門前判定や立直可否に使う
- `closed_kans`: すでにある暗槓数
- `open_pon_tiles`: 加槓候補になりうるポン牌のインデックス列
- `is_first_turn`: まだ最初の一巡目か
- `first_turn_open_calls_seen`: その一巡目が鳴きで崩れていないか
- `dealer_seat`: 親の絶対 seat 番号。これを基準に各 seat の自風が計算される
- `players`: 手牌、立直状態、振聴状態などをまとめたプレイヤータプル列

## 定数

自摸側の行動ビット:

- `SELF_OPT_TSUMO`
- `SELF_OPT_RIICHI`
- `SELF_OPT_ANKAN`
- `SELF_OPT_KAKAN`
- `SELF_OPT_KYUSHUKYUHAI`
- `SELF_OPT_PENUKI`

反応側の行動ビット:

- `REACT_OPT_RON`
- `REACT_OPT_CHI`
- `REACT_OPT_PON`
- `REACT_OPT_MINKAN`

## リポジトリ構成

- `src/bindings.cpp`: Python バインディングと補助関数
- `src/calsht_dw.cpp`: シャンテン計算
- `src/hule.hpp`: 和了評価と点数計算
- `src/shoupai.hpp`: ミュータブルな手牌表現
- `tests/test_regressions.py`: 回帰テスト

## 開発

テスト実行:

```bash
python3 -m pytest
```

## 補足

- API は意図的に低レイヤです。実アプリでは薄いラッパを 1 層載せるのが自然です。
- README では使い方と前提を優先しており、内部ソルバモードの詳細までは説明していません。

## ライセンス

`pymahjong` は `LGPL-3.0-only` です。

この repository には
[`tomohxx/shanten-number`](https://github.com/tomohxx/shanten-number)
由来のシャンテン数計算コードが含まれており、こちらも LGPL v3 系です。
詳細は [THIRD_PARTY.md](THIRD_PARTY.md) を参照してください。
