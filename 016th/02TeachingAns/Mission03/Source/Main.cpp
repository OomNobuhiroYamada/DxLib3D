#include "DxLib.h"
#include "CheckKey.h"
/**
* @file
* @brief Mission03
* @author N.Yamada
* @date 2023/01/02
*
* @details 3Dモデルを画面に表示しよう
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @enum Character
* @brief 関数ポインタのインデックス
*/
enum Character
{
	Hero = 0,	//!< ヒーロー
	Goblin,		//!< ゴブリン
	Golem,		//!< ゴーレム
	Bee,		//!< ビー

	Length,		//!< 列挙数
};

const char *resourceFile[Character::Length] ={
	"Resource/Hero.x",
	"Resource/Goblin.x",
	"Resource/Golem.x",
	"Resource/Bee.x",
};	//!< 画像リソースパス

int modelHandle[Character::Length];	//!< 3Dモデルのハンドル配列

void DrawHero();
void DrawGoblin();
void DrawGolem();
void DrawBee();

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	//-------------------------------------------------------
	// 描画処理ポインタ配列
	void(*drawCharacterMethod[])(void) =
	{
		DrawHero,
		DrawGoblin,
		DrawGolem,
		DrawBee,
		NULL
	};
	//-------------------------------------------------------

	// 3Dモデルの読み込み
	for(int i=0; i<Character::Length; i++)
	{
		modelHandle[i] = MV1LoadModel(resourceFile[i]);
	}

	// カメラのクリッピング距離を設定
	SetCameraNearFar(16.0f, 3800.0f);

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	//-------------------------------------------------------
	// 環境光をX軸のプラス側に向けて放つ
	ChangeLightTypeDir(VGet(1.0f, 0.0f, 0.0f));
	//-------------------------------------------------------

	// 描画するモデルの番号
	int drawIndex = 0;

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		if(CheckDownKey(KEY_INPUT_Z) == 1)
		{
			drawIndex++;
			if(drawIndex >= Character::Length)
			{
				drawIndex = 0;
			}
		}

		// 描画
		{
			// 画面のクリア
			ClearDrawScreen();

			//-------------------------------------------------------
			// 3Dモデルの描画
			drawCharacterMethod[drawIndex]();
			//-------------------------------------------------------

			// 裏画面の内容を表画面に反映
			ScreenFlip();
		}
	}

	// 3Dモデル削除
	for(int i=0; i<Character::Length; i++)
	{
		MV1DeleteModel(modelHandle[i]);
	}

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}

/**
* @fn DrawHero
* @brief Heroを表示
*/
void DrawHero()
{
	//-------------------------------------------------------
	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 125.0f, -300.0f), VGet(0.0f, 125.0f, 0.0f));

	// 3Dモデルの描画
	MV1DrawModel(modelHandle[Character::Hero]);
	//-------------------------------------------------------
}

/**
* @fn DrawGoblin
* @brief Goblinを表示
*/
void DrawGoblin()
{
	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(-60.0f, 90.0f, -95.0f), VGet(0.0f, 120.0f, -20.0f));

	// 3Dモデルの描画
	MV1DrawModel(modelHandle[Character::Goblin]);
}

/**
* @fn DrawGolem
* @brief Golemを表示
*/
void DrawGolem()
{
	//-------------------------------------------------------
	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(-40.0f, 190.0f, -115.0f), VGet(0.0f, 180.0f, -15.0f));

	// 3Dモデルの描画
	MV1DrawModel(modelHandle[Character::Golem]);
	//-------------------------------------------------------
}

/**
* @fn DrawBee
* @brief Beeを表示
*/
void DrawBee()
{
	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 150.0f, -400.0f), VGet(0.0f, 150.0f, 0.0f));

	// ビーの座標
	MV1SetPosition(modelHandle[Character::Bee], VGet(100.0f, 0.0f, 0.0f));

	// 3Dモデルの描画
	MV1DrawModel(modelHandle[Character::Bee]);

	// ビーの座標
	MV1SetPosition(modelHandle[Character::Bee], VGet(-100.0f, 0.0f, 0.0f));

	// 3Dモデルの描画
	MV1DrawModel(modelHandle[Character::Bee]);
}
