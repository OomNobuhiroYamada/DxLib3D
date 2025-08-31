#include "DrawBoard.h"
/**
* @file
* @brief Training11
* @author N.Yamada
* @date 2023/01/06
*
* @details 2D画像を3D空間に配置する関数
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn DrawBoard
* @brief 2D画像を3D空間に配置する関数
* @param[in] VECTOR TopLeft, VECTOR BottomRight, int graphHandle
* @return void
*/
void DrawBoard(VECTOR TopLeft, VECTOR BottomRight, int graphHandle)
{
	VERTEX3D vertex[4];
	WORD index[6];
	// ４頂点分のデータをセット

	// 左上
	vertex[0].pos = TopLeft;
	vertex[0].norm = VGet(0.0f, 0.0f, -1.0f);
	vertex[0].dif = GetColorU8(255, 255, 255, 255);
	vertex[0].spc = GetColorU8(0, 0, 0, 0);
	vertex[0].u = 0.0f;
	vertex[0].v = 0.0f;
	vertex[0].su = 0.0f;
	vertex[0].sv = 0.0f;

	// 右上
	vertex[1].pos = VGet(BottomRight.x, TopLeft.y, BottomRight.z);
	vertex[1].norm = VGet(0.0f, 0.0f, -1.0f);
	vertex[1].dif = GetColorU8(255, 255, 255, 255);
	vertex[1].spc = GetColorU8(0, 0, 0, 0);
	vertex[1].u = 1.0f;
	vertex[1].v = 0.0f;
	vertex[1].su = 1.0f;
	vertex[1].sv = 0.0f;

	// 左下
	vertex[2].pos = VGet(TopLeft.x, BottomRight.y, TopLeft.z);
	vertex[2].norm = VGet(0.0f, 0.0f, -1.0f);
	vertex[2].dif = GetColorU8(255, 255, 255, 255);
	vertex[2].spc = GetColorU8(0, 0, 0, 0);
	vertex[2].u = 0.0f;
	vertex[2].v = 1.0f;
	vertex[2].su = 0.0f;
	vertex[2].sv = 1.0f;

	// 右下
	vertex[3].pos = BottomRight;
	vertex[3].norm = VGet(0.0f, 0.0f, -1.0f);
	vertex[3].dif = GetColorU8(255, 255, 255, 255);
	vertex[3].spc = GetColorU8(0, 0, 0, 0);
	vertex[3].u = 1.0f;
	vertex[3].v = 1.0f;
	vertex[3].su = 1.0f;
	vertex[3].sv = 1.0f;

	// ２ポリゴン分のインデックスデータをセット
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
	index[3] = 3;
	index[4] = 2;
	index[5] = 1;

	// ２ポリゴンの描画
	DrawPolygonIndexed3D(vertex, 4, index, 2, graphHandle, true);
}
