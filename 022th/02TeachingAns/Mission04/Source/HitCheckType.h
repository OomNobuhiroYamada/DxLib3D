#pragma once

/**
* @enum HitCheckType
* @brief 当たり判定方法の列挙型
*/
enum HitCheckType
{
	Type_None,		//!< 当たり判定なし
	Type_Sphere,	//!< 当たり判定球型
	Type_Capsule,	//!< 当たり判定カプセル
};
