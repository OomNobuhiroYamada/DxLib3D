#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson42_6
* @author N.Yamada
* @date 2023/01/15
*
* @details オリジナルシェーダーを使用した3Dモデルの描画基本6 （法線マップ付き剛体メッシュのディレクショナルライトあり描画）
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
	float modelRotateAngle;

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

	// プログラマブルシェーダーモデル３．０が使用できない場合はエラーを表示して終了
	if(GetValidShaderVersion() < 300)
	{
		// エラー表示
		DrawString(0, 0, "プログラマブルシェーダー３．０が使用できない環境のようです", GetColor(255, 255, 255));

		// キー入力待ち
		WaitKey();

		// DXライブラリの後始末
		DxLib_End();

		// ソフト終了
		return 0;
	}

	// 頂点シェーダーを読み込む
	vertexShaderHandle = LoadVertexShader("Resource/NormalMesh_DirLight_NrmMapVS.vso");

	// ピクセルシェーダーを読み込む
	pixelShaderHandle = LoadPixelShader("Resource/NormalMesh_DirLight_NrmMapPS.pso");

	// 剛体メッシュモデルを読み込む
	modelHandle = MV1LoadModel("Resource/NormalBox_NrmMap.mv1");

	// モデルの回転角度を初期化
	modelRotateAngle = 0.0f;

	// モデルの描画にオリジナルシェーダーを使用する設定をＯＮにする
	MV1SetUseOrigShader(true);

	// 使用する頂点シェーダーをセット
	SetUseVertexShader(vertexShaderHandle);

	// 使用するピクセルシェーダーをセット
	SetUsePixelShader(pixelShaderHandle);

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// モデルの見える位置にカメラを配置
	SetCameraPositionAndTarget_UpVecY(VGet(-400.0f, 200.0f, -400.0f), VGet(0.0f, 0.0f, 0.0f));

	// ESCキーが押されるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面を初期化
		ClearDrawScreen();

		// モデルの回転値を加算
		modelRotateAngle += 0.002f;

		// モデルの回転角度を変更
		MV1SetRotationXYZ(modelHandle, VGet(0.0f, modelRotateAngle, 0.0f));

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
