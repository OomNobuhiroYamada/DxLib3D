#include "Player.h"
#include "Camera.h"
#include "Input.h"
#include "NotPlayer.h"
#include "Equipment.h"
#include "Literal.h"
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
* @fn Player::Player
* @brief プレイヤー
*/
Player::Player()
{
	for(int i=0; i<EQUIP_NUM; i++)
	{
		m_equipmentList[i] = nullptr;
	}
}

/**
* @fn Player::Terminate
* @brief キャラクターの後始末
*/
void Player::Terminate()
{
	// モデルの削除
	Character::Terminate();

	// 装備品の削除
	for(int i=0; i<EQUIP_NUM; i++)
	{
		if(m_equipmentList[i] != nullptr)
		{
			m_equipmentList[i]->Terminate();
			delete m_equipmentList[i];
			m_equipmentList[i] = nullptr;
		}
	}
}

/**
* @fn Player::Process
* @brief プレイヤーの処理
*/
void Player::Process(Camera* camera, Input* input, int stageModelHandle)
{
	VECTOR upMoveVec;	// 方向ボタン「↑」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR leftMoveVec;	// 方向ボタン「←」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR moveVec;		// このフレームの移動ベクトル
	bool jumpFlag;		// ジャンプフラグ

	// プレイヤーの移動方向のベクトルを算出
	{
		// 方向ボタン「↑」を押したときのプレイヤーの移動ベクトルはカメラの視線方向からＹ成分を抜いたもの
		upMoveVec = VSub(camera->GetTarget(), camera->GetEye());
		upMoveVec.y = 0.0f;

		// 方向ボタン「←」を押したときのプレイヤーの移動ベクトルは上を押したときの方向ベクトルとＹ軸のプラス方向のベクトルに垂直な方向
		leftMoveVec = VCross(upMoveVec, VGet(0.0f, 1.0f, 0.0f));

		// 二つのベクトルを正規化( ベクトルの長さを１．０にすること )
		upMoveVec = VNorm(upMoveVec);
		leftMoveVec = VNorm(leftMoveVec);
	}

	// このフレームでの移動ベクトルを初期化
	moveVec = VGet(0.0f, 0.0f, 0.0f);

	// ジャンプフラグを倒す
	jumpFlag = false;

	// パッドの３ボタンと左シフトがどちらも押されていなかったらプレイヤーの移動処理
	if (CheckHitKey(KEY_INPUT_LSHIFT) == 0 && (input->GetNowInput() & PAD_INPUT_C) == 0)
	{
		// 方向ボタン「←」が入力されたらカメラの見ている方向から見て左方向に移動する
		if (input->GetNowInput() & PAD_INPUT_LEFT)
		{
			// 移動ベクトルに「←」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, leftMoveVec);
		}
		else
		{
			// 方向ボタン「→」が入力されたらカメラの見ている方向から見て右方向に移動する
			if (input->GetNowInput() & PAD_INPUT_RIGHT)
			{
				// 移動ベクトルに「←」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(leftMoveVec, -1.0f));
			}
		}

		// 方向ボタン「↑」が入力されたらカメラの見ている方向に移動する
		if (input->GetNowInput() & PAD_INPUT_UP)
		{
			// 移動ベクトルに「↑」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, upMoveVec);
		}
		else
		{
			// 方向ボタン「↓」が入力されたらカメラの方向に移動する
			if (input->GetNowInput() & PAD_INPUT_DOWN)
			{
				// 移動ベクトルに「↑」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(upMoveVec, -1.0f));
			}
		}

		// ボタン１が押されていたらジャンプフラグを立てる
		if(input->GetEdgeInput() & PAD_INPUT_A)
		{
			jumpFlag = true;
		}
	}

	// 移動方向を移動速度でスケーリングする
	moveVec = VScale(moveVec, MOVE_SPEED);

	// プレイヤーキャラ以外との当たり判定を行う
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		Collision(&moveVec, &m_notPlyerList[i]);
	}

	// キャラクターを動作させる処理を行う
	_Process(moveVec, jumpFlag, stageModelHandle);

	// 装備品の処理
	for(int i=0; i<EQUIP_NUM; i++)
	{
		if(m_equipmentList[i] != nullptr)
		{
			m_equipmentList[i]->Process();
		}
	}
}

/**
* @fn Player::Render
* @brief プレイヤーの描画
*/
void Player::Render()
{
	// モデルの描画
	Character::Render();

	// 装備品の削除
	for(int i=0; i<EQUIP_NUM; i++)
	{
		if(m_equipmentList[i] != nullptr)
		{
			m_equipmentList[i]->Render();
		}
	}
}

/**
* @fn Player::Equip
* @brief プレイヤーの装備
*/
void Player::Equip(int stickModelHandle, int hatModelHandle)
{
	if(m_equipmentList[0] == nullptr)
	{
		m_equipmentList[0] = new Equipment();
		m_equipmentList[0]->Initialize(stickModelHandle, m_modelHandle, "FingerR");
	}
	if(m_equipmentList[1] == nullptr)
	{
		m_equipmentList[1] = new Equipment();
		m_equipmentList[1]->Initialize(hatModelHandle, m_modelHandle, "Head");
	}
}
