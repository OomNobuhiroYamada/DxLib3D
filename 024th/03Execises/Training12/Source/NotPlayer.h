#pragma once
#include "Character.h"

/**
* @class NotPlayer
*/
class NotPlayer : public Character
{
private:
	const int MOVETIME = 120;		// 一つの方向に移動する時間
	const int JUMPRATIO = 250;		// プレイヤー以外のキャラがジャンプする確率

	int m_moveTime;					// 移動時間
	float m_moveAngle;				// 移動方向
	Character* m_player;
	NotPlayer* m_notPlyerList;

public:
	NotPlayer();

	void Process(int stageModelHandle);

	void SetPlayer(Character* player) { m_player = player; };
	void SetNotPlayerList(NotPlayer* notPlyerList) { m_notPlyerList = notPlyerList; };
};

