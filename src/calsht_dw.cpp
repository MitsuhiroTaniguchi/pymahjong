#include "calsht_dw.hpp"
#include "hash.hpp"
#include <array>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <Python.h>
#include <filesystem>
#include <string>
constexpr int NUM_TIDS = 34;
const std::conditional<ENABLE_NYANTEN, NyantenHash<9>, DefaultHash<9>>::type hash1;
const std::conditional<ENABLE_NYANTEN, NyantenHash<7>, DefaultHash<7>>::type hash2;

void shift(uint64_t& lv, const uint64_t rv,
           uint64_t& lx, const uint64_t rx,
           uint64_t& ly, const uint64_t ry)
{
  if (lv == rv) {
    lx |= rx;
    ly |= ry;
  }
  else if (lv > rv) {
    lv = rv;
    lx = rx;
    ly = ry;
  }
}

CalshtDW::RVec CalshtDW::index1(const int n) const
{
  RVec ret(30, 0u);

  std::fill_n(ret.begin(), 10, 14u);

  ret[0] = 0u;
  ret[1] = std::max(3u - n, 0u);
  ret[5] = std::max(2u - n, 0u);

  ret[10] = (n > 0 ? 1u : 0u);
  ret[11] = (n > 3 ? 1u : 0u);
  ret[15] = (n > 2 ? 1u : 0u);

  ret[20] = 0u;
  ret[21] = (n < 3 ? 1u : 0u);
  ret[25] = (n < 2 ? 1u : 0u);

  return ret;
}

void CalshtDW::add1(LVec& lhs, const RVec& rhs, const int m, const int w) const
{
  for (int j = m + 5; j >= 5; --j) {
    uint64_t sht = lhs[j] + rhs[0];
    uint64_t disc = (lhs[j + 10] << w) | rhs[10];
    uint64_t wait = (lhs[j + 20] << w) | rhs[20];

    shift(sht, lhs[0] + rhs[j],
          disc, (lhs[10] << w) | rhs[j + 10],
          wait, (lhs[20] << w) | rhs[j + 20]);

    for (int k = 5; k < j; ++k) {
      shift(sht, lhs[k] + rhs[j - k],
            disc, (lhs[k + 10] << w) | rhs[j - k + 10],
            wait, (lhs[k + 20] << w) | rhs[j - k + 20]);
      shift(sht, lhs[j - k] + rhs[k],
            disc, (lhs[j - k + 10] << w) | rhs[k + 10],
            wait, (lhs[j - k + 20] << w) | rhs[k + 20]);
    }

    lhs[j] = sht;
    lhs[j + 10] = disc;
    lhs[j + 20] = wait;
  }

  for (int j = m; j >= 0; --j) {
    uint64_t sht = lhs[j] + rhs[0];
    uint64_t disc = (lhs[j + 10] << w) | rhs[10];
    uint64_t wait = (lhs[j + 20] << w) | rhs[20];

    for (int k = 0; k < j; ++k) {
      shift(sht, lhs[k] + rhs[j - k],
            disc, (lhs[k + 10] << w) | rhs[j - k + 10],
            wait, (lhs[k + 20] << w) | rhs[j - k + 20]);
    }

    lhs[j] = sht;
    lhs[j + 10] = disc;
    lhs[j + 20] = wait;
  }
}

void CalshtDW::add2(LVec& lhs, const RVec& rhs, const int m, const int w) const
{
  const int j = m + 5;
  uint64_t sht = lhs[j] + rhs[0];
  uint64_t disc = (lhs[j + 10] << w) | rhs[10];
  uint64_t wait = (lhs[j + 20] << w) | rhs[20];

  shift(sht, lhs[0] + rhs[j],
        disc, (lhs[10] << w) | rhs[j + 10],
        wait, (lhs[20] << w) | rhs[j + 20]);

  for (int k = 5; k < j; ++k) {
    shift(sht, lhs[k] + rhs[j - k],
          disc, (lhs[k + 10] << w) | rhs[j - k + 10],
          wait, (lhs[k + 20] << w) | rhs[j - k + 20]);
    shift(sht, lhs[j - k] + rhs[k],
          disc, (lhs[j - k + 10] << w) | rhs[k + 10],
          wait, (lhs[j - k + 20] << w) | rhs[k + 20]);
  }

  lhs[j] = sht;
  lhs[j + 10] = disc;
  lhs[j + 20] = wait;
}

void CalshtDW::read_file(Iter first, Iter last, std::filesystem::path file) const
{
  std::ifstream fin(file, std::ios_base::in | std::ios_base::binary);

  if (!fin) {
    throw std::runtime_error(file);
  }

  for (; first != last; ++first) {
    for (int j = 0; j < 10; ++j) {
      RVec::value_type tmp;

      fin.read(reinterpret_cast<char*>(&tmp), sizeof(RVec::value_type));
      (*first)[j] = tmp & 0xF;
      (*first)[j + 10] = (tmp >> 4) & 0x1FF;
      (*first)[j + 20] = (tmp >> 13) & 0x1FF;
    }
  }
}

std::tuple<int, uint64_t, uint64_t> CalshtDW::calc_lh(const std::array<int, 34>& t,
                                                      const int m,
                                                      const bool three_player) const
{
  LVec ret = [](const RVec& rhs) {
    return LVec(rhs.cbegin(), rhs.cend());
  }(mp2[hash2(t.cbegin() + 27)]);

  add1(ret, mp1[hash1(t.cbegin() + 18)], m, 9);
  add1(ret, mp1[hash1(t.cbegin() + 9)], m, 9);

  if (three_player) {
    add1(ret, index1(t[8]), m, 1);
    add2(ret, index1(t[0]), m, 8);
  }
  else {
    add2(ret, mp1[hash1(t.cbegin())], m, 9);
  }

  return {static_cast<int>(ret[m + 5]), ret[m + 15], ret[m + 25]};
}

std::tuple<int, uint64_t, uint64_t> CalshtDW::calc_sp(const std::array<int, 34>& t,
                                                      const bool three_player) const
{
  int pair = 0;
  int kind = 0;
  uint64_t disc = 0ul;  // 不要牌
  uint64_t wait = 0ul;  // 有効牌
  uint64_t disc_ = 0ul; // 不要牌候補
  uint64_t wait_ = 0ul; // 有効牌候補

  for (int i = 0; i < NUM_TIDS; ++i) {
    if (three_player && i > 0 && i < 8) continue;

    if (t[i] == 0) {
      wait_ |= 1ul << i;
    }
    else if (t[i] == 1) {
      ++kind;
      disc_ |= 1ul << i;
      wait |= 1ul << i;
    }
    else if (t[i] == 2) {
      ++kind;
      ++pair;
    }
    else if (t[i] > 2) {
      ++kind;
      ++pair;
      disc |= 1ul << i;
    }
  }

  if (kind > 7) disc |= disc_;
  else if (kind < 7) wait |= wait_;

  return {7 - pair + (kind < 7 ? 7 - kind : 0), disc, wait};
}

std::tuple<int, uint64_t, uint64_t> CalshtDW::calc_to(const std::array<int, 34>& t) const
{
  int pair = 0;
  int kind = 0;
  uint64_t disc = 0ul;  // 不要牌
  uint64_t wait = 0ul;  // 有効牌
  uint64_t disc_ = 0ul; // 不要牌候補
  uint64_t wait_ = 0ul; // 有効牌候補

  for (const int i : {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33}) {
    if (t[i] == 0) {
      wait |= 1ul << i;
    }
    else if (t[i] == 1) {
      ++kind;
      wait_ |= 1ul << i;
    }
    else if (t[i] == 2) {
      ++kind;
      ++pair;
      disc_ |= 1ul << i;
    }
    else if (t[i] > 2) {
      ++kind;
      ++pair;
      disc |= 1ul << i;
    }
  }

  for (const int i : {1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, 25}) {
    if (t[i] > 0) disc |= 1ul << i;
  }

  if (pair >= 2) disc |= disc_;
  else if (pair == 0) wait |= wait_;

  return {14 - kind - (pair > 0 ? 1 : 0), disc, wait};
}

std::filesystem::path get_module_path() {
    PyObject* module = PyImport_ImportModule("pymahjong");
    if (!module) {
        throw std::runtime_error("Failed to import module 'pymahjong'");
    }
    PyObject* file_attr = PyObject_GetAttrString(module, "__file__");
    Py_DECREF(module);
    if (!file_attr) {
        throw std::runtime_error("Module 'pymahjong' has no __file__ attribute");
    }
    const char* path_c = PyUnicode_AsUTF8(file_attr);
    Py_DECREF(file_attr);
    if (!path_c) {
        throw std::runtime_error("Failed to get module file path");
    }
    return std::filesystem::path(path_c);
}

void CalshtDW::initialize() {
    std::filesystem::path module_path = get_module_path();
    std::filesystem::path data_dir = module_path.parent_path() / "data";

    read_file(mp1.begin(), mp1.end(), data_dir / "index_dw_s.bin");
    read_file(mp2.begin(), mp2.end(), data_dir / "index_dw_h.bin");
}

std::tuple<int, int, uint64_t, uint64_t> CalshtDW::operator()(const std::array<int, 34>& t,
                                                              const int m,
                                                              const int mode,
                                                              const bool check_hand,
                                                              const bool three_player) const
{
  if (check_hand) {
    int n = 0;

    for (int i = 0; i < NUM_TIDS; ++i) {
      if (t[i] < 0 || t[i] > 4) {
        throw std::invalid_argument("Invalid number of hand's tiles at {}: {}");
      }

      ++n;
    }

    if (ENABLE_NYANTEN && n > 14) {
      throw std::invalid_argument("Invalid sum of hand's tiles");
    }

    if (m < 0 || m > 4) {
      throw std::invalid_argument("Invalid sum of hands's melds");
    }

    if (mode < 0 || mode > 7) {
      throw std::invalid_argument("Invalid caluculation mode");
    }
  }

  std::tuple<int, int, uint64_t, uint64_t> ret{1024, 0, 0, 0};

  if (mode & 1) {
    if (auto [sht, disc, wait] = calc_lh(t, m, three_player); sht < std::get<0>(ret)) {
      ret = {sht, 1, disc, wait};
    }
    else if (sht == std::get<0>(ret)) {
      std::get<1>(ret) |= 1;
      std::get<2>(ret) |= disc;
      std::get<3>(ret) |= wait;
    }
  }

  if ((mode & 2) && m == 4) {
    if (auto [sht, disc, wait] = calc_sp(t, three_player); sht < std::get<0>(ret)) {
      ret = {sht, 2, disc, wait};
    }
    else if (sht == std::get<0>(ret)) {
      std::get<1>(ret) |= 2;
      std::get<2>(ret) |= disc;
      std::get<3>(ret) |= wait;
    }
  }

  if ((mode & 4) && m == 4) {
    if (auto [sht, disc, wait] = calc_to(t); sht < std::get<0>(ret)) {
      ret = {sht, 4, disc, wait};
    }
    else if (sht == std::get<0>(ret)) {
      std::get<1>(ret) |= 4;
      std::get<2>(ret) |= disc;
      std::get<3>(ret) |= wait;
    }
  }

  std::get<0>(ret) -= 1;

  return ret;
}