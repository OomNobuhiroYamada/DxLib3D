#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson28
* @author N.Yamada
* @date 2023/01/02
*
* @details 方向入力によるキャラクター移動
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const float MOVESPEED = 10.0f;				//!< 移動速度

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int modelHandle;
	VECTOR position;
	float angle;

	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 3Dモデルの読み込み
	modelHandle = MV1LoadModel("Resource/Hero.x");

	// 向きを初期化
	angle = 0.0f;

	// 3Dモデルの座標を初期化
	position = VGet(0.0f, 0.0f, 0.0f);

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 300.0f, -500.0f), VGet(0.0f, 100.0f, 0.0f));

	// カメラのクリッピング距離を設定
	SetCameraNearFar(16.0f, 5000.0f);

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面のクリア
		ClearDrawScreen();

		// 押されたキーから移動するベクトルを作る
		VECTOR direction;
		direction.x = 0.0f;
		direction.z = 0.0f;
		if(CheckHitKey(KEY_INPUT_A) != 0)
		{
			direction.x = -1.0f;
		}
		if(CheckHitKey(KEY_INPUT_D) != 0)
		{
			direction.x = 1.0f;
		}
		if(CheckHitKey(KEY_INPUT_W) != 0)
		{
			direction.z = 1.0f;
		}
		if(CheckHitKey(KEY_INPUT_S) != 0)
		{
			direction.z = -1.0f;
		}

		// 方向入力に従ってキャラクターを移動
		if(direction.x != 0.0f || direction.z != 0.0f)
		{
			angle = atan2f(-direction.x, -direction.z);
			position.x -= sinf(angle) * MOVESPEED;
			position.z -= cosf(angle) * MOVESPEED;
		}

		// 新しい向きをセット


		// 3Dモデルに新しい座標をセット


		// 3Dモデルの描画
		MV1DrawModel(modelHandle);

		// 裏画面の内容を表画面に反映
		ScreenFlip();
	}

	// 3Dモデル削除
	MV1DeleteModel(modelHandle);

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
