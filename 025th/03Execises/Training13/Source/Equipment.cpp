#include "Equipment.h"
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
* @fn Equipment::Initialize
* @brief 装備の初期化
*/
void Equipment::Initialize(int baseModelHandle, int equipModelHandle, const char *frameName)
{
	// モデルハンドルの作成


}
/**
* @fn Equipment::Terminate
* @brief モデルの削除
*/
void Equipment::Terminate()
{
	// モデルの削除


}

/**
* @fn Equipment::Process
* @brief 装備の処理
*/
void Equipment::Process()
{
	MATRIX frameMatrix;
	int frameIndex;

	// フレーム名からフレーム番号を取得する


	// フレームの現在のワールドでの状態を示す行列を取得する


	// セットするモデルの状態を示す行列をフレームの状態を示す行列と同じにする


}

/**
* @fn Equipment::Render
* @brief 装備の描画
*/
void Equipment::Render()
{
	// モデルの描画


}
