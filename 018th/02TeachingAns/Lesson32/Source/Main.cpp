#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson32
* @author N.Yamada
* @date 2023/01/03
*
* @details カメラを移動・回転させてみよう
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const float	CAMERA_ANGLE_SPEED = 3.0f;	//!< カメラの回転速度
const float	LINE_AREA_SIZE = 10000.0f;	//!< ラインを描く範囲
const int	LINE_NUM = 50;				//!< ラインの数

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
	int modelHandle = MV1LoadModel("Resource/Hero.x");

	// カメラのクリッピング距離を設定
	SetCameraNearFar(16.0f, 3800.0f);

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	VECTOR cameraPosition = { 0.0f, 180.0f, 0.0f };
	float cameraHAngle = 0.0f;
	float cameraVAngle = 0.0f;

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面のクリア
		ClearDrawScreen();

		// WASDキーでカメラの進行ベクトルを操作
		VECTOR moveVector = { 0.0f, 0.0f, 0.0f };
		if(CheckHitKey(KEY_INPUT_D) == 1)
		{
			moveVector.x += 10.0f;
		}
		if(CheckHitKey(KEY_INPUT_A) == 1)
		{
			moveVector.x -= 10.0f;
		}
		if(CheckHitKey(KEY_INPUT_W) == 1)
		{
			moveVector.z += 10.0f;
		}
		if(CheckHitKey(KEY_INPUT_S) == 1)
		{
			moveVector.z -= 10.0f;
		}

		// 矢印キーでカメラの角度を操作
		if(CheckHitKey(KEY_INPUT_LEFT) == 1)
		{
			cameraHAngle -= CAMERA_ANGLE_SPEED;

			// -180～180の間に収める
			if(cameraHAngle <= -180.0f)
			{
				cameraHAngle += 360.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_RIGHT) == 1)
		{
			cameraHAngle += CAMERA_ANGLE_SPEED;

			// -180～180の間に収める
			if(cameraHAngle >= 180.0f)
			{
				cameraHAngle -= 360.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_DOWN) == 1)
		{
			cameraVAngle += CAMERA_ANGLE_SPEED;

			// 真上より先には回転しない
			if(cameraVAngle >= 90.0f)
			{
				cameraVAngle = 90.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_UP) == 1)
		{
			cameraVAngle -= CAMERA_ANGLE_SPEED;

			// 真下より先には回転しない
			if(cameraVAngle <= -90.0f)
			{
				cameraVAngle = -90.0f;
			}
		}

		// 3Dモデルの位置を設定して描画
		{
			MV1SetPosition(modelHandle, VGet(0.0f, 0.0f, 500.0f));
			MV1DrawModel(modelHandle);

			MV1SetPosition(modelHandle, VGet(0.0f, 0.0f, -500.0f));
			MV1DrawModel(modelHandle);

			MV1SetPosition(modelHandle, VGet(500.0f, 0.0f, 0.0f));
			MV1DrawModel(modelHandle);

			MV1SetPosition(modelHandle, VGet(-500.0f, 0.0f, 0.0f));
			MV1DrawModel(modelHandle);

			MV1SetPosition(modelHandle, VGet(0.0f, 200.0f, 0.0f));
			MV1DrawModel(modelHandle);

		}

		// 位置関係が分かるように地面にラインを描画する
		{
			VECTOR pos1;
			VECTOR pos2;

			SetUseZBufferFlag(true);

			pos1 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			pos2 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, LINE_AREA_SIZE / 2.0f);
			for(int i=0; i<=LINE_NUM; i++)
			{
				DrawLine3D(pos1, pos2, GetColor(255, 255, 255));
				pos1.x += LINE_AREA_SIZE / LINE_NUM;
				pos2.x += LINE_AREA_SIZE / LINE_NUM;
			}

			pos1 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			pos2 = VGet(LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			for(int i=0; i<LINE_NUM; i++)
			{
				DrawLine3D(pos1, pos2, GetColor(255, 255, 255));
				pos1.z += LINE_AREA_SIZE / LINE_NUM;
				pos2.z += LINE_AREA_SIZE / LINE_NUM;
			}

			SetUseZBufferFlag(false);
		}

		//----------------------------------------------------------------------
		// X軸とY軸の回転から回転行列を作成
		MATRIX rotateMatrix = MMult(MGetRotX(cameraVAngle / 180.0f * DX_PI_F), MGetRotY(cameraHAngle / 180.0f * DX_PI_F));

		// 回転行列で進行ベクトルを変換して、カメラの向きを考慮した進行ベクトルを求める
		VECTOR cameraMoveVector = VTransformSR(moveVector, rotateMatrix);

		// カメラ座標に進行ベクトルを加算
		cameraPosition = VAdd(cameraPosition, cameraMoveVector);

		// 回転行列×平行移動行列でカメラ行列を作る
		MATRIX cameraMatrix = MMult(rotateMatrix, MGetTranslate(cameraPosition));

		// カメラ行列の逆行列をビュー行列として設定
		SetCameraViewMatrix(MInverse(cameraMatrix));
		//----------------------------------------------------------------------

		// 裏画面の内容を表画面に反映
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
