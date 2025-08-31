#pragma once
#include "DxLib.h"
#include "Character.h"

class Camera;
class Input;
class NotPlayer;

/**
* @class NotPlayer
*/
class Player : public Character
{
private:
	NotPlayer* m_notPlyerList;

public:
	void Process(Camera* camera, Input* input, int stageModelHandle);

	void SetNotPlayerList(NotPlayer* notPlyerList) { m_notPlyerList = notPlyerList; };
};
