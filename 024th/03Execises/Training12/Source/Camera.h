#pragma once
#include "DxLib.h"

/**
* @class Camera
* @brief カメラクラス
*/
class Camera {
private:
	const float ANGLE_SPEED = 0.05f;			//!< プレイヤー座標からどれだけ高い位置を注視点とするか
	const float PLAYER_TARGET_HEIGHT = 400.0f;	//!< プレイヤー座標からどれだけ高い位置を注視点とするか
	const float PLAYER_LENGTH = 1600.0f;		//!< プレイヤーとの距離
	const float COLLISION_SIZE = 50.0f;			//!< カメラの当たり判定サイズ

	float m_angleH;								//!< 水平角度
	float m_angleV;								//!< 垂直角度
	VECTOR m_eye;								//!< カメラ座標
	VECTOR m_target;							//!< 注視点座標

public:
	void Initialize();							//!< カメラの初期化処理
	void Process(int nowInput, VECTOR& playerPosition, int stageModelHandle);		//!< カメラの処理
	
	VECTOR& GetEye() { return m_eye; }
	VECTOR& GetTarget() { return m_target; }
};
