#include "CheckKey.h"
#include "DxLib.h"
/**
* @file
* @brief Mission04
* @author N.Yamada
* @date 2023/01/06
*
* @details キー入力に関する補助関数
* @details DXLibに存在しないので自作
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @namespace CheckKey
* @brief Key用のキーバッファ
*/
namespace CheckKey
{
	int downBuffer[256];	// CheckDownKey用のキーバッファ
	int upBuffer[256];		// CheckUpKey用のキーバッファ
};

/**
* @fn CheckDownKey
* @brief Main関数
* @param[in] int KeyCode
* @return int 0 キーは押されていない／1 キーを押した瞬間
* @details 指定されたキーが押された瞬間だけ 1 を返す関数
*/
int CheckDownKey(int KeyCode)
{
	// 戻り値用の変数を用意
	int result = 0;

	// 指定キーの現在の状態を取得
	int keyState = CheckHitKey(KeyCode);

	// 前回キーが押されておらず、かつ、現在キーが押されていたら「キーを押した瞬間」とする
	if(CheckKey::downBuffer[KeyCode] == 0 && keyState == 1)
	{
		result = 1;
	}

	// 現在のキー状態をバッファに格納
	CheckKey::downBuffer[KeyCode] = keyState;

	return result;
}

// 指定されたキーが離された瞬間だけ 1 を返す関数
/**
* @fn CheckUpKey
* @brief Main関数
* @param[in] int KeyCode
* @return int 0 キーは離されていない／1 キーを離した瞬間
* @details 指定されたキーが離された瞬間だけ 1 を返す関数
*/
int CheckUpKey(int KeyCode)
{
	// 戻り値用の変数を用意
	int result = 0;

	// 指定キーの現在の状態を取得
	int keyState = CheckHitKey(KeyCode);

	// 前回キーが押されており、かつ、現在キーが押されていなかったら「キーを離した瞬間」とする
	if(CheckKey::upBuffer[KeyCode] == 1 && keyState == 0)
	{
		result = 1;
	}

	// 現在のキー状態をバッファに格納
	CheckKey::upBuffer[KeyCode] = keyState;

	return result;
}
