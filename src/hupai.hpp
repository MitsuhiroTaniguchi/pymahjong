#ifndef HUPAI_HPP
#define HUPAI_HPP

struct Hupai {
	bool is_menqian;
	bool 立直,
		 一発,
		 門前清自摸和,
		 平和,
		 一盃口,
		 断么九,
		 場風_東,
		 場風_南,
		 場風_西,
		 場風_北,
		 自風_東,
		 自風_南,
		 自風_西,
		 自風_北,
		 役牌_白,
		 役牌_發,
		 役牌_中,
		 槍槓,
		 嶺上開花,
		 海底摸月,
		 河底撈魚,
		 両立直,
		 七対子,
		 混全帯么九,
		 一気通貫,
		 三色同順,
		 三色同刻,
		 三槓子,
		 対々和,
		 三暗刻,
		 小三元,
		 混老頭,
		 純全帯么九,
		 二盃口,
		 混一色,
		 清一色,
		 天和,
		 地和,
		 大三元,
		 四暗刻,
		 四暗刻単騎,
		 字一色,
		 緑一色,
		 清老頭,
		 国士無双,
		 国士無双１３面,
		 小四喜,
		 大四喜,
		 四槓子,
		 九蓮宝燈,
		 純正九蓮宝燈;

	void _validate() {
		if (立直 && 両立直) 立直 = false;
		if (一盃口 && 二盃口) 一盃口 = false;
		if (混全帯么九 && 純全帯么九) 混全帯么九 = false;
		if (混全帯么九 && 混老頭) 混全帯么九 = false;
		if (混一色 && 清一色) 混一色 = false;
		if (四暗刻 && 四暗刻単騎) 四暗刻 = false;
		if (国士無双 && 国士無双１３面) 国士無双 = false;
		if (小四喜 && 大四喜) 小四喜 = false;
		if (九蓮宝燈 && 純正九蓮宝燈) 九蓮宝燈 = false;

		if (天和 ||
		    地和 ||
		    大三元 ||
		    四暗刻 ||
		    四暗刻単騎 ||
		    字一色 ||
		    緑一色 ||
		    清老頭 ||
		    国士無双 ||
		    国士無双１３面 ||
		    小四喜 ||
		    大四喜 ||
		    四槓子 ||
		    九蓮宝燈 ||
		    純正九蓮宝燈) {
			立直 = false,
			一発 = false;
			門前清自摸和 = false;
			平和 = false;
			一盃口 = false;
			断么九 = false;
			場風_東 = false;
			場風_南 = false;
			場風_西 = false;
			場風_北 = false;
			自風_東 = false;
			自風_南 = false;
			自風_西 = false;
			自風_北 = false;
			役牌_白 = false;
			役牌_發 = false;
			役牌_中 = false;
			槍槓 = false;
			嶺上開花 = false;
			海底摸月 = false;
			河底撈魚 = false;
			両立直 = false;
			七対子 = false;
			混全帯么九 = false;
			一気通貫 = false;
			三色同順 = false;
			三色同刻 = false;
			三槓子 = false;
			対々和 = false;
			三暗刻 = false;
			小三元 = false;
			混老頭 = false;
			純全帯么九 = false;
			二盃口 = false;
			混一色 = false;
			清一色 = false;
		}
	}

	std::pair<uint64_t, uint64_t> sum() {
		_validate();
		uint64_t fanshu =
				立直 +
				一発 +
				門前清自摸和 +
				平和 +
				一盃口 +
				断么九 +
				場風_東 +
				場風_南 +
				場風_西 +
				場風_北 +
				自風_東 +
				自風_南 +
				自風_西 +
				自風_北 +
				役牌_白 +
				役牌_發 +
				役牌_中 +
				槍槓 +
				嶺上開花 +
				海底摸月 +
				河底撈魚 +
			2 * 両立直 +
			2 * 七対子 +
			2 * 三色同刻 +
			2 * 三槓子 +
			2 * 対々和 +
			2 * 三暗刻 +
			2 * 小三元 +
			2 * 混老頭 +
			(1 + is_menqian) * 混全帯么九 +
			(1 + is_menqian) * 一気通貫 +
			(1 + is_menqian) * 三色同順 +
			(2 + is_menqian) * 純全帯么九 +
			(2 + is_menqian) * 二盃口 +
			(2 + is_menqian) * 混一色 +
			(5 + is_menqian) * 清一色;
		uint64_t damanguan =
				天和 +
				地和 +
				大三元 +
				四暗刻 +
			2 * 四暗刻単騎 +
				字一色 +
				緑一色 +
				清老頭 +
				国士無双 +
			2 * 国士無双１３面 +
				小四喜 +
			2 * 大四喜 +
				四槓子 +
				九蓮宝燈 +
			2 * 純正九蓮宝燈;
		return {fanshu, damanguan};
	}

	std::vector<std::pair<std::string, std::uint8_t>> to_list() {
        _validate();
        std::vector<std::pair<std::string, uint8_t>> ret;

        if (立直) ret.emplace_back("立直", 立直);
        if (一発) ret.emplace_back("一発", 一発);
        if (門前清自摸和) ret.emplace_back("門前清自摸和", 門前清自摸和);
        if (平和) ret.emplace_back("平和", 平和);
        if (一盃口) ret.emplace_back("一盃口", 一盃口);
        if (断么九) ret.emplace_back("断么九", 断么九);
        if (場風_東) ret.emplace_back("場風_東", 場風_東);
        if (場風_南) ret.emplace_back("場風_南", 場風_南);
        if (場風_西) ret.emplace_back("場風_西", 場風_西);
        if (場風_北) ret.emplace_back("場風_北", 場風_北);
        if (自風_東) ret.emplace_back("自風_東", 自風_東);
        if (自風_南) ret.emplace_back("自風_南", 自風_南);
        if (自風_西) ret.emplace_back("自風_西", 自風_西);
        if (自風_北) ret.emplace_back("自風_北", 自風_北);
        if (役牌_白) ret.emplace_back("役牌_白", 役牌_白);
        if (役牌_發) ret.emplace_back("役牌_發", 役牌_發);
        if (役牌_中) ret.emplace_back("役牌_中", 役牌_中);
        if (槍槓) ret.emplace_back("槍槓", 槍槓);
        if (嶺上開花) ret.emplace_back("嶺上開花", 嶺上開花);
		if (海底摸月) ret.emplace_back("海底摸月", 海底摸月);
        if (河底撈魚) ret.emplace_back("河底撈魚", 河底撈魚);
		if (両立直) ret.emplace_back("両立直", 2 * 両立直);
		if (七対子) ret.emplace_back("七対子", 2 * 七対子);
        if (三色同刻) ret.emplace_back("三色同刻", 2 * 三色同刻);
		if (三槓子) ret.emplace_back("三槓子", 2 * 三槓子);
		if (対々和) ret.emplace_back("対々和", 2 * 対々和);
		if (三暗刻) ret.emplace_back("三暗刻", 2 * 三暗刻);
        if (小三元) ret.emplace_back("小三元", 2 * 小三元);
		if (混老頭) ret.emplace_back("混老頭", 2 * 混老頭);
		if (混全帯么九) ret.emplace_back("混全帯么九", (1 + is_menqian) * 混全帯么九);
		if (一気通貫) ret.emplace_back("一気通貫", (1 + is_menqian) * 一気通貫);
		if (三色同順) ret.emplace_back("三色同順", (1 + is_menqian) * 三色同順);
		if (純全帯么九) ret.emplace_back("純全帯么九", (2 + is_menqian) * 純全帯么九);
		if (二盃口) ret.emplace_back("二盃口", (2 + is_menqian) * 二盃口);
        if (混一色) ret.emplace_back("混一色", (2 + is_menqian) * 混一色);
		if (清一色) ret.emplace_back("清一色", (5 + is_menqian) * 清一色);

		if (天和) ret.emplace_back("天和",  天和);
        if (地和) ret.emplace_back("地和",  地和);
        if (大三元) ret.emplace_back("大三元",  大三元);
		if (四暗刻) ret.emplace_back("四暗刻", 四暗刻);
		if (四暗刻単騎) ret.emplace_back("四暗刻単騎", 2 * 四暗刻単騎);
		if (字一色) ret.emplace_back("字一色", 字一色);
        if (緑一色) ret.emplace_back("緑一色", 緑一色);
		if (清老頭) ret.emplace_back("清老頭", 清老頭);
		if (国士無双) ret.emplace_back("国士無双", 国士無双);
		if (国士無双１３面) ret.emplace_back("国士無双１３面", 2 * 国士無双１３面);
		if (小四喜) ret.emplace_back("小四喜", 小四喜);
        if (大四喜) ret.emplace_back("大四喜", 2 * 大四喜);
        if (四槓子) ret.emplace_back("四槓子", 四槓子);
		if (九蓮宝燈) ret.emplace_back("九蓮宝燈", 九蓮宝燈);
		if (純正九蓮宝燈) ret.emplace_back("純正九蓮宝燈", 2 * 純正九蓮宝燈);

        return ret;
	}
};

#endif //HUPAI_HPP
