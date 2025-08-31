#pragma once
#include "DxLib.h"

// キャラクター共通の情報
const float CHARA_PLAY_ANIM_SPEED = 250.0f;		//!< アニメーション速度
const float CHARA_MOVE_SPEED = 30.0f;			//!< 移動速度
const float CHARA_ANIM_BLEND_SPEED = 0.1f;		//!< アニメーションのブレンド率変化速度
const float CHARA_ANGLE_SPEED = 0.2f;			//!< 角度変化速度
const float CHARA_JUMP_POWER = 100.0f;			//!< ジャンプ力
const float CHARA_FALL_UP_POWER = 20.0f;		//!< 足を踏み外した時のジャンプ力
const float CHARA_GRAVITY = 3.0f;				//!< 重力
const int   CHARA_MAX_HITCOLL = 2048;			//!< 処理するコリジョンポリゴンの最大数
const float CHARA_ENUM_DEFAULT_SIZE = 800.0f;	//!< 周囲のポリゴン検出に使用する球の初期サイズ
const float CHARA_HIT_WIDTH = 200.0f;			//!< 当たり判定カプセルの半径
const float CHARA_HIT_HEIGHT = 700.0f;			//!< 当たり判定カプセルの高さ
const int   CHARA_HIT_TRYNUM = 16;				//!< 壁押し出し処理の最大試行回数
const float CHARA_HIT_SLIDE_LENGTH = 5.0f;		//!< 一度の壁押し出し処理でスライドさせる距離
const float CHARA_HIT_PUSH_POWER = 40.0f;		//!< キャラクター同士で当たったときの押し出される力
const float CHARA_SHADOW_SIZE = 200.0f;			//!< 影の大きさ
const float CHARA_SHADOW_HEIGHT = 700.0f;		//!< 影が落ちる高さ

// プレイヤーキャラ以外キャラの情報
const int   NOTPLAYER_NUM = 4;					//!< プレイヤー以外のキャラの数
const int   NOTPLAYER_MOVETIME = 120;			//!< 一つの方向に移動する時間
const int   NOTPLAYER_JUMPRATIO = 250;			//!< プレイヤー以外のキャラがジャンプする確率

/**
* @enum AnimeState
* @brief 当たり判定方法の列挙型
*/
enum AnimeState
{
	Run = 1,
	Jump = 2,
	Neutral = 4,
};

/**
* @struct CHARA
* @brief キャラクター情報
*/
struct CHARA
{
	VECTOR		position;						//!<座標
	VECTOR		targetMoveDirection;			//!< モデルが向くべき方向のベクトル
	float		angle;							//!< モデルが向いている方向の角度
	float		jumpPower;						//!< Ｙ軸方向の速度
	int			modelHandle;					//!< モデルハンドル
	AnimeState	state;							//!< 状態

	int			playAnim1;						//!< 再生しているアニメーション１のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		animPlayCount1;					//!< 再生しているアニメーション１の再生時間
	int			playAnim2;						//!< 再生しているアニメーション２のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float		animPlayCount2;					//!< 再生しているアニメーション２の再生時間
	float		animBlendRate;					//!< 再生しているアニメーション１と２のブレンド率
};

/**
* @struct PLAYER
* @brief プレイヤー情報構造体
*/
struct PLAYER
{
	CHARA	charaInfo;							//!< キャラクター情報
};

/**
* @struct NOTPLAYER
* @brief プレイヤー以外キャラ情報構造体
*/
struct NOTPLAYER
{
	CHARA	charaInfo;							//!< キャラクター情報
	int		moveTime;							//!< 移動時間
	float	moveAngle;							//!< 移動方向
};

/**
* @struct CHARA_COMMON
* @brief キャラクター共通情報
*/
struct CHARA_COMMON
{
	int		baseModelHandle;					//!< 共通の派生元モデルハンドル
	int		shadowHandle;						//!< 影描画用のグラフィックハンドル
};

void Chara_Initialize(CHARA *ch, VECTOR position);					//!< キャラクターの初期化
void Chara_Terminate(CHARA *ch);									//!< キャラクターの後始末
void Chara_Process(CHARA *ch, VECTOR moveVec, bool jumpFlag);		//!< キャラクターの処理
void Chara_Move(CHARA *ch, VECTOR moveVector);						//!< キャラクターの移動処理
void Chara_Collision(CHARA *ch, VECTOR *chMoveVec, CHARA *chkCh);	//!< キャラクターに当たっていたら押し出す処理を行う( chkCh に ch が当たっていたら ch が離れる )
void Chara_AngleProcess(CHARA *ch);									//!< キャラクターの向きを変える処理
void Chara_PlayAnim(CHARA *ch);										//!< キャラクターに新たなアニメーションを再生する
void Chara_AnimProcess(CHARA *ch);									//!< キャラクターのアニメーション処理
void Chara_ShadowRender(CHARA *ch);									//!< キャラクターの影を描画

void Player_Initialize();											//!< プレイヤーの初期化
void Player_Terminate();											//!< プレイヤーの後始末
void Player_Process();												//!< プレイヤーの処理
