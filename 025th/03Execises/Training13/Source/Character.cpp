#include "Character.h"
#include <math.h>
/**
* @file
* @brief Training13
* @author N.Yamada
* @date 2023/01/15
*
* @details クラス化その２
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

/**
* @fn Character::Initialize
* @brief キャラクターの初期化
*/
void Character::Initialize(int baseModelHandle, int shadowHandle, VECTOR position)
{
	// 初期座標は原点
	m_position = position;

	// 回転値は０
	m_angle = 0.0f;

	// ジャンプ力は初期状態では０
	m_jumpPower = 0.0f;

	// モデルハンドルの作成
	m_modelHandle = MV1DuplicateModel(baseModelHandle);

	// 画像ハンドル
	m_shadowHandle = shadowHandle;

	// 初期状態では「立ち止り」状態
	m_state = AnimeState::Neutral;
	PlayAnim();

	// 初期状態はＸ軸方向
	m_targetMoveDirection = VGet(1.0f, 0.0f, 0.0f);

	// アニメーションのブレンド率を初期化
	m_animBlendRate = 1.0f;

	// 初期状態ではアニメーションはアタッチされていないにする
	m_playAnim1 = -1;
	m_playAnim2 = -1;
}

/**
* @fn Character::Terminate
* @brief キャラクターの後始末
*/
void Character::Terminate()
{
	// モデルの削除
	MV1DeleteModel(m_modelHandle);
}

/**
* @fn Character::_Process
* @brief キャラクターの処理
*/
void Character::_Process(VECTOR moveVec, bool jumpFlag, int stageModelHandle)
{
	bool moveFlag;			// 移動したかどうかのフラグ( true:移動した  false:移動していない )

	// ルートフレームのＺ軸方向の移動パラメータを無効にする
	{
		MATRIX localMatrix;

		// ユーザー行列を解除する
		MV1ResetFrameUserLocalMatrix(m_modelHandle, 2);

		// 現在のルートフレームの行列を取得する
		localMatrix = MV1GetFrameLocalMatrix(m_modelHandle, 2);

		// Ｚ軸方向の平行移動成分を無効にする
		localMatrix.m[3][2] = 0.0f;

		// ユーザー行列として平行移動成分を無効にした行列をルートフレームにセットする
		MV1SetFrameUserLocalMatrix(m_modelHandle, 2, localMatrix);
	}

	// 移動したかどうかのフラグをセット、少しでも移動していたら「移動している」を表すtrueにする
	moveFlag = false;
	if(moveVec.x < -0.001f || moveVec.x > 0.001f
	|| moveVec.y < -0.001f || moveVec.y > 0.001f
	|| moveVec.z < -0.001f || moveVec.z > 0.001f)
	{
		moveFlag = true;
	}

	// キャラクターの状態が「ジャンプ」ではなく、且つジャンプフラグが立っていたらジャンプする
	if(m_state != AnimeState::Jump && jumpFlag == true)
	{
		// 状態を「ジャンプ」にする
		m_state = AnimeState::Jump;

		// Ｙ軸方向の速度をセット
		m_jumpPower = JUMP_POWER;

		// ジャンプアニメーションの再生
		PlayAnim();
	}

	// 移動ボタンが押されたかどうかで処理を分岐
	if(moveFlag)
	{
		// 移動ベクトルを正規化したものをキャラクターが向くべき方向として保存
		m_targetMoveDirection = VNorm(moveVec);

		// もし今まで「立ち止まり」状態だったら
		if(m_state == AnimeState::Neutral)
		{
			// 状態を「走り」にする
			m_state = AnimeState::Run;

			// 走りアニメーションを再生する
			PlayAnim();
		}
	}
	else
	{
		// このフレームで移動していなくて、且つ状態が「走り」だったら
		if(m_state == AnimeState::Run)
		{
			// 状態を「立ち止り」にする
			m_state = AnimeState::Neutral;

			// 立ち止りアニメーションを再生する
			PlayAnim();
		}
	}

	// 状態が「ジャンプ」の場合は
	if(m_state == AnimeState::Jump)
	{
		// Ｙ軸方向の速度を重力分減算する
		m_jumpPower -= GRAVITY;

		// 移動ベクトルのＹ成分をＹ軸方向の速度にする
		moveVec.y = m_jumpPower;
	}

	// キャラクターの移動方向にモデルの方向を近づける
	AngleProcess();

	// 移動ベクトルを元にコリジョンを考慮しつつキャラクターを移動
	Move(moveVec, stageModelHandle);

	// アニメーション処理
	AnimProcess();
}

/**
* @fn Character::Move
* @brief キャラクターの移動処理
*/
void Character::Move(VECTOR moveVector, int stageModelHandle)
{
	bool moveFlag;								// 水平方向に移動したかどうかのフラグ( false:移動していない  ture:移動した )
	bool hitFlag;								// ポリゴンに当たったかどうかを記憶しておくのに使う変数( false:当たっていない  true:当たった )
	MV1_COLL_RESULT_POLY_DIM hitDim;			// キャラクターの周囲にあるポリゴンを検出した結果が代入される当たり判定結果構造体
	int kabeNum;								// 壁ポリゴンと判断されたポリゴンの数
	int yukaNum;								// 床ポリゴンと判断されたポリゴンの数
	MV1_COLL_RESULT_POLY *kabe[MAX_HITCOLL];	// 壁ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *yuka[MAX_HITCOLL];	// 床ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *poly;					// ポリゴンの構造体にアクセスするために使用するポインタ( 使わなくても済ませられますがプログラムが長くなるので・・・ )
	HITRESULT_LINE lineRes;						// 線分とポリゴンとの当たり判定の結果を代入する構造体
	VECTOR oldPos;								// 移動前の座標	
	VECTOR nowPos;								// 移動後の座標

	// 移動前の座標を保存
	oldPos = m_position;

	// 移動後の座標を算出
	nowPos = VAdd(m_position, moveVector);

	// キャラクターの周囲にあるステージポリゴンを取得する
	// ( 検出する範囲は移動距離も考慮する )
	hitDim = MV1CollCheck_Sphere(stageModelHandle, -1, m_position, ENUM_DEFAULT_SIZE + VSize(moveVector));

	// x軸かy軸方向に 0.01f 以上移動した場合は「移動した」フラグを１にする
	if(fabs(moveVector.x) > 0.01f || fabs(moveVector.z) > 0.01f)
	{
		moveFlag = true;
	}
	else
	{
		moveFlag = false;
	}

	// 検出されたポリゴンが壁ポリゴン( ＸＺ平面に垂直なポリゴン )か床ポリゴン( ＸＺ平面に垂直ではないポリゴン )かを判断する
	{
		// 壁ポリゴンと床ポリゴンの数を初期化する
		kabeNum = 0;
		yukaNum = 0;

		// 検出されたポリゴンの数だけ繰り返し
		for(int i=0; i<hitDim.HitNum; i++)
		{
			// ＸＺ平面に垂直かどうかはポリゴンの法線のＹ成分が０に限りなく近いかどうかで判断する
			if(hitDim.Dim[i].Normal.y < 0.000001f && hitDim.Dim[i].Normal.y > -0.000001f)
			{
				// 壁ポリゴンと判断された場合でも、キャラクターのＹ座標＋１．０ｆより高いポリゴンのみ当たり判定を行う
				if(hitDim.Dim[i].Position[0].y > m_position.y + 1.0f ||
					hitDim.Dim[i].Position[1].y > m_position.y + 1.0f ||
					hitDim.Dim[i].Position[2].y > m_position.y + 1.0f)
				{
					// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
					if(kabeNum < MAX_HITCOLL)
					{
						// ポリゴンの構造体のアドレスを壁ポリゴンポインタ配列に保存する
						kabe[kabeNum] = &hitDim.Dim[i];

						// 壁ポリゴンの数を加算する
						kabeNum++;
					}
				}
			}
			else
			{
				// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
				if(yukaNum < MAX_HITCOLL)
				{
					// ポリゴンの構造体のアドレスを床ポリゴンポインタ配列に保存する
					yuka[yukaNum] = &hitDim.Dim[i];

					// 床ポリゴンの数を加算する
					yukaNum++;
				}
			}
		}
	}

	// 壁ポリゴンとの当たり判定処理
	if(kabeNum != 0)
	{
		// 壁に当たったかどうかのフラグは初期状態では「当たっていない」にしておく
		hitFlag = false;

		// 移動したかどうかで処理を分岐
		if(moveFlag)
		{
			// 壁ポリゴンの数だけ繰り返し
			for(int i=0; i<kabeNum; i++)
			{
				// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
				poly = kabe[i];

				// ポリゴンとキャラクターが当たっていなかったら次のカウントへ
				if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
				{
					continue;
				}

				// ここにきたらポリゴンとキャラクターが当たっているということなので、ポリゴンに当たったフラグを立てる
				hitFlag = true;

				// 壁に当たったら壁に遮られない移動成分分だけ移動する
				{
					VECTOR slideVec;	// キャラクターをスライドさせるベクトル

					// 進行方向ベクトルと壁ポリゴンの法線ベクトルに垂直なベクトルを算出
					slideVec = VCross(moveVector, poly->Normal);

					// 算出したベクトルと壁ポリゴンの法線ベクトルに垂直なベクトルを算出、これが
					// 元の移動成分から壁方向の移動成分を抜いたベクトル
					slideVec = VCross(poly->Normal, slideVec);

					// それを移動前の座標に足したものを新たな座標とする
					nowPos = VAdd(oldPos, slideVec);
				}

				// 新たな移動座標で壁ポリゴンと当たっていないかどうかを判定する
				int j;
				for(j=0; j<kabeNum; j++)
				{
					// j番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
					poly = kabe[j];

					// 当たっていたらループから抜ける
					if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
					{
						break;
					}
				}

				// j が kabeNum だった場合はどのポリゴンとも当たらなかったということなので
				// 壁に当たったフラグを倒した上でループから抜ける
				if(j == kabeNum)
				{
					hitFlag = false;
					break;
				}
			}
		}
		else
		{
			// 移動していない場合の処理

			// 壁ポリゴンの数だけ繰り返し
			for(int i=0; i<kabeNum; i++)
			{
				// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
				poly = kabe[i];

				// ポリゴンに当たっていたら当たったフラグを立てた上でループから抜ける
				if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
				{
					hitFlag = true;
					break;
				}
			}
		}

		// 壁に当たっていたら壁から押し出す処理を行う
		if(hitFlag)
		{
			// 壁からの押し出し処理を試みる最大数だけ繰り返し
			for(int k=0; k<HIT_TRYNUM; k++)
			{
				// 壁ポリゴンの数だけ繰り返し
				int i;
				for(i=0; i<kabeNum; i++)
				{
					// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
					poly = kabe[i];

					// キャラクターと当たっているかを判定
					if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
					{
						continue;
					}

					// 当たっていたら規定距離分キャラクターを壁の法線方向に移動させる
					nowPos = VAdd(nowPos, VScale(poly->Normal, HIT_SLIDE_LENGTH));

					// 移動した上で壁ポリゴンと接触しているかどうかを判定
					int j;
					for(j=0; j<kabeNum; j++)
					{
						// 当たっていたらループを抜ける
						poly = kabe[j];
						if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
						{
							break;
						}
					}

					// 全てのポリゴンと当たっていなかったらここでループ終了
					if(j == kabeNum)
					{
						break;
					}
				}

				// i が kabeNum ではない場合は全部のポリゴンで押し出しを試みる前に全ての壁ポリゴンと接触しなくなったということなのでループから抜ける
				if(i != kabeNum)
				{
					break;
				}
			}
		}
	}

	// 床ポリゴンとの当たり判定
	if(yukaNum != 0)
	{
		// ジャンプ中且つ上昇中の場合は処理を分岐
		if(m_state == AnimeState::Jump && m_jumpPower > 0.0f)
		{
			float minY;

			// 天井に頭をぶつける処理を行う

			// 一番低い天井にぶつける為の判定用変数を初期化
			minY = 0.0f;

			// 当たったかどうかのフラグを当たっていないを意味する０にしておく
			hitFlag = false;

			// 床ポリゴンの数だけ繰り返し
			for(int i=0; i<yukaNum; i++)
			{
				// i番目の床ポリゴンのアドレスを床ポリゴンポインタ配列から取得
				poly = yuka[i];

				// 足先から頭の高さまでの間でポリゴンと接触しているかどうかを判定
				lineRes = HitCheck_Line_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);

				// 接触していなかったら何もしない
				if(lineRes.HitFlag == false)
				{
					continue;
				}

				// 既にポリゴンに当たっていて、且つ今まで検出した天井ポリゴンより高い場合は何もしない
				if(hitFlag == true && minY < lineRes.Position.y)
				{
					continue;
				}

				// ポリゴンに当たったフラグを立てる
				hitFlag = true;

				// 接触したＹ座標を保存する
				minY = lineRes.Position.y;
			}

			// 接触したポリゴンがあったかどうかで処理を分岐
			if(hitFlag)
			{
				// 接触した場合はキャラクターのＹ座標を接触座標を元に更新
				nowPos.y = minY - HIT_HEIGHT;

				// Ｙ軸方向の速度は反転
				m_jumpPower = -m_jumpPower;
			}
		}
		else
		{
			float MaxY;

			// 下降中かジャンプ中ではない場合の処理

			// 床ポリゴンに当たったかどうかのフラグを倒しておく
			hitFlag = false;

			// 一番高い床ポリゴンにぶつける為の判定用変数を初期化
			MaxY = 0.0f;

			// 床ポリゴンの数だけ繰り返し
			for(int i=0; i<yukaNum; i++)
			{
				// i番目の床ポリゴンのアドレスを床ポリゴンポインタ配列から取得
				poly = yuka[i];

				// ジャンプ中かどうかで処理を分岐
				if(m_state == AnimeState::Jump)
				{
					// ジャンプ中の場合は頭の先から足先より少し低い位置の間で当たっているかを判定
					lineRes = HitCheck_Line_Triangle(VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), VAdd(nowPos, VGet(0.0f, -1.0f, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);
				}
				else
				{
					// 走っている場合は頭の先からそこそこ低い位置の間で当たっているかを判定( 傾斜で落下状態に移行してしまわない為 )
					lineRes = HitCheck_Line_Triangle(VAdd(nowPos, VGet(0.0f, HIT_HEIGHT, 0.0f)), VAdd(nowPos, VGet(0.0f, -40.0f, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);
				}

				// 当たっていなかったら何もしない
				if(lineRes.HitFlag == false)
				{
					continue;
				}

				// 既に当たったポリゴンがあり、且つ今まで検出した床ポリゴンより低い場合は何もしない
				if(hitFlag == true && MaxY > lineRes.Position.y)
				{
					continue;
				}

				// ポリゴンに当たったフラグを立てる
				hitFlag = true;

				// 接触したＹ座標を保存する
				MaxY = lineRes.Position.y;
			}

			// 床ポリゴンに当たったかどうかで処理を分岐
			if(hitFlag)
			{
				// 当たった場合

				// 接触したポリゴンで一番高いＹ座標をキャラクターのＹ座標にする
				nowPos.y = MaxY;

				// Ｙ軸方向の移動速度は０に
				m_jumpPower = 0.0f;

				// もしジャンプ中だった場合は着地状態にする
				if(m_state == AnimeState::Jump)
				{
					// 移動していたかどうかで着地後の状態と再生するアニメーションを分岐する
					if(moveFlag)
					{
						// 移動している場合は走り状態に
						m_state = AnimeState::Run;
					}
					else
					{
						// 移動していない場合は立ち止り状態に
						m_state = AnimeState::Neutral;
					}
					PlayAnim();

					// 着地時はアニメーションのブレンドは行わない
					m_animBlendRate = 1.0f;
				}
			}
			else
			{
				// 床コリジョンに当たっていなくて且つジャンプ状態ではなかった場合は
				if (m_state != AnimeState::Jump)
				{
					// ジャンプ中にする
					m_state = AnimeState::Jump;

					// ちょっとだけジャンプする
					m_jumpPower = FALL_UP_POWER;

					// アニメーションを再生する
					PlayAnim();
				}
			}
		}
	}

	// 新しい座標を保存する
	m_position = nowPos;

	// キャラクターのモデルの座標を更新する
	MV1SetPosition(m_modelHandle, m_position);

	// 検出したキャラクターの周囲のポリゴン情報を開放する
	MV1CollResultPolyDimTerminate(hitDim);
}

/**
* @fn Character::Collision
* @brief キャラクターに当たっていたら押し出す処理を行う( chkCh に ch が当たっていたら ch が離れる )
*/
void Character::Collision(VECTOR *chMoveVec, Character *chkCh)
{
	VECTOR chkChToChVec;
	VECTOR pushVec;
	VECTOR chPosition;
	float length;

	// 移動後の ch の座標を算出
	chPosition = VAdd(m_position, *chMoveVec);

	// 当たっていなかったら何もしない
	if (HitCheck_Capsule_Capsule(
		chPosition, VAdd(chPosition, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH,
		chkCh->m_position, VAdd(chkCh->m_position, VGet(0.0f, HIT_HEIGHT, 0.0f)), HIT_WIDTH) == 1)
	{
		// 当たっていたら ch が chk から離れる処理をする

		// chkCh から ch へのベクトルを算出
		chkChToChVec = VSub(chPosition, chkCh->m_position);

		// Ｙ軸は見ない
		chkChToChVec.y = 0.0f;

		// 二人の距離を算出
		length = VSize(chkChToChVec);

		// chkCh から ch へのベクトルを正規化( ベクトルの長さを 1.0f にする )
		pushVec = VScale(chkChToChVec, 1.0f / length);

		// 押し出す距離を算出、もし二人の距離から二人の大きさを引いた値に押し出し力を足して離れてしまう場合は、ぴったりくっつく距離に移動する
		if (length - HIT_WIDTH * 2.0f + HIT_PUSH_POWER > 0.0f)
		{
			float tempY;

			tempY = chPosition.y;
			chPosition = VAdd(chkCh->m_position, VScale(pushVec, HIT_WIDTH * 2.0f));

			// Ｙ座標は変化させない
			chPosition.y = tempY;
		}
		else
		{
			// 押し出し
			chPosition = VAdd(chPosition, VScale(pushVec, HIT_PUSH_POWER));
		}
	}

	// 当たり判定処理後の移動ベクトルをセット
	*chMoveVec = VSub(chPosition, m_position);
}

/**
* @fn Character::AngleProcess
* @brief キャラクターの向きを変える処理
*/
void Character::AngleProcess()
{
	float targetAngle;			// 目標角度
	float saAngle;				// 目標角度と現在の角度との差

	// 目標の方向ベクトルから角度値を算出する
	targetAngle = atan2f(m_targetMoveDirection.x, m_targetMoveDirection.z);

	// 目標の角度と現在の角度との差を割り出す
	{
		// 最初は単純に引き算
		saAngle = targetAngle - m_angle;

		// ある方向からある方向の差が１８０度以上になることは無いので
		// 差の値が１８０度以上になっていたら修正する
		if(saAngle < -DX_PI_F)
		{
			saAngle += DX_TWO_PI_F;
		}
		else if(saAngle > DX_PI_F)
		{
			saAngle -= DX_TWO_PI_F;
		}
	}

	// 角度の差が０に近づける
	if(saAngle > 0.0f)
	{
		// 差がプラスの場合は引く
		saAngle -= ANGLE_SPEED;
		if(saAngle < 0.0f)
		{
			saAngle = 0.0f;
		}
	}
	else
	{
		// 差がマイナスの場合は足す
		saAngle += ANGLE_SPEED;
		if(saAngle > 0.0f)
		{
			saAngle = 0.0f;
		}
	}

	// モデルの角度を更新
	m_angle = targetAngle - saAngle;
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle + DX_PI_F, 0.0f));
}

/**
* @fn Character::PlayAnim
* @brief キャラクターに新たなアニメーションを再生する
*/
void Character::PlayAnim()
{
	// 再生中のモーション２が有効だったらデタッチする
	if (m_playAnim2 != -1)
	{
		MV1DetachAnim(m_modelHandle, m_playAnim2);
		m_playAnim2 = -1;
	}

	// 今まで再生中のモーション１だったものの情報を２に移動する
	m_playAnim2 = m_playAnim1;
	m_animPlayCount2 = m_animPlayCount1;

	// 新たに指定のモーションをモデルにアタッチして、アタッチ番号を保存する
	m_playAnim1 = MV1AttachAnim(m_modelHandle, m_state);
	m_animPlayCount1 = 0.0f;

	// ブレンド率は再生中のモーション２が有効ではない場合は１．０ｆ( 再生中のモーション１が１００％の状態 )にする
	m_animBlendRate = m_playAnim2 == -1 ? 1.0f : 0.0f;
}

/**
* @fn Character::AnimProcess
* @brief キャラクターのアニメーション処理
*/
void Character::AnimProcess()
{
	float animTotalTime;		// 再生しているアニメーションの総時間

	// ブレンド率が１以下の場合は１に近づける
	if (m_animBlendRate < 1.0f)
	{
		m_animBlendRate += ANIM_BLEND_SPEED;
		if (m_animBlendRate > 1.0f)
		{
			m_animBlendRate = 1.0f;
		}
	}

	// 再生しているアニメーション１の処理
	if (m_playAnim1 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(m_modelHandle, m_playAnim1);

		// 再生時間を進める
		m_animPlayCount1 += PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if (m_animPlayCount1 >= animTotalTime)
		{
			m_animPlayCount1 = fmodf(m_animPlayCount1, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(m_modelHandle, m_playAnim1, m_animPlayCount1);

		// アニメーション１のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(m_modelHandle, m_playAnim1, m_animBlendRate);
	}

	// 再生しているアニメーション２の処理
	if (m_playAnim2 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(m_modelHandle, m_playAnim2);

		// 再生時間を進める
		m_animPlayCount2 += PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if (m_animPlayCount2 > animTotalTime)
		{
			m_animPlayCount2 = fmodf(m_animPlayCount2, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(m_modelHandle, m_playAnim2, m_animPlayCount2);

		// アニメーション２のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(m_modelHandle, m_playAnim2, 1.0f - m_animBlendRate);
	}
}

/**
* @fn Character::ShadowRender
* @brief キャラクターの影を描画
*/
void Character::ShadowRender(int stageModelHandle)
{
	MV1_COLL_RESULT_POLY_DIM hitResDim;
	MV1_COLL_RESULT_POLY *hitRes;
	VERTEX3D vertex[3];
	VECTOR slideVec;

	// ライティングを無効にする
	SetUseLighting(false);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(true);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// キャラクターの直下に存在する地面のポリゴンを取得
	hitResDim = MV1CollCheck_Capsule(stageModelHandle, -1, m_position, VAdd(m_position, VGet(0.0f, -SHADOW_HEIGHT, 0.0f)), SHADOW_SIZE);

	// 頂点データで変化が無い部分をセット
	vertex[0].dif = GetColorU8(255, 255, 255, 255);
	vertex[0].spc = GetColorU8(0, 0, 0, 0);
	vertex[0].su = 0.0f;
	vertex[0].sv = 0.0f;
	vertex[1] = vertex[0];
	vertex[2] = vertex[0];

	// 球の直下に存在するポリゴンの数だけ繰り返し
	hitRes = hitResDim.Dim;
	for (int i = 0; i < hitResDim.HitNum; i++, hitRes++)
	{
		// ポリゴンの座標は地面ポリゴンの座標
		vertex[0].pos = hitRes->Position[0];
		vertex[1].pos = hitRes->Position[1];
		vertex[2].pos = hitRes->Position[2];

		// ちょっと持ち上げて重ならないようにする
		slideVec = VScale(hitRes->Normal, 0.5f);
		vertex[0].pos = VAdd(vertex[0].pos, slideVec);
		vertex[1].pos = VAdd(vertex[1].pos, slideVec);
		vertex[2].pos = VAdd(vertex[2].pos, slideVec);

		// ポリゴンの不透明度を設定する
		vertex[0].dif.a = 0;
		vertex[1].dif.a = 0;
		vertex[2].dif.a = 0;
		if (hitRes->Position[0].y > m_position.y - SHADOW_HEIGHT)
			vertex[0].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[0].y - m_position.y) / SHADOW_HEIGHT));

		if (hitRes->Position[1].y > m_position.y - SHADOW_HEIGHT)
			vertex[1].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[1].y - m_position.y) / SHADOW_HEIGHT));

		if (hitRes->Position[2].y > m_position.y - SHADOW_HEIGHT)
			vertex[2].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[2].y - m_position.y) / SHADOW_HEIGHT));

		// ＵＶ値は地面ポリゴンとキャラクターの相対座標から割り出す
		vertex[0].u = (hitRes->Position[0].x - m_position.x) / (SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[0].v = (hitRes->Position[0].z - m_position.z) / (SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[1].u = (hitRes->Position[1].x - m_position.x) / (SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[1].v = (hitRes->Position[1].z - m_position.z) / (SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[2].u = (hitRes->Position[2].x - m_position.x) / (SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[2].v = (hitRes->Position[2].z - m_position.z) / (SHADOW_SIZE * 2.0f) + 0.5f;

		// 影ポリゴンを描画
		DrawPolygon3D(vertex, 1, m_shadowHandle, true);
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hitResDim);

	// ライティングを有効にする
	SetUseLighting(true);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(false);
}

/**
* @fn Character::Render
* @brief キャラクターのモデル描画
*/
void Character::Render()
{
	// モデルの描画
	MV1DrawModel(m_modelHandle);
}
