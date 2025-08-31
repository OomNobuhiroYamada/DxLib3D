#include "DxLib.h"
#include <malloc.h>
/**
* @file
* @brief Lesson36
* @author N.Yamada
* @date 2023/01/06
*
* @details 3Dモデルのポリゴンを使用した最短経路探索
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

const int   MOVESPEED = 20;				//!< 移動速度
const float SPHERESIZE = 150.0f;			//!< 球体のサイズ
const float COLLWIDTH = 400.0f;				//!< 当たり判定のサイズ

/**
* @struct POLYLINKINFO
* @brief ポリゴン同士の連結情報を保存する為の構造体
*/
struct POLYLINKINFO
{
	int linkPolyIndex[3];					//!< ポリゴンの三つの辺とそれぞれ隣接しているポリゴンのポリゴン番号( -1：隣接ポリゴン無し  -1以外：ポリゴン番号 )
	float linkPolyDistance[3];				//!< 隣接しているポリゴンとの距離
	VECTOR centerPosition;					//!< ポリゴンの中心座標
};

/**
* @struct PATHPLANNING_UNIT
* @brief 経路探索処理用の１ポリゴンの情報
*/
struct PATHPLANNING_UNIT
{
	int polyIndex;							//!< ポリゴン番号
	float totalDistance;					//!< 経路探索でこのポリゴンに到達するまでに通過したポリゴン間の距離の合計
	int prevPolyIndex;						//!< 経路探索で確定した経路上の一つ前のポリゴン( 当ポリゴンが経路上に無い場合は -1 )
	int nextPolyIndex;						//!< 経路探索で確定した経路上の一つ先のポリゴン( 当ポリゴンが経路上に無い場合は -1 )
	PATHPLANNING_UNIT *activeNextUnit;		//!< 経路探索処理対象になっている次のポリゴンのメモリアドレスを格納する変数
};

/**
* @struct PATHPLANNING
* @brief 経路探索処理で使用する情報を保存する為の構造体
*/
struct PATHPLANNING
{
	VECTOR startPosition;					//!< 開始位置
	VECTOR goalPosition;					//!< 目標位置
	PATHPLANNING_UNIT *unitArray;			//!< 経路探索処理で使用する全ポリゴンの情報配列が格納されたメモリ領域の先頭メモリアドレスを格納する変数
	PATHPLANNING_UNIT *activeFirstUnit;		//!< 経路探索処理対象になっているポリゴン群の最初のポリゴン情報へのメモリアドレスを格納する変数
	PATHPLANNING_UNIT *startUnit;			//!< 経路のスタート地点にあるポリゴン情報へのメモリアドレスを格納する変数
	PATHPLANNING_UNIT *goalUnit;			//!< 経路のゴール地点にあるポリゴン情報へのメモリアドレスを格納する変数
};

/**
* @struct PATHMOVEINFO
* @brief 探索した経路を移動する処理に使用する情報を纏めた構造体
*/
struct PATHMOVEINFO
{
	int nowPolyIndex;							//!< 現在乗っているポリゴンの番号
	VECTOR nowPosition;							//!< 現在位置
	VECTOR moveDirection;						//!< 移動方向
	PATHPLANNING_UNIT *nowPathPlanningUnit;		//!< 現在乗っているポリゴンの経路探索情報が格納されているメモリアドレスを格納する変数
	PATHPLANNING_UNIT *targetPathPlanningUnit;	//!< 次の中間地点となる経路上のポリゴンの経路探索情報が格納されているメモリアドレスを格納する変数
};

int stageModelHandle;							//!< ステージモデルハンドル
MV1_REF_POLYGONLIST polyList;					//!< ステージモデルのポリゴン情報

POLYLINKINFO *polyLinkInfo;						//!< ステージモデルの全ポリゴン分の「ポリゴン同士の連結情報」の配列が格納が格納されたメモリエリアの先頭アドレスを格納する変数
PATHPLANNING pathPlanning;						//!< 経路探索処理用の構造体
PATHMOVEINFO pathMove;							//!< 探索した経路を移動する処理に使用する情報を纏めた構造体


int CheckOnPolyIndex(VECTOR Pos);				//!< 指定の座標の直下、若しくは直上にあるポリゴンの番号を取得する( ポリゴンが無かった場合は -1 を返す )

void SetupPolyLinkInfo(void);					//!< ポリゴン同士の連結情報を構築する
void TerminatePolyLinkInfo(void);				//!< ポリゴン同士の連結情報の後始末を行う
bool CheckPolyMove(VECTOR startPos, VECTOR targetPos);	//!< ポリゴン同士の連結情報を使用して指定の二つの座標間を直線的に移動できるかどうかをチェックする( 戻り値  true:直線的に移動できる  false:直線的に移動できない )
bool CheckPolyMoveWidth(VECTOR startPos, VECTOR targetPos, float width);	//!< ポリゴン同士の連結情報を使用して指定の二つの座標間を直線的に移動できるかどうかをチェックする( 戻り値  true:直線的に移動できる  false:直線的に移動できない )( 幅指定版 )

bool SetupPathPlanning(VECTOR startPos, VECTOR goalPos);			//!< 指定の２点の経路を探索する( 戻り値  true:経路構築成功  false:経路構築失敗( スタート地点とゴール地点を繋ぐ経路が無かった等 ) )
void TerminatePathPlanning(void);				//!< 経路探索情報の後始末

void MoveInitialize(void);						//!< 探索した経路を移動する処理の初期化を行う関数
void MoveProcess(void);							//!< 探索した経路を移動する処理の１フレーム分の処理を行う関数
bool RefreshMoveDirection(void);				//!< 探索した経路を移動する処理で移動方向を更新する処理を行う関数( 戻り値  true:ゴールに辿り着いている  false:ゴールに辿り着いていない )

/**
* @fn CheckOnPolyIndex
* @brief 指定の座標の直下、若しくは直上にあるポリゴンの番号を取得
* @param[in] VECTOR Pos
* @return int ポリゴンが無かった場合は -1
*/
int CheckOnPolyIndex(VECTOR Pos)
{
	HITRESULT_LINE hitRes;

	// 指定の座標のY軸方向に大きく伸びる線分の２座標をセット
	VECTOR linePos1 = VGet(Pos.x, 1000000.0f, Pos.z);
	VECTOR linePos2 = VGet(Pos.x, -1000000.0f, Pos.z);

	// ステージモデルのポリゴンの数だけ繰り返し
	MV1_REF_POLYGON *refPoly = polyList.Polygons;
	for(int i=0; i<polyList.PolygonNum; i++, refPoly++)
	{
		// 線分と接するポリゴンがあったらそのポリゴンの番号を返す
		hitRes = HitCheck_Line_Triangle(
			linePos1,
			linePos2,
			polyList.Vertexs[refPoly->VIndex[0]].Position,
			polyList.Vertexs[refPoly->VIndex[1]].Position,
			polyList.Vertexs[refPoly->VIndex[2]].Position
		);
		if(hitRes.HitFlag)
		{
			return i;
		}
	}

	// ここに来たら線分と接するポリゴンが無かったということなので -1 を返す
	return -1;
}

/**
* @fn SetupPolyLinkInfo
* @brief ポリゴン同士の連結情報を構築する
*/
void SetupPolyLinkInfo()
{
	POLYLINKINFO *pLInfoSub;
	MV1_REF_POLYGON *refPolySub;

	// ステージモデル全体の参照用メッシュを構築する
	MV1SetupReferenceMesh(stageModelHandle, 0, true);

	// ステージモデル全体の参照用メッシュの情報を取得する
	polyList = MV1GetReferenceMesh(stageModelHandle, 0, true);

	// ステージモデルの全ポリゴンの連結情報を格納する為のメモリ領域を確保する
	polyLinkInfo = (POLYLINKINFO *)malloc(sizeof(POLYLINKINFO) * polyList.PolygonNum);

	// 全ポリゴンの中心座標を算出
	POLYLINKINFO *pLInfo = polyLinkInfo;
	MV1_REF_POLYGON *refPoly = polyList.Polygons;
	for(int i=0; i<polyList.PolygonNum; i++, pLInfo++, refPoly++)
	{
		pLInfo->centerPosition =
			VScale(VAdd(polyList.Vertexs[refPoly->VIndex[0]].Position,
				VAdd(polyList.Vertexs[refPoly->VIndex[1]].Position,
					polyList.Vertexs[refPoly->VIndex[2]].Position)), 1.0f / 3.0f);
	}

	// ポリゴン同士の隣接情報の構築
	pLInfo = polyLinkInfo;
	refPoly = polyList.Polygons;
	for(int i=0; i<polyList.PolygonNum; i++, pLInfo++, refPoly++)
	{
		// 各辺に隣接ポリゴンは無い、の状態にしておく
		pLInfo->linkPolyIndex[0] = -1;
		pLInfo->linkPolyIndex[1] = -1;
		pLInfo->linkPolyIndex[2] = -1;

		// 隣接するポリゴンを探すためにポリゴンの数だけ繰り返し
		refPolySub = polyList.Polygons;
		pLInfoSub = polyLinkInfo;
		for(int j=0; j<polyList.PolygonNum; j++, refPolySub++, pLInfoSub++)
		{
			// 自分自身のポリゴンだったら何もせず次のポリゴンへ
			if(i == j)
			{
				continue;
			}

			// ポリゴンの頂点番号0と1で形成する辺と隣接していたら隣接情報に追加する
			if(pLInfo->linkPolyIndex[0] == -1 &&
				((refPoly->VIndex[0] == refPolySub->VIndex[0] && refPoly->VIndex[1] == refPolySub->VIndex[2]) ||
				(refPoly->VIndex[0] == refPolySub->VIndex[1] && refPoly->VIndex[1] == refPolySub->VIndex[0]) ||
				(refPoly->VIndex[0] == refPolySub->VIndex[2] && refPoly->VIndex[1] == refPolySub->VIndex[1])))
			{
				pLInfo->linkPolyIndex[0] = j;
				pLInfo->linkPolyDistance[0] = VSize(VSub(pLInfoSub->centerPosition, pLInfo->centerPosition));
			}
			else
			{
				// ポリゴンの頂点番号1と2で形成する辺と隣接していたら隣接情報に追加する
				if(pLInfo->linkPolyIndex[1] == -1 &&
					((refPoly->VIndex[1] == refPolySub->VIndex[0] && refPoly->VIndex[2] == refPolySub->VIndex[2]) ||
					(refPoly->VIndex[1] == refPolySub->VIndex[1] && refPoly->VIndex[2] == refPolySub->VIndex[0]) ||
					(refPoly->VIndex[1] == refPolySub->VIndex[2] && refPoly->VIndex[2] == refPolySub->VIndex[1])))
				{
					pLInfo->linkPolyIndex[1] = j;
					pLInfo->linkPolyDistance[1] = VSize(VSub(pLInfoSub->centerPosition, pLInfo->centerPosition));
				}
				else
				{
					// ポリゴンの頂点番号2と0で形成する辺と隣接していたら隣接情報に追加する
					if(pLInfo->linkPolyIndex[2] == -1 &&
						((refPoly->VIndex[2] == refPolySub->VIndex[0] && refPoly->VIndex[0] == refPolySub->VIndex[2]) ||
						(refPoly->VIndex[2] == refPolySub->VIndex[1] && refPoly->VIndex[0] == refPolySub->VIndex[0]) ||
						(refPoly->VIndex[2] == refPolySub->VIndex[2] && refPoly->VIndex[0] == refPolySub->VIndex[1])))
					{
						pLInfo->linkPolyIndex[2] = j;
						pLInfo->linkPolyDistance[2] = VSize(VSub(pLInfoSub->centerPosition, pLInfo->centerPosition));
					}
				}
			}
		}
	}
}

/**
* @fn TerminatePolyLinkInfo
* @brief ポリゴン同士の連結情報の後始末を行う
*/
void TerminatePolyLinkInfo()
{
	// ポリゴン同士の連結情報を格納していたメモリ領域を解放
	free(polyLinkInfo);
	polyLinkInfo = NULL;
}

/**
* @fn CheckPolyMove
* @brief ポリゴン同士の連結情報を使用して指定の二つの座標間を直線的に移動できるかどうかをチェック
* @param[in] VECTOR startPos, VECTOR targetPos
* @return bool true:直線的に移動できる  false:直線的に移動できない
*/
bool CheckPolyMove(VECTOR startPos, VECTOR targetPos)
{
	VECTOR_D polyPos[3];
	int nextCheckPoly[3];
	int nextCheckPolyPrev[3];
	int nextCheckPolyNum;
	int nextCheckPolyPrevNum;

	// 開始座標と目標座標の y座標値を 0.0f にして、平面上の判定にする
	startPos.y = 0.0f;
	targetPos.y = 0.0f;

	// 精度を上げるために double型にする
	VECTOR_D startPosD = VConvFtoD(startPos);
	VECTOR_D targetPosD = VConvFtoD(targetPos);

	// 開始座標と目標座標の直上、若しくは直下に存在するポリゴンを検索する
	int startPoly = CheckOnPolyIndex(startPos);
	int targetPoly = CheckOnPolyIndex(targetPos);

	// ポリゴンが存在しなかったら移動できないので false を返す
	if(startPoly == -1 || targetPoly == -1)
	{
		return false;
	}

	// 開始座標と目標座標の直上、若しくは直下に存在するポリゴンの連結情報のアドレスを取得しておく
	POLYLINKINFO *pInfoStart = &polyLinkInfo[startPoly];
	POLYLINKINFO *pInfoTarget = &polyLinkInfo[targetPoly];

	// 指定線分上にあるかどうかをチェックするポリゴンとして開始座標の直上、若しくは直下に存在するポリゴンを登録
	int checkPolyNum = 1;
	int checkPoly[3];
	checkPoly[0] = startPoly;
	int checkPolyPrevNum = 0;
	int checkPolyPrev[3];
	checkPolyPrev[0] = -1;

	// 結果が出るまで無条件で繰り返し
	while(1)
	{
		// 次のループでチェック対象になるポリゴンの数をリセットしておく
		nextCheckPolyNum = 0;

		// 次のループでチェック対象から外すポリゴンの数をリセットしておく
		nextCheckPolyPrevNum = 0;

		// チェック対象のポリゴンの数だけ繰り返し
		for(int i=0; i<checkPolyNum; i++)
		{
			// チェック対象のポリゴンの３座標を取得
			polyPos[0] = VConvFtoD(polyList.Vertexs[polyList.Polygons[checkPoly[i]].VIndex[0]].Position);
			polyPos[1] = VConvFtoD(polyList.Vertexs[polyList.Polygons[checkPoly[i]].VIndex[1]].Position);
			polyPos[2] = VConvFtoD(polyList.Vertexs[polyList.Polygons[checkPoly[i]].VIndex[2]].Position);

			// y座標を0.0にして、平面的な判定を行うようにする
			polyPos[0].y = 0.0;
			polyPos[1].y = 0.0;
			polyPos[2].y = 0.0;

			// ポリゴンの頂点番号0と1の辺に隣接するポリゴンが存在する場合で、
			// 且つ辺の線分と移動開始点、終了点で形成する線分が接していたら if 文が真になる
			if(polyLinkInfo[checkPoly[i]].linkPolyIndex[0] != -1 &&
				Segment_Segment_MinLength_SquareD(startPosD, targetPosD, polyPos[0], polyPos[1]) < 0.001)
			{
				// もし辺と接しているポリゴンが目標座標上に存在するポリゴンだったら
				// 開始座標から目標座標上まで途切れなくポリゴンが存在するということなので true を返す
				if(polyLinkInfo[checkPoly[i]].linkPolyIndex[0] == targetPoly)
				{
					return true;
				}

				// 辺と接しているポリゴンを次のチェック対象のポリゴンに加える

				// 既に登録されているポリゴンの場合は加えない
				int j;
				for(j=0; j<nextCheckPolyNum; j++)
				{
					if(nextCheckPoly[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[0])
					{
						break;
					}
				}
				if(j == nextCheckPolyNum)
				{
					// 次のループで除外するポリゴンの対象に加える

					// 既に登録されている除外ポリゴンの場合は加えない
					for(j=0; j<nextCheckPolyPrevNum; j++)
					{
						if(nextCheckPolyPrev[j] == checkPoly[i])
						{
							break;
						}
					}
					if(j == nextCheckPolyPrevNum)
					{
						nextCheckPolyPrev[nextCheckPolyPrevNum] = checkPoly[i];
						nextCheckPolyPrevNum++;
					}

					// 一つ前のループでチェック対象になったポリゴンの場合も加えない
					for(j=0; j<checkPolyPrevNum; j++)
					{
						if(checkPolyPrev[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[0])
						{
							break;
						}
					}
					if(j == checkPolyPrevNum)
					{
						// ここまで来たら漸く次のチェック対象のポリゴンに加える
						nextCheckPoly[nextCheckPolyNum] = polyLinkInfo[checkPoly[i]].linkPolyIndex[0];
						nextCheckPolyNum++;
					}
				}
			}

			// ポリゴンの頂点番号1と2の辺に隣接するポリゴンが存在する場合で、
			// 且つ辺の線分と移動開始点、終了点で形成する線分が接していたら if 文が真になる
			if(polyLinkInfo[checkPoly[i]].linkPolyIndex[1] != -1 &&
				Segment_Segment_MinLength_SquareD(startPosD, targetPosD, polyPos[1], polyPos[2]) < 0.001)
			{
				// もし辺と接しているポリゴンが目標座標上に存在するポリゴンだったら
				// 開始座標から目標座標上まで途切れなくポリゴンが存在するということなので true を返す
				if(polyLinkInfo[checkPoly[i]].linkPolyIndex[1] == targetPoly)
				{
					return true;
				}

				// 辺と接しているポリゴンを次のチェック対象のポリゴンに加える

				int j;
				// 既に登録されているポリゴンの場合は加えない
				for(j=0; j<nextCheckPolyNum; j++)
				{
					if(nextCheckPoly[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[1])
					{
						break;
					}
				}
				if(j == nextCheckPolyNum)
				{
					// 既に登録されている除外ポリゴンの場合は加えない
					for(j=0; j<nextCheckPolyPrevNum; j++)
					{
						if(nextCheckPolyPrev[j] == checkPoly[i])
						{
							break;
						}
					}
					if(j == nextCheckPolyPrevNum)
					{
						nextCheckPolyPrev[nextCheckPolyPrevNum] = checkPoly[i];
						nextCheckPolyPrevNum++;
					}

					// 一つ前のループでチェック対象になったポリゴンの場合も加えない
					for(j=0; j<checkPolyPrevNum; j++)
					{
						if(checkPolyPrev[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[1])
						{
							break;
						}
					}
					if(j == checkPolyPrevNum)
					{
						// ここまで来たら漸く次のチェック対象のポリゴンに加える
						nextCheckPoly[nextCheckPolyNum] = polyLinkInfo[checkPoly[i]].linkPolyIndex[1];
						nextCheckPolyNum++;
					}
				}
			}

			// ポリゴンの頂点番号2と0の辺に隣接するポリゴンが存在する場合で、
			// 且つ辺の線分と移動開始点、終了点で形成する線分が接していたら if 文が真になる
			if(polyLinkInfo[checkPoly[i]].linkPolyIndex[2] != -1 &&
				Segment_Segment_MinLength_SquareD(startPosD, targetPosD, polyPos[2], polyPos[0]) < 0.001)
			{
				// もし辺と接しているポリゴンが目標座標上に存在するポリゴンだったら
				// 開始座標から目標座標上まで途切れなくポリゴンが存在するということなので true を返す
				if(polyLinkInfo[checkPoly[i]].linkPolyIndex[2] == targetPoly)
				{
					return true;
				}

				// 辺と接しているポリゴンを次のチェック対象のポリゴンに加える

				int j;
				// 既に登録されているポリゴンの場合は加えない
				for(j=0; j<nextCheckPolyNum; j++)
				{
					if(nextCheckPoly[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[2])
					{
						break;
					}
				}
				if(j == nextCheckPolyNum)
				{
					// 既に登録されている除外ポリゴンの場合は加えない
					for(j=0; j<nextCheckPolyPrevNum; j++)
					{
						if(nextCheckPolyPrev[j] == checkPoly[i])
						{
							break;
						}
					}
					if(j == nextCheckPolyPrevNum)
					{
						nextCheckPolyPrev[nextCheckPolyPrevNum] = checkPoly[i];
						nextCheckPolyPrevNum++;
					}

					// 一つ前のループでチェック対象になったポリゴンの場合も加えない
					for(j=0; j<checkPolyPrevNum; j++)
					{
						if(checkPolyPrev[j] == polyLinkInfo[checkPoly[i]].linkPolyIndex[2])
						{
							break;
						}
					}
					if(j == checkPolyPrevNum)
					{
						// ここまで来たら漸く次のチェック対象のポリゴンに加える
						nextCheckPoly[nextCheckPolyNum] = polyLinkInfo[checkPoly[i]].linkPolyIndex[2];
						nextCheckPolyNum++;
					}
				}
			}
		}

		// 次のループでチェック対象になるポリゴンが一つもなかったということは
		// 移動開始点、終了点で形成する線分と接するチェック対象のポリゴンに隣接する
		// ポリゴンが一つもなかったということなので、直線的な移動はできないということで false を返す
		if(nextCheckPolyNum == 0)
		{
			return false;
		}

		// 次にチェック対象となるポリゴンの情報をコピーする
		for(int i=0; i<nextCheckPolyNum; i++)
		{
			checkPoly[i] = nextCheckPoly[i];
		}
		checkPolyNum = nextCheckPolyNum;

		// 次にチェック対象外となるポリゴンの情報をコピーする
		for(int i=0; i<nextCheckPolyPrevNum; i++)
		{
			checkPolyPrev[i] = nextCheckPolyPrev[i];
		}
		checkPolyPrevNum = nextCheckPolyPrevNum;
	}
}

/**
* @fn CheckPolyMoveWidth
* @brief ポリゴン同士の連結情報を使用して指定の二つの座標間を直線的に移動できるかどうかをチェック( 幅指定版 )
* @param[in] VECTOR startPos, VECTOR targetPos, float width
* @return bool true:直線的に移動できる  false:直線的に移動できない
*/
bool CheckPolyMoveWidth(VECTOR startPos, VECTOR targetPos, float width)
{
	// 最初に開始座標から目標座標に直線的に移動できるかどうかをチェック
	if(CheckPolyMove(startPos, targetPos) == false)
	{
		return false;
	}

	// 開始座標から目標座標に向かうベクトルを算出
	VECTOR direction = VSub(targetPos, startPos);

	// y座標を 0.0f にして平面的なベクトルにする
	direction.y = 0.0f;

	// 開始座標から目標座標に向かうベクトルに直角な正規化ベクトルを算出
	VECTOR sideDirection = VCross(direction, VGet(0.0f, 1.0f, 0.0f));
	sideDirection = VNorm(sideDirection);

	// 開始座標と目標座標を width / 2.0f 分だけ垂直方向にずらして、再度直線的に移動できるかどうかをチェック
	VECTOR tempVec = VScale(sideDirection, width / 2.0f);
	if(CheckPolyMove(VAdd(startPos, tempVec), VAdd(targetPos, tempVec)) == false)
	{
		return false;
	}

	// 開始座標と目標座標を width / 2.0f 分だけ一つ前とは逆方向の垂直方向にずらして、再度直線的に移動できるかどうかをチェック
	tempVec = VScale(sideDirection, -width / 2.0f);
	if(CheckPolyMove(VAdd(startPos, tempVec), VAdd(targetPos, tempVec)) == false)
	{
		return false;
	}

	// ここまできたら指定の幅があっても直線的に移動できるということなので true を返す
	return true;
}

/**
* @fn SetupPathPlanning
* @brief 指定の２点の経路を探索
* @param[in] VECTOR startPos, VECTOR goalPos
* @return bool true:経路構築成功  false:経路構築失敗( スタート地点とゴール地点を繋ぐ経路が無かった等 )
*/
bool SetupPathPlanning(VECTOR startPos, VECTOR goalPos)
{
	PATHPLANNING_UNIT *pUnitSub;
	PATHPLANNING_UNIT *pUnitSub2;

	// スタート位置とゴール位置を保存
	pathPlanning.startPosition = startPos;
	pathPlanning.goalPosition = goalPos;

	// 経路探索用のポリゴン情報を格納するメモリ領域を確保する
	pathPlanning.unitArray = (PATHPLANNING_UNIT *)malloc(sizeof(PATHPLANNING_UNIT) * polyList.PolygonNum);

	// 経路探索用のポリゴン情報を初期化
	PATHPLANNING_UNIT *pUnit = pathPlanning.unitArray;
	for(int i=0; i<polyList.PolygonNum; i++, pUnit++)
	{
		pUnit->polyIndex = i;
		pUnit->totalDistance = 0.0f;
		pUnit->prevPolyIndex = -1;
		pUnit->nextPolyIndex = -1;
		pUnit->activeNextUnit = NULL;
	}

	// スタート地点にあるポリゴンの番号を取得し、ポリゴンの経路探索処理用の構造体のアドレスを保存
	int polyIndex = CheckOnPolyIndex(startPos);
	if(polyIndex == -1)
	{
		return false;
	}
	pathPlanning.startUnit = &pathPlanning.unitArray[polyIndex];

	// 経路探索処理対象のポリゴンとしてスタート地点にあるポリゴンを登録する
	pathPlanning.activeFirstUnit = &pathPlanning.unitArray[polyIndex];

	// ゴール地点にあるポリゴンの番号を取得し、ポリゴンの経路探索処理用の構造体のアドレスを保存
	polyIndex = CheckOnPolyIndex(goalPos);
	if(polyIndex == -1)
	{
		return false;
	}
	pathPlanning.goalUnit = &pathPlanning.unitArray[polyIndex];

	// ゴール地点にあるポリゴンとスタート地点にあるポリゴンが同じだったら false を返す
	if(pathPlanning.goalUnit == pathPlanning.startUnit)
	{
		return false;
	}

	// 経路を探索してゴール地点のポリゴンにたどり着くまでループを繰り返す
	bool goal = false;
	while(goal == false)
	{
		// 経路探索処理対象になっているポリゴンをすべて処理
		pUnit = pathPlanning.activeFirstUnit;
		pathPlanning.activeFirstUnit = NULL;
		for(; pUnit != NULL; pUnit = pUnit->activeNextUnit)
		{
			// ポリゴンの辺の数だけ繰り返し
			for(int i=0; i<3; i++)
			{
				// 辺に隣接するポリゴンが無い場合は何もしない
				if(polyLinkInfo[pUnit->polyIndex].linkPolyIndex[i] == -1)
				{
					continue;
				}

				// 隣接するポリゴンが既に経路探索処理が行われていて、且つより距離の長い経路となっているか、
				// スタート地点のポリゴンだった場合は何もしない
				pUnitSub = &pathPlanning.unitArray[polyLinkInfo[pUnit->polyIndex].linkPolyIndex[i]];
				if((pUnitSub->prevPolyIndex != -1 && pUnitSub->totalDistance <= pUnit->totalDistance + polyLinkInfo[pUnit->polyIndex].linkPolyDistance[i])
					|| pUnitSub->polyIndex == pathPlanning.startUnit->polyIndex)
				{
					continue;
				}

				// 隣接するポリゴンがゴール地点にあるポリゴンだったらゴールに辿り着いたフラグを立てる
				if(pUnitSub->polyIndex == pathPlanning.goalUnit->polyIndex)
				{
					goal = true;
				}

				// 隣接するポリゴンに経路情報となる自分のポリゴンの番号を代入する
				pUnitSub->prevPolyIndex = pUnit->polyIndex;

				// 隣接するポリゴンにここに到達するまでの距離を代入する
				pUnitSub->totalDistance = pUnit->totalDistance + polyLinkInfo[pUnit->polyIndex].linkPolyDistance[i];

				// 次のループで行う経路探索処理対象に追加する、既に追加されていたら追加しない
				for(pUnitSub2 = pathPlanning.activeFirstUnit; pUnitSub2 != NULL; pUnitSub2 = pUnitSub2->activeNextUnit)
				{
					if(pUnitSub2 == pUnitSub)
					{
						break;
					}
				}
				if(pUnitSub2 == NULL)
				{
					pUnitSub->activeNextUnit = pathPlanning.activeFirstUnit;
					pathPlanning.activeFirstUnit = pUnitSub;
				}
			}
		}

		// ここにきた時に pathPlanning.activeFirstUnit が NULL ということは
		// スタート地点にあるポリゴンからゴール地点にあるポリゴンに辿り着けないということなので false を返す
		if(pathPlanning.activeFirstUnit == NULL)
		{
			return false;
		}
	}

	// ゴール地点のポリゴンからスタート地点のポリゴンに辿って
	// 経路上のポリゴンに次に移動すべきポリゴンの番号を代入する
	pUnit = pathPlanning.goalUnit;
	do
	{
		pUnitSub = pUnit;
		pUnit = &pathPlanning.unitArray[pUnitSub->prevPolyIndex];

		pUnit->nextPolyIndex = pUnitSub->polyIndex;

	} while(pUnit != pathPlanning.startUnit);

	// ここにきたらスタート地点からゴール地点までの経路が探索できたということなので true を返す
	return true;
}

/**
* @fn TerminatePathPlanning
* @brief 経路探索情報の後始末
*/
void TerminatePathPlanning()
{
	// 経路探索の為に確保したメモリ領域を解放
	free(pathPlanning.unitArray);
	pathPlanning.unitArray = NULL;
}

/**
* @fn MoveInitialize
* @brief 探索した経路を移動する処理の初期化を行う
*/
void MoveInitialize()
{
	// 移動開始時点で乗っているポリゴンはスタート地点にあるポリゴン
	pathMove.nowPolyIndex = pathPlanning.startUnit->polyIndex;

	// 移動開始時点の座標はスタート地点にあるポリゴンの中心座標
	pathMove.nowPosition = polyLinkInfo[pathMove.nowPolyIndex].centerPosition;

	// 移動開始時点の経路探索情報はスタート地点にあるポリゴンの情報
	pathMove.nowPathPlanningUnit = pathPlanning.startUnit;

	// 移動開始時点の移動中間地点の経路探索情報もスタート地点にあるポリゴンの情報
	pathMove.targetPathPlanningUnit = pathPlanning.startUnit;
}

/**
* @fn MoveProcess
* @brief 探索した経路を移動する処理の１フレーム分の処理を行う
*/
void MoveProcess()
{
	// 移動方向の更新、ゴールに辿り着いていたら移動はせずに終了する
	if(RefreshMoveDirection())
	{
		return;
	}

	// 移動方向に座標を移動する
	pathMove.nowPosition = VAdd(pathMove.nowPosition, VScale(pathMove.moveDirection, MOVESPEED));

	// 現在の座標で乗っているポリゴンを検索する
	pathMove.nowPolyIndex = CheckOnPolyIndex(pathMove.nowPosition);

	// 現在の座標で乗っているポリゴンの経路探索情報のメモリアドレスを代入する
	pathMove.nowPathPlanningUnit = &pathPlanning.unitArray[pathMove.nowPolyIndex];
}

/**
* @fn RefreshMoveDirection
* @brief 探索した経路を移動する処理で移動方向を更新する処理を行う
* @return bool true:ゴールに辿り着いている  false:ゴールに辿り着いていない
*/
bool RefreshMoveDirection()
{
	PATHPLANNING_UNIT *tempPUnit;

	// 現在乗っているポリゴンがゴール地点にあるポリゴンの場合は処理を分岐
	if(pathMove.nowPathPlanningUnit == pathPlanning.goalUnit)
	{
		// 方向は目標座標
		pathMove.moveDirection = VSub(pathPlanning.goalPosition, pathMove.nowPosition);
		pathMove.moveDirection.y = 0.0f;

		// 目標座標までの距離が移動速度以下だったらゴールに辿りついたことにする
		if(VSize(pathMove.moveDirection) <= MOVESPEED)
		{
			return true;
		}

		// それ以外の場合はまだたどり着いていないものとして移動する
		pathMove.moveDirection = VNorm(pathMove.moveDirection);

		return false;
	}

	// 現在乗っているポリゴンが移動中間地点のポリゴンの場合は次の中間地点を決定する処理を行う
	if(pathMove.nowPathPlanningUnit == pathMove.targetPathPlanningUnit)
	{
		// 次の中間地点が決定するまでループし続ける
		for(;;)
		{
			tempPUnit = &pathPlanning.unitArray[pathMove.targetPathPlanningUnit->nextPolyIndex];

			// 経路上の次のポリゴンの中心座標に直線的に移動できない場合はループから抜ける
			if(CheckPolyMoveWidth(pathMove.nowPosition, polyLinkInfo[tempPUnit->polyIndex].centerPosition, COLLWIDTH) == false)
			{
				break;
			}

			// チェック対象を経路上の更に一つ先のポリゴンに変更する
			pathMove.targetPathPlanningUnit = tempPUnit;

			// もしゴール地点のポリゴンだったらループを抜ける
			if(pathMove.targetPathPlanningUnit == pathPlanning.goalUnit)
			{
				break;
			}
		}
	}

	// 移動方向を決定する、移動方向は現在の座標から中間地点のポリゴンの中心座標に向かう方向
	pathMove.moveDirection = VSub(polyLinkInfo[pathMove.targetPathPlanningUnit->polyIndex].centerPosition, pathMove.nowPosition);
	pathMove.moveDirection.y = 0.0f;
	pathMove.moveDirection = VNorm(pathMove.moveDirection);

	// ここに来たということはゴールに辿り着いていないので false を返す
	return false;
}

const int CAMERA_ANGLE_SPEED = 3;	//!< カメラの回転速度

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	PATHPLANNING_UNIT *pUnit;

	// ウインドウモードで起動
	ChangeWindowMode(true);

	// DXライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// ステージモデルの読み込み
	stageModelHandle = MV1LoadModel("Resource/pathPlanning.mqo");

	// ステージモデルのポリゴン同士の連結情報を構築する
	SetupPolyLinkInfo();

	// 指定の２点の経路情報を探索する
	SetupPathPlanning(VGet(-7400.0f, 0.0f, -7400.0f), VGet(7400.0f, 0.0f, 7400.0f));

	// 探索した経路上を移動する準備を行う
	MoveInitialize();

	// カメラの設定
	{
		// X軸とY軸の回転から回転行列を作成
		MATRIX rotateMatrix = MMult(MGetRotX(65.0f / 180.0f * DX_PI_F), MGetRotY(180.0f / 180.0f * DX_PI_F));

		// カメラ座標に進行ベクトルを加算
		VECTOR cameraPosition = VGet(0.0f, 15000.0f, 8300.0f);

		// 回転行列×平行移動行列でカメラ行列を作る
		MATRIX cameraMatrix = MMult(rotateMatrix, MGetTranslate(cameraPosition));

		// カメラ行列の逆行列をビュー行列として設定
		SetCameraViewMatrix(MInverse(cameraMatrix));
	}

	// モデル全体のコリジョン情報を構築
	MV1SetupCollInfo(stageModelHandle, -1, 8, 8, 8);

	// メインループ(ESCキーが押されたらループを抜ける)
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面の初期化
		ClearDrawScreen();

		// 移動先の変更
		if(GetMouseInput() & MOUSE_INPUT_LEFT)
		{
			// マウスポインタの座標を取得
			int mouseX, mouseY;
			GetMousePoint(&mouseX, &mouseY);

			// マウスポインタがある画面上の座標に該当する３Ｄ空間上の Near 面の座標を取得
			VECTOR startPos = ConvScreenPosToWorldPos(VGet((float)mouseX, (float)mouseY, 0.0f));

			// マウスポインタがある画面上の座標に該当する３Ｄ空間上の Far 面の座標を取得
			VECTOR endPos = ConvScreenPosToWorldPos(VGet((float)mouseX, (float)mouseY, 1.0f));

			// モデルと線分との当たり判定
			MV1_COLL_RESULT_POLY HitPoly = MV1CollCheck_Line(stageModelHandle, -1, startPos, endPos);

			// クリックした地点にマップがあれば、移動先をクリックした地点にする
			if(HitPoly.HitFlag == 1)
			{
				// 座標を表示
				DrawFormatString(5, 5, 65535, "%f, %f, %f", HitPoly.HitPosition.x, HitPoly.HitPosition.y, HitPoly.HitPosition.z);
				
				// 指定の２点の経路情報を探索する
				SetupPathPlanning(pathMove.nowPosition, HitPoly.HitPosition);

				// 探索した経路上を移動する準備を行う
				MoveInitialize();
			}
		}

		// １フレーム分経路上を移動
		MoveProcess();

		// ステージモデルを描画する
		MV1DrawModel(stageModelHandle);

		// 探索した経路のポリゴンの輪郭を描画する( デバッグ表示 )
		pUnit = pathPlanning.goalUnit;
		for(;;)
		{
			DrawTriangle3D(
				polyList.Vertexs[polyList.Polygons[pUnit->polyIndex].VIndex[0]].Position,
				polyList.Vertexs[polyList.Polygons[pUnit->polyIndex].VIndex[1]].Position,
				polyList.Vertexs[polyList.Polygons[pUnit->polyIndex].VIndex[2]].Position,
				GetColor(255, 0, 0),
				false
			);

			if(pUnit->prevPolyIndex == -1)
			{
				break;
			}

			pUnit = &pathPlanning.unitArray[pUnit->prevPolyIndex];
		}

		// 移動中の現在座標に球体を描画する
		DrawSphere3D(VAdd(pathMove.nowPosition, VGet(0.0f, 40.0f, 0.0f)), SPHERESIZE, 10, GetColor(255, 0, 0), GetColor(0, 0, 0), true);

		// 裏画面の内容を表画面に反映
		ScreenFlip();
	}

	// 経路情報の後始末
	TerminatePathPlanning();

	// ステージモデルのポリゴン同士の連結情報の後始末
	TerminatePolyLinkInfo();

	// DXライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
