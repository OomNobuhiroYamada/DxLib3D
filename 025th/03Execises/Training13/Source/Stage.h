#pragma once

/**
* @class Stage
* @brief ステージクラス
*/
class Stage {
private:
	int m_modelHandle;	// モデルハンドル

public:
	void Initialize();	// 初期化処理
	void Terminate();	// 後始末処理
	void Render();

	int GetModelHandle() { return m_modelHandle; }
};
