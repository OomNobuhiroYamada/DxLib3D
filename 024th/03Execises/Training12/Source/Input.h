#pragma once

/**
* @class Input
* @brief 入力クラス
*/
class Input
{
private:
	int m_nowInput;			//!< 現在の入力
	int m_edgeInput;		//!< 現在のフレームで押されたボタンのみビットが立っている入力値

public:
	void Process();			//!< 入力処理

	int GetNowInput() { return m_nowInput; }
	int GetEdgeInput() { return m_edgeInput; }
};
