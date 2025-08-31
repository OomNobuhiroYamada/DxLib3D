#include "Input.h"
#include "DxLib.h"
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
* @fn Input::Process
* @brief 入力の処理
*/

void Input::Process()
{
	int old;

	// ひとつ前のフレームの入力を変数にとっておく
	old = m_nowInput;

	// 現在の入力状態を取得
	m_nowInput = GetJoypadInputState(DX_INPUT_KEY_PAD1);

	// 今のフレームで新たに押されたボタンのビットだけ立っている値を edgeInput に代入する
	m_edgeInput = m_nowInput & ~old;
}
