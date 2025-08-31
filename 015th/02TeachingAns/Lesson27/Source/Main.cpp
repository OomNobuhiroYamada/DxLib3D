#include "DxLib.h"
/**
* @file
* @brief Lesson27
* @author N.Yamada
* @date 2023/01/02
*
* @details 3Dモデルを画面に表示しよう
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 現在のカウントを取得する
	int time = GetNowCount();

	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	//----------------------------------------------------------
	// 3Dモデルの読み込み
	int modelHandle = MV1LoadModel("Resource/Hero.x");

	// 3Dモデルの位置を設定
	MV1SetPosition(modelHandle, VGet(0.0f, 0.0f, 0.0f));

	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(
		VGet(0.0f, 100.0f, -200.0f),
		VGet(0.0f, 100.0f, 0.0f));

	// カメラのクリッピング距離を設定
	SetCameraNearFar(16.0f, 3800.0f);
	//----------------------------------------------------------

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面のクリア
		ClearDrawScreen();

		//----------------------------------------------------------
		// 3Dモデルの描画
		MV1DrawModel(modelHandle);
		//----------------------------------------------------------

		// 裏画面の内容を表画面に反映
		ScreenFlip();

		// １７ミリ秒(約秒間６０フレームだった時の１フレームあたりの経過時間)
		// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
		}

	}

	//----------------------------------------------------------
	// 3Dモデル削除
	MV1DeleteModel(modelHandle);
	//----------------------------------------------------------

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
