#pragma once

/**
* @struct STAGE
* @brief ステージ情報構造体
*/
struct STAGE
{
	int		modelHandle;		//!< モデルハンドル
};

void Stage_Initialize();		//!< ステージの初期化処理
void Stage_Terminate();			//!< ステージの後始末処理
