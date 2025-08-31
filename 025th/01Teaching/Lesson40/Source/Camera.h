#pragma once
#include "DxLib.h"

#define CAMERA_ANGLE_SPEED			0.05f		// 旋回速度
#define CAMERA_PLAYER_TARGET_HEIGHT	400.0f		// プレイヤー座標からどれだけ高い位置を注視点とするか
#define CAMERA_PLAYER_LENGTH		1600.0f		// プレイヤーとの距離
#define CAMERA_COLLISION_SIZE		50.0f		// カメラの当たり判定サイズ

// カメラ情報構造体
struct CAMERA
{
	float		angleH;				// 水平角度
	float		angleV;				// 垂直角度
	VECTOR		eye;				// カメラ座標
	VECTOR		target;				// 注視点座標
};

void Camera_Initialize(void);		// カメラの初期化処理
void Camera_Process(void);			// カメラの処理
