#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "calsht_dw.hpp"
#include "shoupai.hpp"
#include "mianzi.hpp"
#include "hupai.hpp"
#include "hule.hpp"
#include "action.hpp"

namespace py = pybind11;

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
        .def_property_readonly("tingpai", [](const Shoupai &s) {
            std::vector<bool> v(s.tingpai.size());
            for (size_t i = 0; i < s.tingpai.size(); i++)
                v[i] = s.tingpai.test(i);
            return v;
        })
        .def_property_readonly("red", [](const Shoupai &s) {
            std::vector<bool> v(s.red.size());
            for (size_t i = 0; i < s.red.size(); i++)
                v[i] = s.red.test(i);
            return v;
        })
        .def("apply", &Shoupai::apply)
        .def("update", &Shoupai::update);

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
