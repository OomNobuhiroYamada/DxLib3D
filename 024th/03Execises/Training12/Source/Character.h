#pragma once
#include "DxLib.h"

/**
* @class Character
* @brief キャラクタークラス
*/
class Character {
protected:
	const float PLAY_ANIM_SPEED = 250.0f;	//!< アニメーション速度
	const float MOVE_SPEED = 30.0f;			//!< 移動速度
	const float ANIM_BLEND_SPEED = 0.1f;	//!< アニメーションのブレンド率変化速度
	const float ANGLE_SPEED = 0.2f;			//!< 角度変化速度
	const float JUMP_POWER = 100.0f;		//!< ジャンプ力
	const float FALL_UP_POWER = 20.0f;		//!< 足を踏み外した時のジャンプ力
	const float GRAVITY = 3.0f;				//!< 重力
	static const int MAX_HITCOLL = 2048;	//!< 処理するコリジョンポリゴンの最大数
	const float ENUM_DEFAULT_SIZE = 800.0f;	//!< 周囲のポリゴン検出に使用する球の初期サイズ
	const float HIT_WIDTH = 200.0f;			//!< 当たり判定カプセルの半径
	const float HIT_HEIGHT = 700.0f;		//!< 当たり判定カプセルの高さ
	const int HIT_TRYNUM = 16;				//!< 壁押し出し処理の最大試行回数
	const float HIT_SLIDE_LENGTH = 5.0f;	//!< 一度の壁押し出し処理でスライドさせる距離
	const float HIT_PUSH_POWER = 40.0f;		//!< キャラクター同士で当たったときの押し出される力
	const float SHADOW_SIZE = 200.0f;		//!< 影の大きさ
	const float SHADOW_HEIGHT = 700.0f;		//!< 影が落ちる高さ

	enum AnimeState
	{
		Run = 1,
		Jump = 2,
		Neutral = 4,
	};

	VECTOR m_position;						//!< 座標
	VECTOR m_targetMoveDirection;			//!< モデルが向くべき方向のベクトル
	float m_angle;							//!< モデルが向いている方向の角度
	float m_jumpPower;						//!< Ｙ軸方向の速度
	int m_modelHandle;						//!< モデルハンドル
	int m_shadowHandle;						//!< 影の画像ハンドル
	AnimeState m_state;						//!< 状態
	int m_playAnim1;						//!< 再生しているアニメーション１のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float m_animPlayCount1;					//!< 再生しているアニメーション１の再生時間
	int m_playAnim2;						//!< 再生しているアニメーション２のアタッチ番号( -1:何もアニメーションがアタッチされていない )
	float m_animPlayCount2;					//!< 再生しているアニメーション２の再生時間
	float m_animBlendRate;					//!< 再生しているアニメーション１と２のブレンド率

	void Move(VECTOR moveVector, int stageModelHandle);		//!< キャラクターの移動処理
	void Collision(VECTOR *chMoveVec, Character *chkCh);	//!< キャラクターに当たっていたら押し出す処理を行う( chkCh に ch が当たっていたら ch が離れる )
	void AngleProcess();					//!< キャラクターの向きを変える処理
	void PlayAnim();						//!< キャラクターに新たなアニメーションを再生する
	void AnimProcess();						//!< キャラクターのアニメーション処理

public:
	void Initialize(int baseModelHandle, int shadowHandle, VECTOR position);	//!< キャラクターの初期化
	void Terminate();							//!< キャラクターの後始末
	void _Process(VECTOR moveVec, bool jumpFlag, int stageModelHandle);			//!< キャラクターの処理
	void ShadowRender(int stageModelHandle);	//!< キャラクターの影を描画
	void Render();

	VECTOR& GetPosition() { return m_position; }
};

/**
* @struct CHARA_COMMON
* @brief キャラクター共通情報
*/
struct CHARA_COMMON
{
	int		baseModelHandle;		//!< 共通の派生元モデルハンドル
	int		shadowHandle;			//!< 影描画用のグラフィックハンドル
};
