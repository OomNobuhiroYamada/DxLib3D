#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Lesson30
* @author N.Yamada
* @date 2023/01/03
*
* @details キャラクターをアニメーションさせよう
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const float	MOVESPEED = 10.0f;			//!< 移動速度
const float	JUMP_POWER = 30.0f;			//!< ジャンプ力
const float GRAVITY = 2.0f;				//!< 重力
const float	LINE_AREA_SIZE = 10000.0f;	//!< ラインを描く範囲
const int	LINE_NUM = 50;				//!< ラインの数

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

	// 3Dモデルの読み込み
	int modelHandle = MV1LoadModel("Resource/Hero.x");

	// 3Dモデルのアニメーション番号を読み込む
	int animationIndex[Animation::Length];			// アニメーション番号
	for(int i=0; i<Animation::Length; i++)
	{


	}

	// 現在のアニメーション番号
	Animation animationNowIndex = Animation::Neutral;
	Animation animationOldIndex = animationNowIndex;

	// 待機アニメーションをアタッチする


	// 待機アニメーションの総時間を取得しておく


	// アニメーション再生時間を初期化


	// アニメーションで移動をしているフレームの番号を検索する
	int moveAnimFrameIndex = MV1SearchFrame(modelHandle, "root");

	// アニメーションで移動をしているフレームを無効にする
	MV1SetFrameUserLocalMatrix(modelHandle, moveAnimFrameIndex, MV1GetFrameLocalMatrix(modelHandle, moveAnimFrameIndex));

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

	// カメラの位置と向きを設定
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 300.0f, -500.0f), VGet(0.0f, 100.0f, 0.0f));

	// カメラのクリッピング距離を設定
	SetCameraNearFar(16.0f, 5000.0f);

	// 背景の色を灰色にする
	SetBackgroundColor(128, 128, 128);

	// メインループ(何かキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面のクリア
		ClearDrawScreen();

		animationNowIndex = Animation::Neutral;

		// 押されたキーから移動するベクトルを作る
		VECTOR moveVector = VGet(0.0f, 0.0f, 0.0f);
		if(CheckHitKey(KEY_INPUT_A) != 0)
		{
			moveVector.x = -1.0f;
		}
		if(CheckHitKey(KEY_INPUT_D) != 0)
		{
			moveVector.x = 1.0f;
		}
		if(CheckHitKey(KEY_INPUT_W) != 0)
		{
			moveVector.z = 1.0f;
		}
		if(CheckHitKey(KEY_INPUT_S) != 0)
		{
			moveVector.z = -1.0f;
		}

		// 方向入力に従ってキャラクターを移動
		if(moveVector.x != 0.0f || moveVector.z != 0.0f)
		{
			// 走るアニメーション
			animationNowIndex = Animation::Run;

			// カメラの角度に合わせて移動ベクトルを回転してから加算
			angle = atan2f(-moveVector.x, -moveVector.z);
			position.x -= sinf(angle) * MOVESPEED;
			position.z -= cosf(angle) * MOVESPEED;
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
		MV1SetRotationXYZ(modelHandle, VGet(0.0f, angle, 0.0f));

		// 3Dモデルに新しい座標をセット
		MV1SetPosition(modelHandle, position);

		// 3Dモデルの描画
		MV1DrawModel(modelHandle);

		// 位置関係が分かるように地面にラインを描画する
		{
			VECTOR pos1;
			VECTOR pos2;

			SetUseZBufferFlag(true);

			pos1 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			pos2 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, LINE_AREA_SIZE / 2.0f);
			for(int i = 0; i <= LINE_NUM; i++)
			{
				DrawLine3D(pos1, pos2, GetColor(255, 255, 255));
				pos1.x += LINE_AREA_SIZE / LINE_NUM;
				pos2.x += LINE_AREA_SIZE / LINE_NUM;
			}

			pos1 = VGet(-LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			pos2 = VGet(LINE_AREA_SIZE / 2.0f, 0.0f, -LINE_AREA_SIZE / 2.0f);
			for(int i = 0; i < LINE_NUM; i++)
			{
				DrawLine3D(pos1, pos2, GetColor(255, 255, 255));
				pos1.z += LINE_AREA_SIZE / LINE_NUM;
				pos2.z += LINE_AREA_SIZE / LINE_NUM;
			}

			SetUseZBufferFlag(false);
		}

		// 裏画面の内容を表画面に反映
		ScreenFlip();

		// １７ミリ秒(約秒間６０フレームだった時の１フレームあたりの経過時間)
		// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
		}
	}

	// 3Dモデル削除
	MV1DeleteModel(modelHandle);

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
