#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson42_3
* @author N.Yamada
* @date 2023/01/15
*
* @details オリジナルシェーダーを使用した3Dモデルの描画基本3 （剛体メッシュのポイントライトあり描画）
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／ - 1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int modelHandle;
	int pixelShaderHandle;
	int vertexShaderHandle;
	float lightRotateAngle;

	// ウインドウモードで起動
	ChangeWindowMode(true);

	// Direct3D9Ex を使用する
	SetUseDirect3DVersion(DX_DIRECT3D_9EX);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		// エラーが発生したら直ちに終了
		return -1;
	}

	// プログラマブルシェーダーモデル２．０が使用できない場合はエラーを表示して終了
	if(GetValidShaderVersion() < 200)
	{
		// エラー表示
		DrawString(0, 0, "プログラマブルシェーダー２．０が使用できない環境のようです", GetColor(255, 255, 255));

		// キー入力待ち
		WaitKey();

		// DXライブラリの後始末
		DxLib_End();

		// ソフト終了
		return 0;
	}

	// 頂点シェーダーを読み込む
	vertexShaderHandle = LoadVertexShader("Resource/NormalMesh_PointLightVS.vso");

	// ピクセルシェーダーを読み込む
	pixelShaderHandle = LoadPixelShader("Resource/NormalMesh_PointLightPS.pso");

	// 剛体メッシュモデルを読み込む
	modelHandle = MV1LoadModel("Resource/NormalBox.mqo");

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader(true);

	// 使用する頂点シェーダーをセット
	SetUseVertexShader(vertexShaderHandle);

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader(pixelShaderHandle);

	// 観察しやすい位置にカメラを移動
	SetCameraPositionAndTarget_UpVecY(VGet(400.0f, 400.0f, -400.0f), VGet(0.0f, 0.0f, 0.0f));

	// ライトの位置を回転する値を初期化
	lightRotateAngle = 0.0f;

	// 標準ライトのタイプをポイントライトにする
	ChangeLightTypePoint(VGet(0.0f, 0.0f, 0.0f), 700.0f, 0.391586f, 0.001662f, 0.0f);

	// ESCキーが押されるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面を初期化
		ClearDrawScreen();

		// ライトの位置の回転値を加算
		lightRotateAngle += 0.02f;

		// ライトの位置の更新
		SetLightPosition(VGet(sinf(lightRotateAngle) * 400.0f, 200.0f, cosf(lightRotateAngle) * 400.0f));

		// モデルを描画
		MV1DrawModel(modelHandle);

		// 裏画面の内容を表画面に反映させる
		ScreenFlip();
	}

	// 読み込んだ頂点シェーダーの削除
	DeleteShader(vertexShaderHandle);

	// 読み込んだピクセルシェーダーの削除
	DeleteShader(pixelShaderHandle);

	// 読み込んだモデルの削除
	MV1DeleteModel(modelHandle);

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
