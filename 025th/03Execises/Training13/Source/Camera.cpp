#include "Camera.h"
/**
* @file
* @brief Training13
* @author N.Yamada
* @date 2023/01/15
*
* @details クラス化その２
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn Camera::Initialize
* @brief カメラの初期化処理
*/
void Camera::Initialize()
{
	// カメラの初期水平角度は１８０度
	m_angleH = DX_PI_F;

	// 垂直角度は０度
	m_angleV = 0.0f;
}

/**
* @fn Camera::Process
* @brief カメラの処理
*/
void Camera::Process(int nowInput, VECTOR& playerPosition, int stageModelHandle)
{
	// パッドの３ボタンか、シフトキーが押されている場合のみ角度変更操作を行う
	if(CheckHitKey(KEY_INPUT_LSHIFT) || (nowInput & PAD_INPUT_C))
	{
		// 「←」ボタンが押されていたら水平角度をマイナスする
		if(nowInput & PAD_INPUT_LEFT)
		{
			m_angleH -= ANGLE_SPEED;

			// －１８０度以下になったら角度値が大きくなりすぎないように３６０度を足す
			if(m_angleH < -DX_PI_F)
			{
				m_angleH += DX_TWO_PI_F;
			}
		}

		// 「→」ボタンが押されていたら水平角度をプラスする
		if(nowInput & PAD_INPUT_RIGHT)
		{
			m_angleH += ANGLE_SPEED;

			// １８０度以上になったら角度値が大きくなりすぎないように３６０度を引く
			if(m_angleH > DX_PI_F)
			{
				m_angleH -= DX_TWO_PI_F;
			}
		}

		// 「↑」ボタンが押されていたら垂直角度をマイナスする
		if(nowInput & PAD_INPUT_UP)
		{
			m_angleV -= ANGLE_SPEED;

			// ある一定角度以下にはならないようにする
			if(m_angleV < -DX_PI_F / 2.0f + 0.6f)
			{
				m_angleV = -DX_PI_F / 2.0f + 0.6f;
			}
		}

		// 「↓」ボタンが押されていたら垂直角度をプラスする
		if(nowInput & PAD_INPUT_DOWN)
		{
			m_angleV += ANGLE_SPEED;

			// ある一定角度以上にはならないようにする
			if(m_angleV > DX_PI_F / 2.0f - 0.6f)
			{
				m_angleV = DX_PI_F / 2.0f - 0.6f;
			}
		}
	}

	// カメラの注視点はプレイヤー座標から規定値分高い座標
	m_target = VAdd(playerPosition, VGet(0.0f, PLAYER_TARGET_HEIGHT, 0.0f));

	// カメラの座標を決定する
	{
		MATRIX rotZ, rotY;
		float cameraPlayerLength;
		MV1_COLL_RESULT_POLY_DIM hRes;
		int hitNum;

		// 水平方向の回転はＹ軸回転
		rotY = MGetRotY(m_angleH);

		// 垂直方向の回転はＺ軸回転 )
		rotZ = MGetRotZ(m_angleV);

		// カメラからプレイヤーまでの初期距離をセット
		cameraPlayerLength = PLAYER_LENGTH;

		// カメラの座標を算出
		// Ｘ軸にカメラとプレイヤーとの距離分だけ伸びたベクトルを
		// 垂直方向回転( Ｚ軸回転 )させたあと水平方向回転( Ｙ軸回転 )して更に
		// 注視点の座標を足したものがカメラの座標
		m_eye = VAdd(VTransform(VTransform(VGet(-cameraPlayerLength, 0.0f, 0.0f), rotZ), rotY), m_target);

		// 注視点からカメラの座標までの間にステージのポリゴンがあるか調べる
		hRes = MV1CollCheck_Capsule(stageModelHandle, -1, m_target, m_eye, COLLISION_SIZE);
		hitNum = hRes.HitNum;
		MV1CollResultPolyDimTerminate(hRes);
		if(hitNum != 0)
		{
			float notHitLength;
			float hitLength;
			float testLength;
			VECTOR testPosition;

			// あったら無い位置までプレイヤーに近づく

			// ポリゴンに当たらない距離をセット
			notHitLength = 0.0f;

			// ポリゴンに当たる距離をセット
			hitLength = cameraPlayerLength;
			do
			{
				// 当たるかどうかテストする距離をセット( 当たらない距離と当たる距離の中間 )
				testLength = notHitLength + (hitLength - notHitLength) / 2.0f;

				// テスト用のカメラ座標を算出
				testPosition = VAdd(VTransform(VTransform(VGet(-testLength, 0.0f, 0.0f), rotZ), rotY), m_target);

				// 新しい座標で壁に当たるかテスト
				hRes = MV1CollCheck_Capsule(stageModelHandle, -1, m_target, testPosition, COLLISION_SIZE);
				hitNum = hRes.HitNum;
				MV1CollResultPolyDimTerminate(hRes);
				if(hitNum != 0)
				{
					// 当たったら当たる距離を testLength に変更する
					hitLength = testLength;
				}
				else
				{
					// 当たらなかったら当たらない距離を testLength に変更する
					notHitLength = testLength;
				}

				// hitLength と NoHitLength が十分に近づいていなかったらループ
			} while (hitLength - notHitLength > 0.1f);

			// カメラの座標をセット
			m_eye = testPosition;
		}
	}

	// カメラの情報をライブラリのカメラに反映させる
	SetCameraPositionAndTarget_UpVecY(m_eye, m_target);
}
