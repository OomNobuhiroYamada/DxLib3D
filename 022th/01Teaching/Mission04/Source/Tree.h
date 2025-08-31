#pragma once

#include "DxLib.h"
#include "HitCheckType.h"
#include "Primitive.h"

/**
* @class Tree
* @brief 板ポリを2枚組み合わせて木を表示する
*/
class Tree {
private:
	int m_graphHandle;	// 画像ハンドル
	VECTOR m_position;	// 座標
	float m_scale;		// 拡大率

	//-------------------------------------------------------------------------------------------
	// 点と直線の最短距離
	float CalcPointLineDist(const Point &p, const Line &l, Point &h, float &t);

	// ∠p1p2p3は鋭角？
	bool IsSharpAngle(const Point &p1, const Point &p2, const Point &p3);

	// 点と線分の最短距離
	float CalcPointSegmentDist(const Point &p, const Segment &seg, Point &h, float &t);

	// 2直線の最短距離
	float CalcLineLineDist(const Line &l1, const Line &l2, Point &p1, Point &p2, float &t1, float &t2);

	// 0～1の間にクランプ
	void Clamp01(float &v);

	// 2線分の最短距離
	float CalcSegmentSegmentDist(const Segment &s1, const Segment &s2, Point &p1, Point &p2, float &t1, float &t2);
	//-------------------------------------------------------------------------------------------

public:
	static const float COLLISION_SPHERE_RADIUS;					//!< 当たり判定球の半径
	static const float COLLISION_HEIGHT;						//!< 当たり判定球の中心点の高さ

	static const float COLLISION_CAPSULE_RADIUS;				//!< 当たり判定カプセルの半径  35.0f
	static const float COLLISION_CAPSULE_HEIGHT;				//!< 当たり判定カプセルの高さ 140.0f

	Tree(VECTOR position, float scale, int graphHandle);		//!< コンストラクタ

	void Draw(int drawType);									//!< 描画
	bool CheckSphereToSphere(VECTOR centerPosition, float r);	//!< 球と球の当たり判定
	bool CheckCapsuleToCapsule(const Capsule &other);			//!< カプセルとカプセルの当たり判定
};
