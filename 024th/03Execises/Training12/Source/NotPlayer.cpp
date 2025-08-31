#include "NotPlayer.h"
#include "Literal.h"
#include "DxLib.h"
#include <math.h>
/**
* @file
* @brief Training12
* @author N.Yamada
* @date 2023/01/09
*
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn NotPlayer::NotPlayer
*/
NotPlayer::NotPlayer()
{
	m_moveTime = 0;
	m_moveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;
}

/**
* @fn NotPlayer::Process
* @brief プレイヤー以外キャラの処理
*/
void NotPlayer::Process(int stageModelHandle)
{
	VECTOR moveVec;
	bool jumpFlag;

	// ジャンプフラグを倒しておく
	jumpFlag = false;

	// 一定時間が経過したら移動する方向を変更する
	m_moveTime++;
	if (m_moveTime >= MOVETIME)
	{
		m_moveTime = 0;

		// 新しい方向の決定
		m_moveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;

		// 一定確率でジャンプする
		if (GetRand(1000) < JUMPRATIO)
		{
			jumpFlag = true;
		}
	}

	// 新しい方向の角度からベクトルを算出
	moveVec.x = cosf(m_moveAngle) * MOVE_SPEED;
	moveVec.y = 0.0f;
	moveVec.z = sinf(m_moveAngle) * MOVE_SPEED;

	// プレイヤーとの当たり判定を行う
	Collision(&moveVec, m_player);

	// 自分以外のプレイヤーキャラとの当たり判定を行う
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		// 自分との当たり判定はしない
		if(this == &m_notPlyerList[i])
		{
			continue;
		}

		Collision(&moveVec, &m_notPlyerList[i]);
	}

	// 移動処理を行う
	_Process(moveVec, jumpFlag, stageModelHandle);
}
