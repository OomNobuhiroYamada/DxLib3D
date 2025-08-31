#pragma once

/**
* @struct PADINPUT
* @brief 入力情報構造体
*/
struct PADINPUT
{
	int		nowInput;			//!< 現在の入力
	int		edgeInput;			//!< 現在のフレームで押されたボタンのみビットが立っている入力値
};

void Input_Process();			//!< 入力処理
