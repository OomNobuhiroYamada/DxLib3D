#pragma once

/**
* @class Equipment
* @brief 装備クラス
*/
class Equipment {
private:
	int m_modelHandle;
	int m_equipModelHandle;
	const char* m_frameName;

public:
	void Initialize(int baseModelHandle, int equipModelHandle, const char *frameName);
	void Terminate();
	void Process();
	void Render();
};
