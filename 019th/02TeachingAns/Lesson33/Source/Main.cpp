#include "DxLib.h"
#include "CheckKey.h"
/**
* @file
* @brief Lesson33
* @author N.Yamada
* @date 2023/01/03
*
* @details 迷路を3Dで表示
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const float BLOCK_SIZE = 1000.0f;	//!< ブロックのサイズ
const int   BLOCK_NUM_X = 16;		//!< Ｘ方向のブロック数
const int   BLOCK_NUM_Z = 16;		//!< Ｚ方向のブロック数
const float CAMERA_Y = 500.0f;		//!< カメラの高さ
const int   MONSTER_NUM = 10;		//!< モンスター出現数

char map[BLOCK_NUM_Z][BLOCK_NUM_X] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,1,1,2,1,0,1,1,1,0,1,1,0,1,1,0,
	0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,
	0,1,1,2,1,1,1,0,1,0,0,1,1,1,1,0,
	0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,0,
	0,0,0,1,1,1,0,1,0,0,0,1,0,0,1,0,
	0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,
	0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,
	0,0,0,1,1,1,1,1,0,0,0,1,0,0,1,0,
	0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0,
	0,1,1,1,0,1,0,0,0,0,1,0,1,0,1,0,
	0,1,0,1,1,1,0,0,0,1,1,0,1,0,0,0,
	0,1,0,1,0,0,0,1,1,1,0,0,1,1,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};	//!< マップ( 1:道  0:壁 )

/**
* @enum Direction
* @brief 方向
*/
enum Direction
{
	East = 0,	//!< x軸プラス方向
	South,		//!< z軸マイナス方向
	West,		//!< x軸マイナス方向
	North,		//!< z軸プラス方向
};

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int frameNo;		// フレーム番号
	VECTOR camPos;		// カメラの座標
	VECTOR camTarg;		// カメラの注視点

	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 壁モデルの読みこみ
	int kabeModel = MV1LoadModel("Resource/Kabe.mqo");

	// ゴブリン読み込み
	int monsterModel = MV1LoadModel("Resource/Goblin.x");
	MV1SetScale(monsterModel, VGet(6.0f, 6.0f, 6.0f));		// ゴブリンのモデルサイズがマップに対して小さいので拡大する

	// モンスターの出現情報配列を初期化
	VECTOR* monsterData[MONSTER_NUM];
	for(int i=0; i<MONSTER_NUM; i++)
	{
		monsterData[i] = NULL;
	}

	// 位置と向きと入力状態の初期化
	int posX = 1;
	int posZ = 1;
	int dir = Direction::East;

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	bool monsterSpawn = false;

	// メインループ
	// エスケープキーが押されるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面をクリアする
		ClearDrawScreen();

		// 移動量
		int moveX = 0;
		int moveZ = 0;

		// 上が押されたら向いている方向に移動する
		if(CheckDownKey(KEY_INPUT_W) == 1)
		{
			// 向きによって移動方向が変わる
			switch(dir)
			{
			case Direction::East: // Ｘ軸プラス方向
				moveX = 1;
				break;

			case Direction::South: // Ｚ軸マイナス方向
				moveZ = -1;
				break;
				
			case Direction::West: // Ｘ軸マイナス方向
				moveX = -1;
				break;

			case Direction::North: // Ｚ軸プラス方向
				moveZ = 1;
				break;
			}
		}

		// 下が押されたら向いている方向と逆方向に移動する
		if(CheckDownKey(KEY_INPUT_S) == 1)
		{
			// 向きによって移動方向が変わる
			switch(dir)
			{
			case Direction::East: // Ｘ軸プラス方向
				moveX = -1;
				break;
			
			case Direction::South: // Ｚ軸マイナス方向
				moveZ = 1;
				break;
			
			case Direction::West: // Ｘ軸マイナス方向
				moveX = 1;
				break;
			
			case Direction::North: // Ｚ軸プラス方向
				moveZ = -1;
				break;
			}
		}

		// 移動量があったら移動判定をする
		if(moveX != 0 || moveZ != 0)
		{
			// 移動先のマスが道だったら移動する
			if(map[posZ + moveZ][posX + moveX] >= 1)
			{
				posX += moveX;
				posZ += moveZ;
			}
		}

		// スイッチマップに到達したら眼前にモンスターを出現させる
		if(map[posZ][posX] == 2)
		{
			for(int i=0; i<MONSTER_NUM; i++)
			{
				// 配列の空いている箇所を探して出現させる
				if(monsterData[i] == NULL)
				{
					// 一旦自分と同じ場所に指定
					monsterData[i] = new VECTOR;
					monsterData[i]->x = (float)posX;
					monsterData[i]->z = (float)posZ;

					// 向いている方向にずらす
					switch(dir)
					{
					case Direction::East: // Ｘ軸プラス方向
						monsterData[i]->x += 1;
						monsterData[i]->y = 90.0f * DX_PI_F / 180;	// 座標として使われない変数yにモデルの回転量を入れておく
						break;

					case Direction::South: // Ｚ軸マイナス方向
						monsterData[i]->z -= 1;
						monsterData[i]->y = 180.0f * DX_PI_F / 180;	// 座標として使われない変数yにモデルの回転量を入れておく
						break;

					case Direction::West: // Ｘ軸マイナス方向
						monsterData[i]->x -= 1;
						monsterData[i]->y = -90.0f * DX_PI_F / 180;	// 座標として使われない変数yにモデルの回転量を入れておく
						break;

					case Direction::North: // Ｚ軸プラス方向
						monsterData[i]->z += 1;
						monsterData[i]->y = 0.0f * DX_PI_F / 180;	// 座標として使われない変数yにモデルの回転量を入れておく
						break;
					}

					// 一度出現したらもう出現しない
					map[posZ][posX] = 1;

					// 出現させたのでfor文から抜ける
					break;
				}
			}
		}

		// 左が押されていたら向いている方向を左に９０度変更する
		if(CheckDownKey(KEY_INPUT_A) == 1)
		{
			if(dir == Direction::East)
			{
				dir = Direction::North;
			}
			else
			{
				dir--;
			}
		}

		// 右が押されていたら向いている方向を右に９０度変更する
		if(CheckDownKey(KEY_INPUT_D) == 1)
		{
			if(dir == Direction::North)
			{
				dir = Direction::East;
			}
			else
			{
				dir++;
			}
		}

		// カメラの座標をセット
		camPos = VGet(posX * BLOCK_SIZE, CAMERA_Y, posZ * BLOCK_SIZE);

		// カメラの注視ベクトルをセット
		switch(dir)
		{
		case Direction::East: // Ｘ軸プラス方向
			camTarg = VGet(1.0f, 0.0f, 0.0f);
			break;

		case Direction::South: // Ｚ軸マイナス方向
			camTarg = VGet(0.0f, 0.0f, -1.0f);
			break;

		case Direction::West: // Ｘ軸マイナス方向
			camTarg = VGet(-1.0f, 0.0f, 0.0f);
			break;

		case Direction::North: // Ｚ軸プラス方向
			camTarg = VGet(0.0f, 0.0f, 1.0f);
			break;
		}
		camTarg = VAdd(camPos, camTarg);

		// カメラの位置と向きをセットする
		SetCameraPositionAndTarget_UpVecY(camPos, camTarg);

		// マップを描画する
		for(int i=0; i<BLOCK_NUM_Z; i++)
		{
			for(int j=0; j<BLOCK_NUM_X; j++)
			{
				// 道ではないところは描画しない
				if(map[i][j] == 0)
				{
					continue;
				}

				// 壁モデルの座標を変更する
				MV1SetPosition(kabeModel, VGet(j * BLOCK_SIZE, 0.0f, i * BLOCK_SIZE));

				// ４方の壁の状態で描画するフレーム番号を変更する
				frameNo = 0;
				if(map[i][j + 1] == 0)
				{
					frameNo += 1;
				}
				if(map[i][j - 1] == 0)
				{
					frameNo += 2;
				}
				if(map[i + 1][j] == 0)
				{
					frameNo += 4;
				}
				if(map[i - 1][j] == 0)
				{
					frameNo += 8;
				}

				// 割り出した番号のフレームを描画する
				MV1DrawFrame(kabeModel, frameNo);

				// モンスター出現位置配列の中で有効なものを描画
				for(int k=0; k<MONSTER_NUM; k++)
				{
					if(monsterData[k] != NULL)
					{
						// モンスターの座標をセット
						MV1SetPosition(monsterModel, VGet(monsterData[k]->x * BLOCK_SIZE, 0.0f, monsterData[k]->z * BLOCK_SIZE));

						// モンスターの回転をセット
						MV1SetRotationXYZ(monsterModel, VGet(0.0f, monsterData[k]->y, 0.0f));

						// モンスターを描画
						MV1DrawModel(monsterModel);
					}
				}
			}
		}

		DrawFormatString(5, 5, 65535, "%d, %d", posX, posZ);
		DrawFormatString(5, 25, 65535, "%d", dir);

		// 裏画面の内容を表画面に反映する
		ScreenFlip();

		// １７ミリ秒(約秒間６０フレームだった時の１フレームあたりの経過時間)
// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
		}
	}

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
