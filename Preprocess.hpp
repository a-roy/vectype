#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "Geometry.hpp"

namespace vt
{
	inline constexpr std::size_t CurvesPerCell = 8;

	using IndexArray = std::array<std::size_t, CurvesPerCell>;

	using CharRange = std::pair<char32_t, char32_t>;
	constexpr CharRange CharRangeDefault = { 0x0020, 0x00FF };

	constexpr auto CharRangesDefault = { CharRangeDefault };

	constexpr CharRange CharRangeChinese1 = { 0x3000, 0x30FF };
	constexpr CharRange CharRangeChinese2 = { 0x31F0, 0x31FF };
	constexpr CharRange CharRangeChinese3 = { 0xFF00, 0xFFEF };
	constexpr CharRange CharRangeChinese4 = { 0x4E00, 0x9FAF };

	constexpr auto CharRangesChinese =
	{
		CharRangeDefault,
		CharRangeChinese1,
		CharRangeChinese2,
		CharRangeChinese3,
		CharRangeChinese4,
	};

	constexpr CharRange CharRangeCyrillic1 = { 0x0400, 0x052F };
	constexpr CharRange CharRangeCyrillic2 = { 0x2DE0, 0x2DFF };
	constexpr CharRange CharRangeCyrillic3 = { 0xA640, 0xA69F };

	constexpr auto CharRangesCyrillic =
	{
		CharRangeDefault,
		CharRangeCyrillic1,
		CharRangeCyrillic2,
		CharRangeCyrillic3,
	};

	constexpr CharRange CharRangeKorean1 = { 0x3131, 0x3163 };
	constexpr CharRange CharRangeKorean2 = { 0xAC00, 0xD79D };

	constexpr auto CharRangesKorean =
	{
		CharRangeDefault,
		CharRangeKorean1,
		CharRangeKorean2,
	};

	struct GlyphGrid
	{
		Region Bounds;
		std::size_t GridWidth;
		std::size_t GridHeight;
		std::vector<Point> Beziers;
		std::vector<IndexArray> IntersectingIndices;
	};

	inline Point MakePoint(FT_Vector_& vector)
	{
		return { static_cast<float>(vector.x), static_cast<float>(vector.y) };
	}

	void ComputeGridForGlyph(const FT_Outline& outline, GlyphGrid& grid);

	void ProcessGlyphs(
		FT_Face face,
		std::vector<GlyphGrid>& grids,
		std::vector<CharRange> ranges = CharRangesDefault);

	void PackGlyphGrids(
		const std::vector<GlyphGrid>& grids,
		std::size_t atlasWidth,
		std::vector<float>& beziers,
		std::vector<std::uint16_t>& gridIndices);
}
