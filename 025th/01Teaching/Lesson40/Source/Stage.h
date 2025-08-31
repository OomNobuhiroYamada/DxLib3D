#pragma once

// ステージ情報構造体
struct STAGE
{
	int		modelHandle;			// モデルハンドル
};

void Stage_Initialize(void);		// ステージの初期化処理
void Stage_Terminate(void);			// ステージの後始末処理
