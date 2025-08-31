#pragma once
#include "DxLib.h"

#define CHARA_PLAY_ANIM_SPEED		250.0f		// アニメーション速度
#define CHARA_MOVE_SPEED			30.0f		// 移動速度
#define CHARA_ANIM_BLEND_SPEED		0.1f		// アニメーションのブレンド率変化速度
#define CHARA_ANGLE_SPEED			0.2f		// 角度変化速度
#define CHARA_JUMP_POWER			100.0f		// ジャンプ力
#define CHARA_FALL_UP_POWER			20.0f		// 足を踏み外した時のジャンプ力
#define CHARA_GRAVITY				3.0f		// 重力
#define CHARA_MAX_HITCOLL			2048		// 処理するコリジョンポリゴンの最大数
#define CHARA_ENUM_DEFAULT_SIZE		800.0f		// 周囲のポリゴン検出に使用する球の初期サイズ
#define CHARA_HIT_WIDTH				200.0f		// 当たり判定カプセルの半径
#define CHARA_HIT_HEIGHT			700.0f		// 当たり判定カプセルの高さ
#define CHARA_HIT_TRYNUM			16			// 壁押し出し処理の最大試行回数
#define CHARA_HIT_SLIDE_LENGTH		5.0f		// 一度の壁押し出し処理でスライドさせる距離
#define CHARA_SHADOW_SIZE			200.0f		// 影の大きさ
#define CHARA_SHADOW_HEIGHT			700.0f		// 影が落ちる高さ

enum AnimeState
{
	Run = 1,
	Jump = 2,
	Neutral = 4,
};

// プレイヤー情報構造体
struct PLAYER
{
	VECTOR		position;				// 座標
	VECTOR		targetMoveDirection;	// モデルが向くべき方向のベクトル
	float		angle;					// モデルが向いている方向の角度
	float		jumpPower;				// Ｙ軸方向の速度
	int			modelHandle;			// モデルハンドル
	int			shadowHandle;			// 影画像ハンドル
	AnimeState	state;					// 状態

	int			playAnim1;				// 再生しているアニメーション１のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		animPlayCount1;			// 再生しているアニメーション１の再生時間
	int			playAnim2;				// 再生しているアニメーション２のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		animPlayCount2;			// 再生しているアニメーション２の再生時間
	float		animBlendRate;			// 再生しているアニメーション１と２のブレンド率

	int			stickModelHandle;		// 棒のモデルハンドル
	int			hatModelHandle;			// 帽子のモデルハンドル
};

void Player_Initialize(void);		// プレイヤーの初期化
void Player_Terminate(void);		// プレイヤーの後始末
void Player_Process(void);			// プレイヤーの処理
void Player_Move(VECTOR moveVector);// プレイヤーの移動処理
void Player_AngleProcess(void);		// プレイヤーの向きを変える処理
void Player_PlayAnim(int playAnim);	// プレイヤーに新たなアニメーションを再生する
void Player_AnimProcess(void);		// プレイヤーのアニメーション処理
void Player_ShadowRender(void);		// プレイヤーの影を描画
