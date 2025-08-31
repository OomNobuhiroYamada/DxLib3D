#include "Stage.h"
#include "DxLib.h"
/**
* @file
* @brief Training12
* @author N.Yamada
* @date 2023/01/09
*
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn Stage::Initialize
* @brief ステージの初期化処理
*/
void Stage::Initialize()
{
	// ステージモデルの読み込み
	m_modelHandle = MV1LoadModel("Resource/ColTestStage.mqo");

	// モデル全体のコリジョン情報のセットアップ
	MV1SetupCollInfo(m_modelHandle, -1);
}

/**
* @fn Stage::Terminate
* @brief ステージの後始末処理
*/
void Stage::Terminate()
{
	// ステージモデルの後始末
	MV1DeleteModel(m_modelHandle);
}

/**
* @fn Stage::Render
* @brief ステージの描画
*/
void Stage::Render()
{
	MV1DrawModel(m_modelHandle);
}
