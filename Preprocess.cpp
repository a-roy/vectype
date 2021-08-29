#include "Preprocess.hpp"

#include FT_GLYPH_H

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>

#pragma warning( disable : 26812 )

namespace vt
{
	void ComputeGridForGlyph(const FT_Outline& outline, GlyphGrid& grid)
	{
		// tag that says whether the point is on or off the curve
		constexpr char Outline_OnBit = 1 << 0;

		auto& bounds = grid.Bounds;
		auto& gridWidth = grid.GridWidth;
		auto& gridHeight = grid.GridHeight;
		auto& beziers = grid.Beziers;
		auto& allCellIndices = grid.IntersectingIndices;

		float xMin = std::numeric_limits<float>::max();
		float xMax = std::numeric_limits<float>::min();
		float yMin = std::numeric_limits<float>::max();
		float yMax = std::numeric_limits<float>::min();

		for (short i = 0; i < outline.n_points; i++)
		{
			float x = static_cast<float>(outline.points[i].x);
			float y = static_cast<float>(outline.points[i].y);

			if (i == 0)
			{
				beziers.push_back({ x, y });
			}
			else if (outline.tags[i] & Outline_OnBit)
			{
				// for now we don't care about cubic beziers
				// any collinear point can be used to express a line as a bezier
				beziers.push_back(
					{
						static_cast<float>(outline.points[i - 1].x),
						static_cast<float>(outline.points[i - 1].y),
					});

				beziers.push_back({ x, y });
			}

			if (i > 0 && !(outline.tags[i] & Outline_OnBit) &&
				!(outline.tags[i - 1] & Outline_OnBit))
			{
				x = (x + outline.points[i - 1].x) * 0.5f;
				y = (y + outline.points[i - 1].y) * 0.5f;
			}

			if (x < xMin)
			{
				xMin = x;
			}
			if (x > xMax)
			{
				xMax = x;
			}
			if (y < yMin)
			{
				yMin = y;
			}
			if (y > yMax)
			{
				yMax = y;
			}
		}

		bounds = { xMin, yMin, xMax - xMin, yMax - yMin };

		gridWidth = 1;
		gridHeight = 1;

		while (true)
		{
			std::size_t maxCellCurves = 0;
			std::size_t maxRow = 0;
			std::size_t maxCol = 0;
			auto allCellCurves = std::vector<std::size_t>(gridHeight * gridWidth, 0);
			allCellIndices = std::vector<IndexArray>(gridHeight * gridWidth);

			float width = (xMax - xMin) / gridWidth;
			float height = (yMax - yMin) / gridHeight;
			for (std::size_t row = 0; row < gridHeight; row++)
			{
				float yCell = (yMin + (yMax - yMin) * row) / gridHeight;

				for (std::size_t col = 0; col < gridWidth; col++)
				{
					float xCell = (xMin + (xMax - xMin) * col) / gridWidth;

					Region cellRegion{ xCell, yCell, width, height };

					auto& cellCurves = allCellCurves[row * gridWidth + col];
					auto& cellIndices = allCellIndices[row * gridWidth + col];
					std::size_t curveIndex = 0;
					for (short i = 1; i < outline.n_points; i++)
					{
						if (!(outline.tags[i] & Outline_OnBit) &&
							(outline.tags[i - 1] & Outline_OnBit))
						{
							continue;
						}
						if (outline.tags[i] & Outline_OnBit)
						{
							curveIndex++;

							auto p1 = MakePoint(outline.points[i]);

							if (outline.tags[i - 1] & Outline_OnBit)
							{
								auto p0 = MakePoint(outline.points[i - 1]);

								Line line{ p0, p1 };

								if (LineIntersectsRegion(cellRegion, { p0, p1 }))
								{
									if (cellCurves < CurvesPerCell)
									{
										cellIndices[cellCurves] = curveIndex;
									}

									cellCurves++;
								}
							}
							else if (i > 1)
							{
								auto cp = MakePoint(outline.points[i - 1]);
								auto p0 = MakePoint(outline.points[i - 2]);

								BezierCurve curve{ p0, cp, p1 };

								if (i > 2 && !(outline.tags[i - 2] & Outline_OnBit))
								{
									p0.x = (p0.x + outline.points[i - 3].x) * 0.5f;
									p0.y = (p0.y + outline.points[i - 3].y) * 0.5f;
								}

								if (CurveIntersectsRegion(cellRegion, curve))
								{
									if (cellCurves < CurvesPerCell)
									{
										cellIndices[cellCurves] = curveIndex;
									}

									cellCurves++;
								}
							}
						}
					}

					if (cellCurves > maxCellCurves)
					{
						maxCellCurves = cellCurves;
						maxRow = row;
						maxCol = col;
					}
				}
			}

#ifdef DEBUG_GRIDS
			for (int i = 0; i < gridHeight; i++)
			{
				if (i == 0)
					std::cout << "[[";
				else
					std::cout << " [";

				for (int j = 0; j < gridWidth; j++)
				{
					std::cout << allCellCurves[i * gridWidth + j];
					if (j < gridWidth - 1)
						std::cout << ", ";
					else
						std::cout << "]";
				}

				if (i == gridHeight - 1)
					std::cout << "]";

				std::cout << std::endl;
			}
#endif

			if (maxCellCurves <= CurvesPerCell)
			{
				break;
			}

			auto rowCurves = std::vector<std::size_t>(gridHeight, 0);
			auto colCurves = std::vector<std::size_t>(gridWidth, 0);
			for (std::size_t row = 0; row < gridHeight; row++)
			{
				for (std::size_t col = 0; col < gridWidth; col++)
				{
					rowCurves[row] += allCellCurves[row * gridWidth + col];
					colCurves[col] += allCellCurves[row * gridWidth + col];
				}
			}

			std::size_t maxRowCurves = *std::max_element(rowCurves.begin(), rowCurves.end());
			std::size_t maxColCurves = *std::max_element(colCurves.begin(), colCurves.end());

			if (maxColCurves > maxRowCurves ||
				(maxColCurves == maxRowCurves && width >= height))
			{
				gridWidth++;
			}

			if (maxRowCurves > maxColCurves ||
				(maxRowCurves == maxColCurves && height >= width))
			{
				gridHeight++;
			}
		}
	}

	void ProcessGlyphs(FT_Face face, std::vector<GlyphGrid>& grids, std::vector<CharRange> ranges)
	{
		FT_Error error;

		grids.reserve(face->num_glyphs);

		FT_UInt gindex;
		FT_ULong charcode;

		charcode = FT_Get_First_Char(face, &gindex);

		while (gindex != 0)
		{
			charcode = FT_Get_Next_Char(face, charcode, &gindex);

			bool charcode_in_range = false;
			for (auto& range : ranges)
			{
				if (charcode >= range.first && charcode <= range.second)
				{
					charcode_in_range = true;
					break;
				}
			}

			if (!charcode_in_range)
			{
				continue;
			}

			FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);

			error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);

			FT_Glyph glyph;
			error = FT_Get_Glyph(face->glyph, &glyph);

			if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
			{
				throw std::runtime_error{ "Unexpected glyph format." };
			}

			FT_OutlineGlyph outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(glyph);
			FT_Outline& outline = outlineGlyph->outline;

			grids.push_back(GlyphGrid{});
			GlyphGrid& grid = grids.back();

			ComputeGridForGlyph(outline, grid);

			FT_Done_Glyph(glyph);
		}
	}

	void PackGlyphGrids(
		const std::vector<GlyphGrid>& grids,
		std::size_t atlasWidth,
		std::vector<float>& beziers,
		std::vector<std::uint16_t>& gridIndices)
	{
		std::vector<GlyphGrid> sorted = grids;
		auto comp = [](const GlyphGrid& a, const GlyphGrid& b)
		{
			return a.GridHeight < b.GridHeight;
		};

		std::sort(sorted.begin(), sorted.end(), comp);

		std::size_t gridsPos = 0;
		std::size_t shelfSpace = 0;
		while (!sorted.empty())
		{
			auto& nextGrid = sorted.back();
			std::size_t height = nextGrid.GridHeight;
			std::size_t width = nextGrid.GridWidth * (CurvesPerCell / 4);
			if (shelfSpace < width)
			{
				gridsPos = gridIndices.size();
				gridIndices.resize(gridsPos + 4 * (height * atlasWidth));
				shelfSpace = atlasWidth;
			}

			assert(shelfSpace >= width);

			beziers.push_back(static_cast<float>((gridsPos / 4) % atlasWidth));
			beziers.push_back(static_cast<float>((gridsPos / 4) / atlasWidth));
			beziers.push_back(static_cast<float>(nextGrid.GridWidth));
			beziers.push_back(static_cast<float>(nextGrid.GridHeight));
			beziers.push_back(0.0f);
			beziers.push_back(0.0f);
			for (auto& p : nextGrid.Beziers)
			{
				auto n = NormalizeToRegion(nextGrid.Bounds, p);
				beziers.push_back(n.x);
				beziers.push_back(n.y);
			}

			assert(nextGrid.IntersectingIndices.size() == nextGrid.GridHeight * nextGrid.GridWidth);

			for (std::size_t col = 0; col < nextGrid.GridWidth; col++)
			{
				for (std::size_t row = 0; row < nextGrid.GridHeight; row++)
				{
					auto& indexArray = nextGrid.IntersectingIndices[row * nextGrid.GridWidth + col];
					if (indexArray[0] == 0)
					{
						continue;
					}

					if (indexArray.size() > 4 && indexArray[4] > 0)
					{
						auto offset1 = 4 * (row * atlasWidth + col);
						auto offset2 = 4 * (row * atlasWidth + col + nextGrid.GridWidth);

						for (std::size_t i = 0; i < 4; i++)
						{
							assert(indexArray[i] < nextGrid.Beziers.size());

							gridIndices[gridsPos + offset1 + i] =
								static_cast<std::uint16_t>(indexArray[i]);
						}

						for (std::size_t i = 0; i < 4; i++)
						{
							assert(indexArray[4 + i] < nextGrid.Beziers.size());

							gridIndices[gridsPos + offset2 + i] =
								static_cast<std::uint16_t>(indexArray[4 + i]);
						}
					}
					else
					{
						auto offset = 4 * (row * atlasWidth + col);

						for (std::size_t i = 0; i < 4; i++)
						{
							assert(indexArray[3 - i] < nextGrid.Beziers.size());

							gridIndices[gridsPos + offset + i] =
								static_cast<std::uint16_t>(indexArray[3 - i]);
						}
					}
				}
			}

			gridsPos += width;
			shelfSpace -= width;

			sorted.pop_back();
		}
	}
}
