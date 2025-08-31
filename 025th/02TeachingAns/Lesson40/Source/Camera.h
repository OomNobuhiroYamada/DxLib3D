#pragma once
#include "DxLib.h"

const float CAMERA_ANGLE_SPEED = 0.05f;				//!< プレイヤー座標からどれだけ高い位置を注視点とするか
const float CAMERA_PLAYER_TARGET_HEIGHT = 400.0f;	//!< プレイヤー座標からどれだけ高い位置を注視点とするか
const float CAMERA_PLAYER_LENGTH = 1600.0f;			//!< プレイヤーとの距離
const float CAMERA_COLLISION_SIZE = 50.0f;			//!< カメラの当たり判定サイズ

/**
* @struct CAMERA
* @brief カメラ情報構造体
*/
struct CAMERA
{
	float		angleH;			//!< 水平角度
	float		angleV;			//!< 垂直角度
	VECTOR		eye;			//!< カメラ座標
	VECTOR		target;			//!< 注視点座標
};

void Camera_Initialize();		//!< カメラの初期化処理
void Camera_Process();			//!< カメラの処理
