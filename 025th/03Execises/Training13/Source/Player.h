#pragma once
#include "DxLib.h"
#include "Character.h"

class Camera;
class Input;
class NotPlayer;
class Equipment;

/**
* @class Player
* @brief プレイヤークラス
*/
class Player : public Character {
private:
	static const int EQUIP_NUM = 2;

	NotPlayer* m_notPlyerList;
	Equipment* m_equipmentList[EQUIP_NUM];

public:
	Player();

	void Terminate();
	void Process(Camera* camera, Input* input, int stageModelHandle);
	void Render(); 
	void Equip(int stickModelHandle, int hatModelHandle);

	void SetNotPlayerList(NotPlayer* notPlyerList) { m_notPlyerList = notPlyerList; };
};
