#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson42_2
* @author N.Yamada
* @date 2023/01/15
*
* @details オリジナルシェーダーを使用した3Dモデルの描画基本5 （剛体メッシュのディレクショナルライトとポイントライトあり描画）
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const int   DRAW_NUM = 3;
const float SPACE = 512.0f;

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
	int dirLightHandle;
	int pointLightHandle;
	float drawX, drawZ;

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
	vertexShaderHandle = LoadVertexShader("Resource/NormalMesh_DirPointLightVS.vso");

	// ピクセルシェーダーを読み込む
	pixelShaderHandle = LoadPixelShader("Resource/NormalMesh_DirPointLightPS.pso");

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
	SetCameraPositionAndTarget_UpVecY(VGet(800.0f, 400.0f, -800.0f), VGet(0.0f, 0.0f, 0.0f));

	// ライトの位置を回転する値を初期化
	lightRotateAngle = 0.0f;

	// 標準ライトを無効にする
	SetLightEnable(false);

	// ディレクショナルライトを作成する
	dirLightHandle = CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));

	// ディレクショナルライトのアンビエントカラーを抑える
	SetLightAmbColorHandle(dirLightHandle, GetColorF(0.0f, 0.1f, 0.0f, 0.0f));

	// ディレクショナルライトのディフューズカラーを緑にする
	SetLightDifColorHandle(dirLightHandle, GetColorF(0.0f, 1.0f, 0.0f, 0.0f));

	// ポイントライトを作成する
	pointLightHandle = CreatePointLightHandle(VGet(0.0f, 0.0f, 0.0f), 7000.0f, 1.016523f, 0.000100f, 0.000010f);

	// ポイントライトのアンビエントカラーを無効にする
	SetLightAmbColorHandle(pointLightHandle, GetColorF(0.0f, 0.0f, 0.0f, 0.0f));

	// ポイントライトのディフューズカラーを強い赤色にする
	SetLightDifColorHandle(pointLightHandle, GetColorF(2.0f, 0.0f, 0.0f, 0.0f));

	// ESCキーが押されるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面を初期化
		ClearDrawScreen();

		// ポイントライトの位置の回転値を加算
		lightRotateAngle += 0.02f;

		// ポイントライトの位置の更新
		SetLightPositionHandle(pointLightHandle, VGet(sinf(lightRotateAngle) * 400.0f, 400.0f, cosf(lightRotateAngle) * 400.0f));

		// モデルを描画
		drawZ = -(DRAW_NUM - 1) * SPACE / 2.0f;
		for(int i=0; i<DRAW_NUM; i++)
		{
			drawX = -(DRAW_NUM - 1) * SPACE / 2.0f;
			for(int j=0; j<DRAW_NUM; j++)
			{
				// 位置を設定
				MV1SetPosition(modelHandle, VGet(drawX, 0.0f, drawZ));

				// 描画
				MV1DrawModel(modelHandle);

				drawX += SPACE;
			}
			drawZ += SPACE;
		}

		// 裏画面の内容を表画面に反映させる
		ScreenFlip();
	}

	// ディレクショナルライトの削除
	DeleteLightHandle(dirLightHandle);

	// ポイントライトの削除
	DeleteLightHandle(pointLightHandle);

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
