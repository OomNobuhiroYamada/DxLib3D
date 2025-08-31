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


	//---------------------------------------------------------------------------------
}
