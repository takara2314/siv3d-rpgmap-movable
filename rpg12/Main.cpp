/*
	This work features an adaptation of Siv3D-kun, ©2015 Siv3D, licensed under a CC BY-NC: http://creativecommons.org/licenses/by-nc/4.0/
*/

#include <Siv3D.hpp>

// ゲームに表示するマップチップの量
const int DrawChipNumX = 16;
const int DrawChipNumY = 16;
// 扱うマップデータの1タイルの大きさ (px)
const int ChipSize = 32;
// 扱うマップデータの一列のタイルの数
const int ChipNumX = 8;
const int ChipNumY = 8;
// 扱うキャラデータの1タイルの大きさ (px)
const int CharaSizeX = 20;
const int CharaSizeY = 28;

// CSVファイルを読み込み、マップデータを返す
Grid<int> LoadCSV(const FilePath& path)
{
	const CSVData csv(path);

	// CSVファイルの読み込みが失敗したら、エラーを出す
	if (!csv)
	{
		throw Error(U"CSVの読み込みに失敗しました。");
	}

	// 1行目の列数
	const size_t xCount = csv.columns(0);
	// 行数
	const size_t yCount = csv.rows();

	// マップデータ
	Grid<int> map(xCount, yCount);

	// マップデータに一つずつ代入
	for (size_t y = 0; y < yCount; ++y)
	{
		for (size_t x = 0; x < xCount; ++x)
		{
			map[y][x] = csv.get<int>(y, x);
		}
	}

	return map;
}

// マップを描画する関数
void DrawMapChips(const Grid<int>& grid, const Texture& texture)
{
	for (size_t y = 0; y < grid.height(); ++y)
	{
		for (size_t x = 0; x < grid.width(); ++x)
		{
			// そのマスのマップ情報を取得
			const int mapChip = grid[y][x];

			// -1(そのマスは空白)なら、このループをスキップ
			if (mapChip == -1)
			{
				continue;
			}

			const int chipX = (mapChip % ChipNumX) * ChipSize;
			const int chipY = (mapChip / ChipNumY) * ChipSize;

			// マップ情報が指す場所のタイル画像を描画
			texture(chipX, chipY, ChipSize, ChipSize)
				.draw(x * ChipSize, y * ChipSize);
		}
	}
}

void Main()
{
	// ウィンドウの幅を設定
	Window::Resize(512, 512);
	// 背景色を設定
	Scene::SetBackground(Color(5, 25, 75));

	// 使用するマップチップ画像を用意
	// https://pipoya.net/sozai/assets/map-chip_tileset32/
	Texture forestTile(U"map.png");
	// プレイヤーの画像は絵文字を使用
	// https://github.com/lriki/Siv3D-PixelArt
	Texture player(U"Siv3D-kun.png");

	// 1次レイヤー (地面)
	const Grid<int> mapLayer1 = LoadCSV(U"map_layer1.csv");
	// 2次レイヤー (装飾物1)
	const Grid<int> mapLayer2 = LoadCSV(U"map_layer2.csv");
	// 3次レイヤー (装飾物2)
	const Grid<int> mapLayer3 = LoadCSV(U"map_layer3.csv");
	// 当たり判定
	const Grid<int> mapCollision = LoadCSV(U"map_collision.csv");

	// 現在のプレイヤーの位置
	//   Point型… 座標情報(x, y)をint32型(整数型)で格納する
	//   Vec2型…  座標情報(x, y)をdouble型(実数型)で格納する
	Point playerCell(7, 10);
	// プレイヤーが進む位置
	Point playerNextCell = playerCell;

	// 歩行の速さ
	const double walkSpeed = 4.0;

	// 歩行の進捗
	//   移動開始: 0.0
	//   移動完了: 1.0
	double walkProgress = 1.0;

	// 歩行方向
	//   0 | 北, 北東, 東, 南東, 南, 南西, 西, 北西 | 7
	int direction = 4;

	// 歩行方向に対するSiv3Dくんの停止画像の始点
	//   { 北, 北東, 東, 南東, 南, 南西, 西, 北西 }
	const Array<Vec2> startPosOfSiv3D = {
		Vec2(0 + CharaSizeX, 84),
		Vec2(60 + CharaSizeX, 84),
		Vec2(0 + CharaSizeX, 56),
		Vec2(60 + CharaSizeX, 28),
		Vec2(0 + CharaSizeX, 0),
		Vec2(60 + CharaSizeX, 0),
		Vec2(0 + CharaSizeX, 28),
		Vec2(60 + CharaSizeX, 56)
	};

	// 2Dカメラ
	// 中心:   カメラの中心をプレイヤーの位置にする
	// 拡大率: 1.0
	Camera2D camera(
		(playerCell * ChipSize + Vec2(ChipSize / 2, ChipSize / 2)).asPoint(),
		1.0,
		Camera2DParameters::NoControl()
	);

	while (System::Update())
	{
		///////////////////////////////////
		//  移動に関する処理
		///////////////////////////////////

		// プレイヤーが移動中でない場合、矢印キーでの操作を許可する
		if (playerCell == playerNextCell)
		{
			//  direction
			//    0 | 北, 北東, 東, 南東, 南, 南西, 西, 北西 | 7
			if (KeyLeft.pressed() && !KeyRight.pressed())
			{
				direction = 6;
				--playerNextCell.x;
				if (KeyUp.pressed() && !KeyDown.pressed())
				{
					direction = 7;
					--playerNextCell.y;
				}
				else if (KeyDown.pressed() && !KeyUp.pressed())
				{
					direction = 5;
					++playerNextCell.y;
				}
			}
			else if (KeyRight.pressed() && !KeyLeft.pressed())
			{
				direction = 2;
				++playerNextCell.x;
				if (KeyUp.pressed() && !KeyDown.pressed())
				{
					direction = 1;
					--playerNextCell.y;
				}
				else if (KeyDown.pressed() && !KeyUp.pressed())
				{
					direction = 3;
					++playerNextCell.y;
				}
			}
			else if (KeyUp.pressed() && !KeyDown.pressed())
			{
				--playerNextCell.y;
				direction = 0;
				if (KeyLeft.pressed() && !KeyRight.pressed())
				{
					direction = 7;
					--playerNextCell.x;
				}
				else if (KeyRight.pressed() && !KeyLeft.pressed())
				{
					direction = 1;
					++playerNextCell.x;
				}
			}
			else if (KeyDown.pressed() && !KeyUp.pressed())
			{
				direction = 4;
				++playerNextCell.y;
				if (KeyLeft.pressed() && !KeyRight.pressed())
				{
					direction = 5;
					--playerNextCell.x;
				}
				else if (KeyRight.pressed() && !KeyLeft.pressed())
				{
					direction = 3;
					++playerNextCell.x;
				}
			}

			// マップの範囲外に移動しようとしているとき、マップの範囲内に収める
			//   Clamp関数… 引数1の数値を引数2～引数3の範囲に収める
			playerNextCell.x = Clamp(
				playerNextCell.x,
				0,
				static_cast<int>(mapLayer1.width() - 1)
			);
			playerNextCell.y = Clamp(
				playerNextCell.y,
				0,
				static_cast<int>(mapLayer1.height() - 1)
			);

			// 通行できない場所に移動しようとしているとき、移動しない
			if (mapCollision[playerNextCell] != -1)
			{
				playerNextCell = playerCell;
			}
			else if (playerCell != playerNextCell)
			{
				// 歩行開始
				walkProgress = 0.0;
			}
		}

		// 歩行中の場合
		if (playerCell != playerNextCell)
		{
			// 歩行の進捗を進める
			walkProgress += Scene::DeltaTime() * walkSpeed;
			camera.jumpTo(
				(playerCell.lerp(playerNextCell, walkProgress) * ChipSize
				+ Vec2(ChipSize / 2, ChipSize / 2)).asPoint(),
				1.0
			);

			// 歩行の進捗が1.0以上になったら
			if (walkProgress >= 1.0)
			{
				// 現在の位置を移動しようとしている位置にする
				playerCell = playerNextCell;
				walkProgress = 1.0;
			}
		}

		///////////////////////////////////
		//  描画処理
		///////////////////////////////////

		// 2Dカメラを更新
		camera.update();
		{
			// 2Dカメラの設定から Transformer2D を作成
			//   カメラが移動すると共に、このスコープ(このかもめ括弧内)の
			//   オブジェクトの描画座標が動的に変わるようになる
			const auto t = camera.createTransformer();

			// 1次レイヤー (地面) の描画
			DrawMapChips(mapLayer1, forestTile);
			// 2次レイヤー (装飾物1) の描画
			DrawMapChips(mapLayer2, forestTile);
			// 3次レイヤー (装飾物2) の描画
			DrawMapChips(mapLayer3, forestTile);
		}

		// テクスチャ拡大描画時に、綺麗に表示されるようにする
		// (フィルタリングしないサンプラーステートを適用)
		ScopedRenderStates2D renderState(SamplerState::ClampNearest);

		// 足先
		//   -1: 右
		//    0: 中立
		//    1: 左
		int playerFoot = 0;
		if (walkProgress < 0.5)
		{
			playerFoot = -1;
		}
		else if (walkProgress < 1.0)
		{
			playerFoot = 1;
		}

		// プレイヤーを描画
		player(
			startPosOfSiv3D[direction].movedBy(playerFoot * CharaSizeX, 0),
			CharaSizeX,
			CharaSizeY
		)
		.scaled(2.5)
		.draw(
			Arg::bottomCenter(
				Vec2(512/2, 512/2).movedBy(0, 14)
			)
		);
	}
}
