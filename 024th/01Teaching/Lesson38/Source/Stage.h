#pragma once

const int STAGECOLLOBJ_MAXNUM = 256;					//!< 追加のコリジョンオブジェクトの最大数

/**
* @struct STAGE
* @brief ステージ情報構造体
*/
struct STAGE
{
	int		modelHandle;								//!< ステージのモデルハンドル
	int		collObjBaseModelHandle;						//!< コリジョンモデルの派生元ハンドル
	int		collObjModelHandle[STAGECOLLOBJ_MAXNUM];	//!< ステージに配置するコリジョンモデルのハンドル
	int		collObjNum;									//!< ステージに配置しているコリジョンモデルの数
};

void Stage_Initialize();								//!< ステージの初期化処理
void Stage_Terminate();									//!< ステージの後始末処理
