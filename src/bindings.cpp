#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <tuple>
#include <cstdint>

#include "calsht_dw.hpp"
#include "shoupai.hpp"
#include "mianzi.hpp"
#include "hupai.hpp"
#include "hule.hpp"
#include "action.hpp"

namespace py = pybind11;

namespace {
uint64_t bitset34_to_mask(const std::bitset<34>& bits) {
    uint64_t mask = 0;
    for (int i = 0; i < 34; ++i) {
        if (bits.test(i)) {
            mask |= (uint64_t(1) << i);
        }
    }
    return mask;
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
        .def_readwrite("tingpai", &Shoupai::tingpai)
        .def_readwrite("red", &Shoupai::red)
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

    m.def("wait_mask", [](const std::array<int, 34>& hand, int meld_count) {
        static CalshtDW xiangting_calculator;
        auto [x, _mode, _disc, wait] = xiangting_calculator(hand, 4 - meld_count, 7, false, false);
        if (x != 0) return uint64_t(0);
        return static_cast<uint64_t>(wait);
    }, py::arg("hand"), py::arg("meld_count"));

    m.def("has_riichi_discard", [](const std::array<int, 34>& hand, int meld_count) {
        static CalshtDW xiangting_calculator;
        auto base = hand;
        for (int i = 0; i < 34; ++i) {
            if (base[i] == 0) continue;
            --base[i];
            auto [x, _mode, _disc, _wait] = xiangting_calculator(base, 4 - meld_count, 7, false, false);
            ++base[i];
            if (x == 0) return true;
        }
        return false;
    }, py::arg("hand"), py::arg("meld_count"));

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
             bool is_lingshang) {
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
                  static CalshtDW xiangting_calculator;
                  auto base = hand;
                  for (int i = 0; i < 34; ++i) {
                      if (base[i] == 0) continue;
                      --base[i];
                      auto [x, _mode, _disc, _wait] = xiangting_calculator(
                          base, 4 - closed_kans, 7, false, false);
                      ++base[i];
                      if (x == 0) {
                          can_riichi_discard = true;
                          break;
                      }
                  }
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
          py::arg("is_lingshang"));

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
