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
	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// 3Dモデルの読み込み


	// 3Dモデルの位置を設定


	// カメラの位置と向きを設定


	// カメラのクリッピング距離を設定


	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面のクリア
		ClearDrawScreen();

		// 3Dモデルの描画


		// 裏画面の内容を表画面に反映
		ScreenFlip();
	}

	// 3Dモデル削除


	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
