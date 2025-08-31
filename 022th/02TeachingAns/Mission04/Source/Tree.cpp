#include "Tree.h"
#include "DrawBoard.h"
/**
* @file
* @brief Mission04
* @author N.Yamada
* @date 2023/01/06
*
* @details 板ポリを2枚組み合わせて木を表示する
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/


const float Tree::COLLISION_SPHERE_RADIUS = 80.0f;		//!< 木の当たり判定球の半径
const float Tree::COLLISION_HEIGHT = 90.0f;				//!< 木の当たり判定球の中心点の高さ

const float Tree::COLLISION_CAPSULE_RADIUS = 20.0f;		//!< 当たり判定カプセルの半径
const float Tree::COLLISION_CAPSULE_HEIGHT = 80.0f;		//!< 当たり判定カプセルの高さ

/**
* @fn Tree::Tree
* @brief コンストラクタ
*/
Tree::Tree(VECTOR position, float scale, int graphHandle)
{
	m_graphHandle = graphHandle;
	m_position = position;
	m_scale = scale;
}

//---------------------------------------------------------------------------------
/**
* @fn Tree::CalcPointLineDist
* @brief 点と直線の最短距離
* @param[in] const Point &p, const Line &l, Point &h, float &t
* @return float 最短距離
*
* @details p : 点
* @details l : 直線
* @details h : 点から下ろした垂線の足（戻り値）
* @details t :ベクトル係数（戻り値）
*/
float Tree::CalcPointLineDist(const Point &p, const Line &l, Point &h, float &t)
{
	float lenSqV = l.v.lengthSq();
	t = 0.0f;
	if (lenSqV > 0.0f)
		t = l.v.dot(p - l.p) / lenSqV;

	h = l.p + t * l.v;
	return (h - p).length();
}

/**
* @fn Tree::IsSharpAngle
* @brief ∠p1p2p3は鋭角？
* @param[in] const Point &p1, const Point &p2, const Point &p3
* @return bool true 鋭角／ false 鋭角じゃない
*/
bool Tree::IsSharpAngle(const Point &p1, const Point &p2, const Point &p3)
{
	return Vec3(p1 - p2).isSharpAngle(p3 - p2);
}

/**
* @fn Tree::CalcPointSegmentDist
* @brief 点と線分の最短距離
* @param[in] const Point &p, const Segment &seg, Point &h, float &t
* @return float 最短距離
*
* @details p 　: 点
* @details seg : 線分
* @details h 　: 点から下ろした垂線の足（戻り値）
* @details t 　:ベクトル係数（戻り値）
*/
float Tree::CalcPointSegmentDist(const Point &p, const Segment &seg, Point &h, float &t)
{
	const Point e = seg.getEndPoint();

	// 垂線の長さ、垂線の足の座標及びtを算出
	float len = CalcPointLineDist(p, Line(seg.p, e - seg.p), h, t);

	if(IsSharpAngle(p, seg.p, e) == false)
	{
		// 始点側の外側
		h = seg.p;
		return (seg.p - p).length();
	}
	else if(IsSharpAngle(p, e, seg.p) == false)
	{
		// 終点側の外側
		h = e;
		return (e - p).length();
	}

	return len;
}

/**
* @fn Tree::CalcLineLineDist
* @brief 2直線の最短距離
* @param[in] const Line &l1, const Line &l2, Point &p1, Point &p2, float &t1, float &t2
* @return float 最短距離
*
* @details l1 : L1
* @details l2 : L2
* @details p1 : L1側の垂線の足（戻り値）
* @details p2 : L2側の垂線の足（戻り値）
* @details t1 : L1側のベクトル係数（戻り値）
* @details t2 : L2側のベクトル係数（戻り値）
*/
float Tree::CalcLineLineDist(const Line &l1, const Line &l2, Point &p1, Point &p2, float &t1, float &t2)
{
	// 2直線が平行？
	if(l1.v.isParallel(l2.v) == true)
	{

		// 点P11と直線L2の最短距離の問題に帰着
		float len = CalcPointLineDist(l1.p, l2, p2, t2);
		p1 = l1.p;
		t1 = 0.0f;

		return len;
	}

	// 2直線はねじれ関係
	float DV1V2 = l1.v.dot(l2.v);
	float DV1V1 = l1.v.lengthSq();
	float DV2V2 = l2.v.lengthSq();
	Vec3 P21P11 = l1.p - l2.p;
	t1 = (DV1V2 * l2.v.dot(P21P11) - DV2V2 * l1.v.dot(P21P11)) / (DV1V1 * DV2V2 - DV1V2 * DV1V2);
	p1 = l1.getPoint(t1);
	t2 = l2.v.dot(p1 - l2.p) / DV2V2;
	p2 = l2.getPoint(t2);

	return (p2 - p1).length();
}

/**
* @fn Tree::Clamp01
* @brief 0～1の間にクランプ
* @param[in] float &v
*/
void Tree::Clamp01(float &v)
{
	if(v < 0.0f)
	{
		v = 0.0f;
	}
	else if(v > 1.0f)
	{
		v = 1.0f;
	}
}

/**
* @fn Tree::CalcSegmentSegmentDist
* @brief 2線分の最短距離
* @param[in] const Segment &s1, const Segment &s2, Point &p1, Point &p2, float &t1, float &t2
* @return float 最短距離
*
* @details s1 : S1(線分1)
* @details s2 : S2(線分2)
* @details p1 : S1側の垂線の足（戻り値）
* @details p2 : S2側の垂線の足（戻り値）
* @details t1 : S1側のベクトル係数（戻り値）
* @details t2 : S2側のベクトル係数（戻り値）
*/
float Tree::CalcSegmentSegmentDist(const Segment &s1, const Segment &s2, Point &p1, Point &p2, float &t1, float &t2)
{
	// S1が縮退している？
	if(s1.v.lengthSq() < _OX_EPSILON_)
	{
		// S2も縮退？
		if(s2.v.lengthSq() < _OX_EPSILON_)
		{
			// 点と点の距離の問題に帰着
			float len = (s2.p - s1.p).length();
			p1 = s1.p;
			p2 = s2.p;
			t1 = t2 = 0.0f;
			return len;
		}
		else
		{
			// S1の始点とS2の最短問題に帰着
			float len = CalcPointSegmentDist(s1.p, s2, p2, t2);
			p1 = s1.p;
			t1 = 0.0f;
			Clamp01(t2);
			return len;
		}
	}

	// S2が縮退している？
	else if(s2.v.lengthSq() < _OX_EPSILON_)
	{
		// S2の始点とS1の最短問題に帰着
		float len = CalcPointSegmentDist(s2.p, s1, p1, t1);
		p2 = s2.p;
		Clamp01(t1);
		t2 = 0.0f;
		return len;
	}

	/* 線分同士 */

	// 2線分が平行だったら垂線の端点の一つをP1に仮決定
	if(s1.v.isParallel(s2.v) == true)
	{
		t1 = 0.0f;
		p1 = s1.p;
		float len = CalcPointSegmentDist(s1.p, s2, p2, t2);
		if(0.0f <= t2 && t2 <= 1.0f)
		{
			return len;
		}
	}
	else
	{
		// 線分はねじれの関係
		// 2直線間の最短距離を求めて仮のt1,t2を求める
		float len = CalcLineLineDist(s1, s2, p1, p2, t1, t2);
		if(0.0f <= t1 && t1 <= 1.0f && 0.0f <= t2 && t2 <= 1.0f)
		{
			return len;
		}
	}

	// 垂線の足が外にある事が判明
	// S1側のt1を0～1の間にクランプして垂線を降ろす
	Clamp01(t1);
	p1 = s1.getPoint(t1);
	float len = CalcPointSegmentDist(p1, s2, p2, t2);
	if(0.0f <= t2 && t2 <= 1.0f)
	{
		return len;
	}

	// S2側が外だったのでS2側をクランプ、S1に垂線を降ろす
	Clamp01(t2);
	p2 = s2.getPoint(t2);
	len = CalcPointSegmentDist(p2, s1, p1, t1);
	if(0.0f <= t1 && t1 <= 1.0f)
	{
		return len;
	}

	// 双方の端点が最短と判明
	Clamp01(t1);
	p1 = s1.getPoint(t1);
	return (p2 - p1).length();
}
//---------------------------------------------------------------------------------

/**
* @fn Tree::Draw
* @brief 描画
* @param[in] drawType
*/
void Tree::Draw(int drawType)
{
	DrawBoard(
		VGet(
			m_position.x - 100.0f * m_scale,
			m_position.y + 200.0f * m_scale,
			m_position.z
		),
		VGet(
			m_position.x + 100.0f * m_scale,
			m_position.y,
			m_position.z
		), m_graphHandle);

	DrawBoard(
		VGet(
			m_position.x,
			m_position.y + 200.0f * m_scale,
			m_position.z - 100.0f * m_scale
		),
		VGet(
			m_position.x,
			m_position.y,
			m_position.z + 100.0f * m_scale
		), m_graphHandle);

	//当たり判定を半透明で描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	switch(drawType)
	{
	// 球
	case HitCheckType::Type_Sphere:
		DrawSphere3D(
			VAdd(m_position, VGet(0.0f, COLLISION_HEIGHT * m_scale, 0.0f)),
			COLLISION_SPHERE_RADIUS * m_scale,
			8, GetColor(255, 255, 255), GetColor(255, 255, 255), false);
		break;

	// カプセル
	case HitCheckType::Type_Capsule:
		DrawCapsule3D(
			VAdd(m_position, VGet(0.0f, COLLISION_CAPSULE_RADIUS * m_scale, 0.0f)),
			VAdd(m_position, VGet(0.0f, COLLISION_CAPSULE_HEIGHT * m_scale, 0.0f)),
			COLLISION_CAPSULE_RADIUS * m_scale,
			8, GetColor(255, 255, 255), GetColor(255, 255, 255), false);
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

/**
* @fn Tree::CheckSphereToSphere
* @brief 球体と球体の衝突判定
* @param[in] centerPosition, r
* @return bool true　衝突した／ false 衝突していない
*/
bool Tree::CheckSphereToSphere(VECTOR centerPosition, float r)
{
	//---------------------------------------------------------------------------------
	return HitCheck_Sphere_Sphere(
		VAdd(m_position, VGet(0.0f, COLLISION_HEIGHT * m_scale, 0.0f)),
		COLLISION_SPHERE_RADIUS * m_scale,
		centerPosition,
		r);
	//---------------------------------------------------------------------------------
}

/**
* @fn Tree::CheckCapsuleToCapsule
* @brief カプセルとカプセルの衝突判定
* @param[in] &other
* @return bool true　衝突した／ false 衝突していない
*/
bool Tree::CheckCapsuleToCapsule(const Capsule &other)
{
	//---------------------------------------------------------------------------------

	Capsule self(
		Point(m_position.x, m_position.y + COLLISION_CAPSULE_RADIUS * m_scale, m_position.z),
		Point(m_position.x, m_position.y + COLLISION_CAPSULE_HEIGHT * m_scale, m_position.z),
		COLLISION_CAPSULE_RADIUS * m_scale);

	Point p1, p2;
	float t1, t2;
	float d = CalcSegmentSegmentDist(other.s, self.s, p1, p2, t1, t2);
	return (d <= other.r + self.r);
	//---------------------------------------------------------------------------------
}
