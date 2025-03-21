#ifndef HUPAI_HPP
#define HUPAI_HPP

struct Hupai {
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

	std::pair<uint64_t, uint64_t> sum(bool is_menqian) const {
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
};

#endif //HUPAI_HPP
