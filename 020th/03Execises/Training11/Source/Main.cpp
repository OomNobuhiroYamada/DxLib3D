#include "DxLib.h"
#include "DrawTree.h"
#include <math.h>
/**
* @file
* @brief Training11
* @author N.Yamada
* @date 2023/01/06
*
* @details 板ポリを組み合わせて木を作ろう
* @details ＜操作＞
* @details 方向キー：キャラクターモデル移動
* @details A,Dキー：カメラの水平角度を変更
* @details W,Sキー：カメラの垂直角度を変更
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const float	MOVESPEED = 10.0f;					//!< 移動速度
const float	JUMP_POWER = 30.0f;					//!< ジャンプ力
const float GRAVITY = 2.0f;						//!< 重力
const float CAMERA_ANGLE_SPEED = 3.0f;			//!< カメラの回転速度
const float CAMERA_LOOK_AT_HEIGHT = 180.0f;		//!< カメラの注視点の高さ
const float CAMERA_LOOK_AT_DISTANCE = 250.0f;	//!< カメラと注視点の距離
const float	LINE_AREA_SIZE = 10000.0f;			//!< ラインを描く範囲
const int	LINE_NUM = 50;						//!< ラインの数

/**
* @enum Animation
* @brief アニメーションＩＤ
*/
enum Animation
{
	Neutral = 0,
	Run,
	Jump,

	Length,
};
const char* animationName[Animation::Length] = {
	"Neutral",
	"Run",
	"JumpLoop",
};	//!< アニメーション名

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

	// 画像の読み込み
	int graphHandle = LoadGraph("Resource/DxLogo.png");

	// 画像の読み込み
	int treeHandle = LoadGraph("Resource/Tree.png");

	// 3Dモデルの読み込み
	int modelHandle = MV1LoadModel("Resource/Hero.x");

	// 3Dモデルのアニメーション番号を読み込む
	int animationIndex[Animation::Length];			// アニメーション番号
	for(int i = 0; i < Animation::Length; i++)
	{
		animationIndex[i] = MV1GetAnimIndex(modelHandle, animationName[i]);
	}

	// 現在のアニメーション番号
	Animation animationNowIndex = Animation::Neutral;
	Animation animationOldIndex = animationNowIndex;

	// 待機アニメーションをアタッチする
	int attachIndex = MV1AttachAnim(modelHandle, animationIndex[animationNowIndex], -1, FALSE);

	// 待機アニメーションの総時間を取得しておく
	float animationTotalTime = MV1GetAttachAnimTotalTime(modelHandle, attachIndex);

	// アニメーション再生時間を初期化
	float animationNowTime = 0.0f;
	MV1SetAttachAnimTime(modelHandle, attachIndex, animationNowTime);

	// アニメーションで移動をしているフレームの番号を検索する
	int moveAnimFrameIndex = MV1SearchFrame(modelHandle, "root");

	// アニメーションで移動をしているフレームを無効にする
	MV1SetFrameUserLocalMatrix(modelHandle, moveAnimFrameIndex, MV1GetFrameLocalMatrix(modelHandle, moveAnimFrameIndex));

	// カメラの向きを初期化
	float cameraHAngle = 0.0f;
	float cameraVAngle = 20.0f;

	// 向きを初期化
	float angle = 0.0f;

	// ジャンプフラグを倒す
	bool jumpFlag = false;

	// ジャンプ速度をなくす
	float jumpPower = 0.0f;

	// 3Dモデルの座標を初期化
	VECTOR position = VGet(0.0f, 0.0f, 0.0f);

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// カメラのクリッピング距離を設定
	SetCameraNearFar(100.0f, 50000.0f);

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	// Zバッファに書き込む準備
	SetUseZBufferFlag(true);
	SetWriteZBufferFlag(true);

	// メインループ(ESCキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面のクリア
		ClearDrawScreen();

		animationNowIndex = Animation::Neutral;

		// 方向キーでカメラの操作
		if(CheckHitKey(KEY_INPUT_LEFT) == 1)
		{
			cameraHAngle += CAMERA_ANGLE_SPEED;
			if(cameraHAngle >= 180.0f)
			{
				cameraHAngle -= 360.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_RIGHT) == 1)
		{
			cameraHAngle -= CAMERA_ANGLE_SPEED;
			if(cameraHAngle <= -180.0f)
			{
				cameraHAngle += 360.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_UP) == 1)
		{
			cameraVAngle += CAMERA_ANGLE_SPEED;
			if(cameraVAngle >= 80.0f)
			{
				cameraVAngle = 80.0f;
			}
		}
		if(CheckHitKey(KEY_INPUT_DOWN) == 1)
		{
			cameraVAngle -= CAMERA_ANGLE_SPEED;
			if(cameraVAngle <= 0.0f)
			{
				cameraVAngle = 0.0f;
			}
		}

		// 方向入力に従ってキャラクターの移動ベクトルと向きを設定
		VECTOR moveVector = VGet(0.0f, 0.0f, 0.0f);
		if(CheckHitKey(KEY_INPUT_A) == 1)
		{
			angle = 90.0f - cameraHAngle;
			moveVector.x = -MOVESPEED;
		}
		if(CheckHitKey(KEY_INPUT_D) == 1)
		{
			angle = -90.0f - cameraHAngle;
			moveVector.x = MOVESPEED;
		}
		if(CheckHitKey(KEY_INPUT_W) == 1)
		{
			angle = 180.0f - cameraHAngle;
			moveVector.z = MOVESPEED;
		}
		if(CheckHitKey(KEY_INPUT_S) == 1)
		{
			angle = 0.0f - cameraHAngle;
			moveVector.z = -MOVESPEED;
		}

		// 方向入力に従ってキャラクターを移動
		if(moveVector.x != 0.0f || moveVector.z != 0.0f)
		{
			// 走るアニメーション
			animationNowIndex = Animation::Run;

			// カメラの角度に合わせて移動ベクトルを回転してから加算
			VECTOR tempMoveVector;
			float sinParam = sinf(cameraHAngle / 180.0f * DX_PI_F);
			float cosParam = cosf(cameraHAngle / 180.0f * DX_PI_F);
			tempMoveVector.x = moveVector.x * cosParam - moveVector.z * sinParam;
			tempMoveVector.y = 0.0f;
			tempMoveVector.z = moveVector.x * sinParam + moveVector.z * cosParam;

			position = VAdd(position, tempMoveVector);
		}

		// スペースキーが押されたらジャンプ動作をする
		if(CheckHitKey(KEY_INPUT_SPACE) == 1 && jumpFlag == false)
		{
			jumpFlag = true;
			jumpPower = JUMP_POWER;
		}

		// ジャンプをする処理
		if(jumpFlag == true)
		{
			// ジャンプ力を重力分減算する
			jumpPower -= GRAVITY;

			// Y軸の移動ベクトルにジャンプ力を設定
			position.y += jumpPower;

			// 地面にめり込みそうな移動ベクトルになっていたら地面に着地するように調整
			if(position.y <= 0.0f)
			{
				// 現在のY軸の高さ分だけ落下する
				position.y = -position.y;

				// ジャンプ処理終了
				jumpFlag = false;
			}

			// アニメーション切り替え
			animationNowIndex = Animation::Jump;
		}

		if(animationOldIndex != animationNowIndex)
		{
			animationOldIndex = animationNowIndex;

			// 今までアタッチしていたアニメーションをデタッチ
			MV1DetachAnim(modelHandle, attachIndex);

			// 新しいアニメーションをアタッチ
			attachIndex = MV1AttachAnim(modelHandle, animationIndex[animationNowIndex], -1, FALSE);

			// アニメーションの総時間を取得しておく
			animationTotalTime = MV1GetAttachAnimTotalTime(modelHandle, attachIndex);

			// アニメーション再生時間を初期化
			animationNowTime = 0.0f;
		}
		else
		{
			// アニメーション再生時間を進める
			animationNowTime += 0.5f;

			// アニメーション再生時間がアニメーションの総時間を越えていたらループさせる
			if(animationNowTime >= animationTotalTime)
			{
				// 新しいアニメーション再生時間は、アニメーション再生時間からアニメーション総時間を引いたもの
				animationNowTime -= animationTotalTime;
			}
		}

		// 新しいアニメーション再生時間をセット
		MV1SetAttachAnimTime(modelHandle, attachIndex, animationNowTime);

		// 新しい向きをセット
		MV1SetRotationXYZ(modelHandle, VGet(0.0f, angle / 180.0f * DX_PI_F, 0.0f));

		// 3Dモデルに新しい座標をセット
		MV1SetPosition(modelHandle, position);

		// カメラの位置と向きを設定
		{
			VECTOR tempPosition1;
			VECTOR tempPosition2;
			VECTOR cameraPosition;
			VECTOR cameraLookAtPosition;

			// 注視点はキャラクターモデルの座標から CAMERA_LOOK_AT_HEIGHT 分だけ高い位置
			cameraLookAtPosition = position;
			cameraLookAtPosition.y += CAMERA_LOOK_AT_HEIGHT;

			// カメラの位置はカメラの水平角度と垂直角度から算出
			{
				float sinParam;
				float cosParam;

				// 最初に垂直角度を反映した位置を算出
				sinParam = sinf(cameraVAngle / 180.0f * DX_PI_F);
				cosParam = cosf(cameraVAngle / 180.0f * DX_PI_F);
				tempPosition1.x = 0.0f;
				tempPosition1.y = sinParam * CAMERA_LOOK_AT_DISTANCE;
				tempPosition1.z = -cosParam * CAMERA_LOOK_AT_DISTANCE;

				// 次に水平角度を反映した位置を算出
				sinParam = sinf(cameraHAngle / 180.0f * DX_PI_F);
				cosParam = cosf(cameraHAngle / 180.0f * DX_PI_F);
				tempPosition2.x = cosParam * tempPosition1.x - sinParam * tempPosition1.z;
				tempPosition2.y = tempPosition1.y;
				tempPosition2.z = sinParam * tempPosition1.x + cosParam * tempPosition1.z;
			}

			// 算出した座標に注視点の位置を加算したものがカメラの位置
			cameraPosition = VAdd(tempPosition2, cameraLookAtPosition);

			// カメラの設定に反映する
			SetCameraPositionAndTarget_UpVecY(cameraPosition, cameraLookAtPosition);
		}

		// 3Dモデルの描画
		MV1DrawModel(modelHandle);

		// 木を描画
		DrawTree(VGet(-750.0f, 0.0f, 0.0f), 3.0f, treeHandle);
		DrawTree(VGet(750.0f, 0.0f, 0.0f), 3.0f, treeHandle);
		DrawTree(VGet(-750.0f, 0.0f, 1500.0f), 3.0f, treeHandle);
		DrawTree(VGet(750.0f, 0.0f, 1500.0f), 3.0f, treeHandle);

		// ビルボード
		{
			// プレイヤーの頭上に描画する
			DrawBillboard3D(VAdd(position, VGet(0.0f, 250.0f, 0.0f)), 0.5f, 0.5f, 100.0f, 0.0f, graphHandle, true);
		}

		// 位置関係が分かるように地面にラインを描画する
		{
			VECTOR pos1;
			VECTOR pos2;

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
		}

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
