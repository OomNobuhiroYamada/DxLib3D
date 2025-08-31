#include "DxLib.h"
#include "Input.h"
#include "Character.h"
#include "Player.h"
#include "NotPlayer.h"
#include "Stage.h"
#include "Camera.h"
#include "Literal.h"
/**
* @file
* @brief Training12
* @author N.Yamada
* @date 2023/01/09
*
* @details クラス化
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

	// ライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}
	
	CHARA_COMMON charaCommon;	// キャラクターの共通情報の実体宣言

	// モデルの読み込み
	charaCommon.baseModelHandle = MV1LoadModel("Resource/DxChara.x");

	// 影描画用の画像の読み込み
	charaCommon.shadowHandle = LoadGraph("Resource/Shadow.tga");

	// 入力の宣言
	Input input;

	// プレイヤーの初期化
	Player player;
	player.Initialize(charaCommon.baseModelHandle, charaCommon.shadowHandle, VGet(0.0f, 0.0f, 0.0f));

	// NPCの初期位置
	static VECTOR firstPosition[NOTPLAYER_NUM] =
	{
		{ -3000.0f, 0.0f, 2300.0f },
		{ -2500.0f, 7300.0f, -2500.0f },
		{ -2600.0f, 0.0f, -3100.0f },
		{  2800.0f, 0.0f, 200.0f },
	};

	// プレイヤー以外キャラの初期化
	NotPlayer npc[NOTPLAYER_NUM];
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		npc[i].Initialize(charaCommon.baseModelHandle, charaCommon.shadowHandle, firstPosition[i]);

		npc[i].SetPlayer(&player);
		npc[i].SetNotPlayerList(npc);
	}
	
	player.SetNotPlayerList(npc);

	// ステージの初期化
	Stage stage;
	stage.Initialize();

	// カメラの初期化
	Camera camera;
	camera.Initialize();

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// ＥＳＣキーが押されるか、ウインドウが閉じられるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面をクリア
		ClearDrawScreen();

		// 入力処理
		input.Process();

		// プレイヤー以外キャラの処理
		for(int i=0; i<NOTPLAYER_NUM; i++)
		{
			npc[i].Process(stage.GetModelHandle());
		}

		// プレイヤーの処理
		player.Process(&camera, &input, stage.GetModelHandle());

		// カメラの処理
		VECTOR ppos = VGet(0.0f, 0.0f, 0.0f);
		camera.Process(input.GetNowInput(), player.GetPosition(), stage.GetModelHandle());

		// 描画処理
		{
			// ステージモデルの描画
			stage.Render();

			// プレイヤーモデルの描画
			player.Render();

			// プレイヤー以外キャラモデルの描画
			for(int i=0; i<NOTPLAYER_NUM; i++)
			{
				npc[i].Render();
			}

			// プレイヤーの影の描画
			player.ShadowRender(stage.GetModelHandle());

			// プレイヤー以外キャラの影の描画
			for(int i=0; i<NOTPLAYER_NUM; i++)
			{
				npc[i].ShadowRender(stage.GetModelHandle());
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

	// プレイヤー以外キャラの後始末
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		// キャラクター情報の後始末
		npc[i].Terminate();
	}

	// プレイヤーの後始末
	player.Terminate();
	
	// モデルの削除
	MV1DeleteModel(charaCommon.baseModelHandle);

	// 影用画像の削除
	DeleteGraph(charaCommon.shadowHandle);

	// ステージの後始末
	stage.Terminate();

	// ライブラリの後始末
	DxLib_End();

	// ソフト終了
	return 0;
}
