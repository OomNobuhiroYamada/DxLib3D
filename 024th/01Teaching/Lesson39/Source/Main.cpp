#include "DxLib.h"
#include "Input.h"
#include "Player.h"
#include "Stage.h"
#include "Camera.h"
#include <math.h>
/**
* @file
* @brief Lesson39
* @author N.Yamada
* @date 2023/01/09
*
* @details 3Dアクション基本3（プレイヤー以外のキャラクター）
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

void Render_Process();			//!< 描画処理

PADINPUT input;					//!< 入力情報の実体宣言
PLAYER player;					//!< プレイヤー情報の実体宣言
NOTPLAYER npc[NOTPLAYER_NUM];	//!< プレイヤーではないキャラの実体宣言
CHARA_COMMON charaCommon;		//!< キャラクターの共通情報の実体宣言
STAGE stage;					//!< ステージ情報の実体宣言
CAMERA camera;					//!< カメラ情報の実体宣言

/**
* @fn Input_Process
* @brief 入力処理
*/
void Input_Process()
{
	int old;

	// ひとつ前のフレームの入力を変数にとっておく
	old = input.nowInput;

	// 現在の入力状態を取得
	input.nowInput = GetJoypadInputState(DX_INPUT_KEY_PAD1);

	// 今のフレームで新たに押されたボタンのビットだけ立っている値を edgeInput に代入する
	input.edgeInput = input.nowInput & ~old;
}

/**
* @fn CharaCommon_Initialize
* @brief キャラクターの共通情報を初期化
*/
void CharaCommon_Initialize()
{
	// モデルの読み込み
	charaCommon.baseModelHandle = MV1LoadModel("Resource/DxChara.x");

	// 影描画用の画像の読み込み
	charaCommon.shadowHandle = LoadGraph("Resource/Shadow.tga");
}

/**
* @fn CharaCommon_Terminate
* @brief キャラクター共通情報の後始末
*/
void CharaCommon_Terminate()
{
	// モデルの削除
	MV1DeleteModel(charaCommon.baseModelHandle);

	// 影用画像の削除
	DeleteGraph(charaCommon.shadowHandle);
}

/**
* @fn Chara_Initialize
* @brief キャラクターの初期化
* @param[in] CHARA *ch, VECTOR position
*/
void Chara_Initialize(CHARA *ch, VECTOR position)
{
	// 初期座標は原点
	ch->position = position;

	// 回転値は０
	ch->angle = 0.0f;

	// ジャンプ力は初期状態では０
	ch->jumpPower = 0.0f;

	// モデルハンドルの作成
	ch->modelHandle = MV1DuplicateModel(charaCommon.baseModelHandle);

	// 初期状態では「立ち止り」状態
	ch->state = AnimeState::Neutral;
	Chara_PlayAnim(ch);

	// 初期状態はＸ軸方向
	ch->targetMoveDirection = VGet(1.0f, 0.0f, 0.0f);

	// アニメーションのブレンド率を初期化
	ch->animBlendRate = 1.0f;

	// 初期状態ではアニメーションはアタッチされていないにする
	ch->playAnim1 = -1;
	ch->playAnim2 = -1;
}

/**
* @fn Chara_Terminate
* @brief キャラクターの後始末
* @param[in] CHARA *ch
*/
void Chara_Terminate(CHARA *ch)
{
	// モデルの削除
	MV1DeleteModel(ch->modelHandle);
}

/**
* @fn Chara_Process
* @brief キャラクターの処理
* @param[in] CHARA *ch, VECTOR moveVec, bool jumpFlag
*/
void Chara_Process(CHARA *ch, VECTOR moveVec, bool jumpFlag)
{
	bool moveFlag;			// 移動したかどうかのフラグ( true:移動した  false:移動していない )

	// ルートフレームのＺ軸方向の移動パラメータを無効にする
	{
		MATRIX localMatrix;

		// ユーザー行列を解除する
		MV1ResetFrameUserLocalMatrix(ch->modelHandle, 2);

		// 現在のルートフレームの行列を取得する
		localMatrix = MV1GetFrameLocalMatrix(ch->modelHandle, 2);

		// Ｚ軸方向の平行移動成分を無効にする
		localMatrix.m[3][2] = 0.0f;

		// ユーザー行列として平行移動成分を無効にした行列をルートフレームにセットする
		MV1SetFrameUserLocalMatrix(ch->modelHandle, 2, localMatrix);
	}

	// 移動したかどうかのフラグをセット、少しでも移動していたら「移動している」を表すtrueにする
	moveFlag = false;
	if(moveVec.x < -0.001f || moveVec.x > 0.001f ||
		moveVec.y < -0.001f || moveVec.y > 0.001f ||
		moveVec.z < -0.001f || moveVec.z > 0.001f)
	{
		moveFlag = true;
	}

	// キャラクターの状態が「ジャンプ」ではなく、且つジャンプフラグが立っていたらジャンプする
	if(ch->state != AnimeState::Jump && jumpFlag == true)
	{
		// 状態を「ジャンプ」にする
		ch->state = AnimeState::Jump;

		// Ｙ軸方向の速度をセット
		ch->jumpPower = CHARA_JUMP_POWER;

		// ジャンプアニメーションの再生
		Chara_PlayAnim(ch);
	}

	// 移動ボタンが押されたかどうかで処理を分岐
	if(moveFlag)
	{
		// 移動ベクトルを正規化したものをキャラクターが向くべき方向として保存
		ch->targetMoveDirection = VNorm(moveVec);

		// もし今まで「立ち止まり」状態だったら
		if(ch->state == AnimeState::Neutral)
		{
			// 状態を「走り」にする
			ch->state = AnimeState::Run;

			// 走りアニメーションを再生する
			Chara_PlayAnim(ch);
		}
	}
	else
	{
		// このフレームで移動していなくて、且つ状態が「走り」だったら
		if(ch->state == AnimeState::Run)
		{
			// 状態を「立ち止り」にする
			ch->state = AnimeState::Neutral;

			// 立ち止りアニメーションを再生する
			Chara_PlayAnim(ch);
		}
	}

	// 状態が「ジャンプ」の場合は
	if(ch->state == AnimeState::Jump)
	{
		// Ｙ軸方向の速度を重力分減算する
		ch->jumpPower -= CHARA_GRAVITY;

		// 移動ベクトルのＹ成分をＹ軸方向の速度にする
		moveVec.y = ch->jumpPower;
	}

	// キャラクターの移動方向にモデルの方向を近づける
	Chara_AngleProcess(ch);

	// 移動ベクトルを元にコリジョンを考慮しつつキャラクターを移動
	Chara_Move(ch, moveVec);

	// アニメーション処理
	Chara_AnimProcess(ch);
}

/**
* @fn Chara_Move
* @brief キャラクターの移動処理
* @param[in] CHARA *ch, VECTOR moveVector
*/
void Chara_Move(CHARA *ch, VECTOR moveVector)
{
	bool moveFlag;									// 水平方向に移動したかどうかのフラグ( false:移動していない  ture:移動した )
	bool hitFlag;									// ポリゴンに当たったかどうかを記憶しておくのに使う変数( false:当たっていない  true:当たった )
	MV1_COLL_RESULT_POLY_DIM hitDim;				// キャラクターの周囲にあるポリゴンを検出した結果が代入される当たり判定結果構造体
	int kabeNum;									// 壁ポリゴンと判断されたポリゴンの数
	int yukaNum;									// 床ポリゴンと判断されたポリゴンの数
	MV1_COLL_RESULT_POLY *kabe[CHARA_MAX_HITCOLL];	// 壁ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *yuka[CHARA_MAX_HITCOLL];	// 床ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *poly;						// ポリゴンの構造体にアクセスするために使用するポインタ( 使わなくても済ませられますがプログラムが長くなるので・・・ )
	HITRESULT_LINE lineRes;							// 線分とポリゴンとの当たり判定の結果を代入する構造体
	VECTOR oldPos;									// 移動前の座標	
	VECTOR nowPos;									// 移動後の座標

	// 移動前の座標を保存
	oldPos = ch->position;

	// 移動後の座標を算出
	nowPos = VAdd(ch->position, moveVector);

	// キャラクターの周囲にあるステージポリゴンを取得する
	// ( 検出する範囲は移動距離も考慮する )
	hitDim = MV1CollCheck_Sphere(stage.modelHandle, -1, ch->position, CHARA_ENUM_DEFAULT_SIZE + VSize(moveVector));

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
				if(hitDim.Dim[i].Position[0].y > ch->position.y + 1.0f ||
					hitDim.Dim[i].Position[1].y > ch->position.y + 1.0f ||
					hitDim.Dim[i].Position[2].y > ch->position.y + 1.0f)
				{
					// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
					if(kabeNum < CHARA_MAX_HITCOLL)
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
				if(yukaNum < CHARA_MAX_HITCOLL)
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
				if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
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
					if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
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
				if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
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
			for(int k=0; k<CHARA_HIT_TRYNUM; k++)
			{
				// 壁ポリゴンの数だけ繰り返し
				int i;
				for(i=0; i<kabeNum; i++)
				{
					// i番目の壁ポリゴンのアドレスを壁ポリゴンポインタ配列から取得
					poly = kabe[i];

					// キャラクターと当たっているかを判定
					if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
					{
						continue;
					}

					// 当たっていたら規定距離分キャラクターを壁の法線方向に移動させる
					nowPos = VAdd(nowPos, VScale(poly->Normal, CHARA_HIT_SLIDE_LENGTH));

					// 移動した上で壁ポリゴンと接触しているかどうかを判定
					int j;
					for(j=0; j<kabeNum; j++)
					{
						// 当たっていたらループを抜ける
						poly = kabe[j];
						if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == 1)
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
		if(ch->state == AnimeState::Jump && ch->jumpPower > 0.0f)
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
				lineRes = HitCheck_Line_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);

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
				nowPos.y = minY - CHARA_HIT_HEIGHT;

				// Ｙ軸方向の速度は反転
				ch->jumpPower = -ch->jumpPower;
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
				if(ch->state == AnimeState::Jump)
				{
					// ジャンプ中の場合は頭の先から足先より少し低い位置の間で当たっているかを判定
					lineRes = HitCheck_Line_Triangle(VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), VAdd(nowPos, VGet(0.0f, -1.0f, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);
				}
				else
				{
					// 走っている場合は頭の先からそこそこ低い位置の間で当たっているかを判定( 傾斜で落下状態に移行してしまわない為 )
					lineRes = HitCheck_Line_Triangle(VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), VAdd(nowPos, VGet(0.0f, -40.0f, 0.0f)), poly->Position[0], poly->Position[1], poly->Position[2]);
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
				ch->jumpPower = 0.0f;

				// もしジャンプ中だった場合は着地状態にする
				if(ch->state == AnimeState::Jump)
				{
					// 移動していたかどうかで着地後の状態と再生するアニメーションを分岐する
					if(moveFlag)
					{
						// 移動している場合は走り状態に
						ch->state = AnimeState::Run;
						Chara_PlayAnim(ch);
					}
					else
					{
						// 移動していない場合は立ち止り状態に
						ch->state = AnimeState::Neutral;
						Chara_PlayAnim(ch);
					}

					// 着地時はアニメーションのブレンドは行わない
					ch->animBlendRate = 1.0f;
				}
			}
			else
			{
				// 床コリジョンに当たっていなくて且つジャンプ状態ではなかった場合は
				if(ch->state != AnimeState::Jump)
				{
					// ジャンプ中にする
					ch->state = AnimeState::Jump;

					// ちょっとだけジャンプする
					ch->jumpPower = CHARA_FALL_UP_POWER;

					// アニメーションを再生する
					Chara_PlayAnim(ch);
				}
			}
		}
	}

	// 新しい座標を保存する
	ch->position = nowPos;

	// キャラクターのモデルの座標を更新する
	MV1SetPosition(ch->modelHandle, ch->position);

	// 検出したキャラクターの周囲のポリゴン情報を開放する
	MV1CollResultPolyDimTerminate(hitDim);
}

/**
* @fn Chara_Collision
* @brief キャラクターに当たっていたら押し出す処理を行う( chkCh に ch が当たっていたら ch が離れる )
* @param[in] CHARA *ch, VECTOR *chMoveVec, CHARA *chkCh
*/
void Chara_Collision(CHARA *ch, VECTOR *chMoveVec, CHARA *chkCh)
{
	VECTOR chkChToChVec;
	VECTOR pushVec;
	VECTOR chPosition;
	float length;

	// 移動後の ch の座標を算出
	chPosition = VAdd(ch->position, *chMoveVec);

	// 当たっていなかったら何もしない
	if(HitCheck_Capsule_Capsule(
		chPosition, VAdd(chPosition, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH,
		chkCh->position, VAdd(chkCh->position, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH) == 1)
	{
		// 当たっていたら ch が chk から離れる処理をする

		// chkCh から ch へのベクトルを算出
		chkChToChVec = VSub(chPosition, chkCh->position);

		// Ｙ軸は見ない
		chkChToChVec.y = 0.0f;

		// 二人の距離を算出
		length = VSize(chkChToChVec);

		// chkCh から ch へのベクトルを正規化( ベクトルの長さを 1.0f にする )
		pushVec = VScale(chkChToChVec, 1.0f / length);

		// 押し出す距離を算出、もし二人の距離から二人の大きさを引いた値に押し出し力を足して離れてしまう場合は、ぴったりくっつく距離に移動する
		if(length - CHARA_HIT_WIDTH * 2.0f + CHARA_HIT_PUSH_POWER > 0.0f)
		{
			float tempY;

			tempY = chPosition.y;
			chPosition = VAdd(chkCh->position, VScale(pushVec, CHARA_HIT_WIDTH * 2.0f));

			// Ｙ座標は変化させない
			chPosition.y = tempY;
		}
		else
		{
			// 押し出し
			chPosition = VAdd(chPosition, VScale(pushVec, CHARA_HIT_PUSH_POWER));
		}
	}

	// 当たり判定処理後の移動ベクトルをセット
	*chMoveVec = VSub(chPosition, ch->position);
}

/**
* @fn Chara_AngleProcess
* @brief キャラクターの向きを変える処理
* @param[in] CHARA *ch
*/
void Chara_AngleProcess(CHARA *ch)
{
	float targetAngle;			// 目標角度
	float saAngle;				// 目標角度と現在の角度との差

	// 目標の方向ベクトルから角度値を算出する
	targetAngle = atan2f(ch->targetMoveDirection.x, ch->targetMoveDirection.z);

	// 目標の角度と現在の角度との差を割り出す
	{
		// 最初は単純に引き算
		saAngle = targetAngle - ch->angle;

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
		saAngle -= CHARA_ANGLE_SPEED;
		if(saAngle < 0.0f)
		{
			saAngle = 0.0f;
		}
	}
	else
	{
		// 差がマイナスの場合は足す
		saAngle += CHARA_ANGLE_SPEED;
		if(saAngle > 0.0f)
		{
			saAngle = 0.0f;
		}
	}

	// モデルの角度を更新
	ch->angle = targetAngle - saAngle;
	MV1SetRotationXYZ(ch->modelHandle, VGet(0.0f, ch->angle + DX_PI_F, 0.0f));
}

/**
* @fn Chara_PlayAnim
* @brief キャラクターに新たなアニメーションを再生する
* @param[in] CHARA *ch
*/
void Chara_PlayAnim(CHARA *ch)
{
	// 再生中のモーション２が有効だったらデタッチする
	if(ch->playAnim2 != -1)
	{
		MV1DetachAnim(ch->modelHandle, ch->playAnim2);
		ch->playAnim2 = -1;
	}

	// 今まで再生中のモーション１だったものの情報を２に移動する
	ch->playAnim2 = ch->playAnim1;
	ch->animPlayCount2 = ch->animPlayCount1;

	// 新たに指定のモーションをモデルにアタッチして、アタッチ番号を保存する
	ch->playAnim1 = MV1AttachAnim(ch->modelHandle, ch->state);
	ch->animPlayCount1 = 0.0f;

	// ブレンド率は再生中のモーション２が有効ではない場合は１．０ｆ( 再生中のモーション１が１００％の状態 )にする
	ch->animBlendRate = ch->playAnim2 == -1 ? 1.0f : 0.0f;
}

/**
* @fn Chara_AnimProcess
* @brief キャラクターのアニメーション処理
* @param[in] CHARA *ch
*/
void Chara_AnimProcess(CHARA *ch)
{
	float animTotalTime;		// 再生しているアニメーションの総時間

	// ブレンド率が１以下の場合は１に近づける
	if(ch->animBlendRate < 1.0f)
	{
		ch->animBlendRate += CHARA_ANIM_BLEND_SPEED;
		if(ch->animBlendRate > 1.0f)
		{
			ch->animBlendRate = 1.0f;
		}
	}

	// 再生しているアニメーション１の処理
	if(ch->playAnim1 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(ch->modelHandle, ch->playAnim1);

		// 再生時間を進める
		ch->animPlayCount1 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if(ch->animPlayCount1 >= animTotalTime)
		{
			ch->animPlayCount1 = fmodf(ch->animPlayCount1, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(ch->modelHandle, ch->playAnim1, ch->animPlayCount1);

		// アニメーション１のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(ch->modelHandle, ch->playAnim1, ch->animBlendRate);
	}

	// 再生しているアニメーション２の処理
	if(ch->playAnim2 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(ch->modelHandle, ch->playAnim2);

		// 再生時間を進める
		ch->animPlayCount2 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if(ch->animPlayCount2 > animTotalTime)
		{
			ch->animPlayCount2 = fmodf(ch->animPlayCount2, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(ch->modelHandle, ch->playAnim2, ch->animPlayCount2);

		// アニメーション２のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(ch->modelHandle, ch->playAnim2, 1.0f - ch->animBlendRate);
	}
}

/**
* @fn Chara_ShadowRender
* @brief キャラクターの影を描画
* @param[in] CHARA *ch
*/
void Chara_ShadowRender(CHARA *ch)
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
	hitResDim = MV1CollCheck_Capsule(stage.modelHandle, -1, ch->position, VAdd(ch->position, VGet(0.0f, -CHARA_SHADOW_HEIGHT, 0.0f)), CHARA_SHADOW_SIZE);

	// 頂点データで変化が無い部分をセット
	vertex[0].dif = GetColorU8(255, 255, 255, 255);
	vertex[0].spc = GetColorU8(0, 0, 0, 0);
	vertex[0].su = 0.0f;
	vertex[0].sv = 0.0f;
	vertex[1] = vertex[0];
	vertex[2] = vertex[0];

	// 球の直下に存在するポリゴンの数だけ繰り返し
	hitRes = hitResDim.Dim;
	for(int i=0; i<hitResDim.HitNum; i++, hitRes++)
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
		if(hitRes->Position[0].y > ch->position.y - CHARA_SHADOW_HEIGHT)
			vertex[0].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[0].y - ch->position.y) / CHARA_SHADOW_HEIGHT));

		if(hitRes->Position[1].y > ch->position.y - CHARA_SHADOW_HEIGHT)
			vertex[1].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[1].y - ch->position.y) / CHARA_SHADOW_HEIGHT));

		if(hitRes->Position[2].y > ch->position.y - CHARA_SHADOW_HEIGHT)
			vertex[2].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[2].y - ch->position.y) / CHARA_SHADOW_HEIGHT));

		// ＵＶ値は地面ポリゴンとキャラクターの相対座標から割り出す
		vertex[0].u = (hitRes->Position[0].x - ch->position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[0].v = (hitRes->Position[0].z - ch->position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[1].u = (hitRes->Position[1].x - ch->position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[1].v = (hitRes->Position[1].z - ch->position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[2].u = (hitRes->Position[2].x - ch->position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
		vertex[2].v = (hitRes->Position[2].z - ch->position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;

		// 影ポリゴンを描画
		DrawPolygon3D(vertex, 1, charaCommon.shadowHandle, true);
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hitResDim);

	// ライティングを有効にする
	SetUseLighting(true);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(false);
}


/**
* @fn Player_Initialize
* @brief プレイヤーの初期化
*/
void Player_Initialize()
{
	// キャラクター情報を初期化
	Chara_Initialize(&player.charaInfo, VGet(0.0f, 0.0f, 0.0f));
}

/**
* @fn Player_Terminate
* @brief プレイヤーの後始末
*/
void Player_Terminate()
{
	// キャラクター情報の後始末
	Chara_Terminate(&player.charaInfo);
}

/**
* @fn Player_Process
* @brief プレイヤーの処理
*/
void Player_Process()
{
	VECTOR upMoveVec;	// 方向ボタン「↑」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR leftMoveVec;	// 方向ボタン「←」を入力をしたときのプレイヤーの移動方向ベクトル
	VECTOR moveVec;		// このフレームの移動ベクトル
	bool jumpFlag;		// ジャンプフラグ

	// プレイヤーの移動方向のベクトルを算出
	{
		// 方向ボタン「↑」を押したときのプレイヤーの移動ベクトルはカメラの視線方向からＹ成分を抜いたもの
		upMoveVec = VSub(camera.target, camera.eye);
		upMoveVec.y = 0.0f;

		// 方向ボタン「←」を押したときのプレイヤーの移動ベクトルは上を押したときの方向ベクトルとＹ軸のプラス方向のベクトルに垂直な方向
		leftMoveVec = VCross(upMoveVec, VGet(0.0f, 1.0f, 0.0f));

		// 二つのベクトルを正規化( ベクトルの長さを１．０にすること )
		upMoveVec = VNorm(upMoveVec);
		leftMoveVec = VNorm(leftMoveVec);
	}

	// このフレームでの移動ベクトルを初期化
	moveVec = VGet(0.0f, 0.0f, 0.0f);

	// ジャンプフラグを倒す
	jumpFlag = false;

	// パッドの３ボタンと左シフトがどちらも押されていなかったらプレイヤーの移動処理
	if(CheckHitKey(KEY_INPUT_LSHIFT) == 0 && (input.nowInput & PAD_INPUT_C) == 0)
	{
		// 方向ボタン「←」が入力されたらカメラの見ている方向から見て左方向に移動する
		if(input.nowInput & PAD_INPUT_LEFT)
		{
			// 移動ベクトルに「←」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, leftMoveVec);
		}
		else
		{
			// 方向ボタン「→」が入力されたらカメラの見ている方向から見て右方向に移動する
			if(input.nowInput & PAD_INPUT_RIGHT)
			{
				// 移動ベクトルに「←」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(leftMoveVec, -1.0f));
			}
		}

		// 方向ボタン「↑」が入力されたらカメラの見ている方向に移動する
		if(input.nowInput & PAD_INPUT_UP)
		{
			// 移動ベクトルに「↑」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, upMoveVec);
		}
		else
		{
			// 方向ボタン「↓」が入力されたらカメラの方向に移動する
			if(input.nowInput & PAD_INPUT_DOWN)
			{
				// 移動ベクトルに「↑」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(upMoveVec, -1.0f));
			}
		}

		// ボタン１が押されていたらジャンプフラグを立てる
		if(input.edgeInput & PAD_INPUT_A)
		{
			jumpFlag = true;
		}
	}

	// 移動方向を移動速度でスケーリングする
	moveVec = VScale(moveVec, CHARA_MOVE_SPEED);

	// プレイヤーキャラ以外との当たり判定を行う
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		Chara_Collision(&player.charaInfo, &moveVec, &npc[i].charaInfo);
	}

	// キャラクターを動作させる処理を行う
	Chara_Process(&player.charaInfo, moveVec, jumpFlag);
}

/**
* @fn NotPlayer_Initialize
* @brief プレイヤー以外キャラの初期化
*/
void NotPlayer_Initialize()
{
	static VECTOR firstPosition[NOTPLAYER_NUM] =
	{
		{ -3000.0f, 0.0f, 2300.0f },
		{ -2500.0f, 7300.0f, -2500.0f },
		{ -2600.0f, 0.0f, -3100.0f },
		{  2800.0f, 0.0f, 200.0f },
	};

	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		// キャラクター情報を初期化
		Chara_Initialize(&npc[i].charaInfo, firstPosition[i]);

		// 移動時間を初期化
		npc[i].moveTime = 0;

		// 移動方向を初期化
		npc[i].moveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;
	}
}

/**
* @fn NotPlayer_Terminate
* @brief プレイヤー以外キャラの後始末
*/
void NotPlayer_Terminate()
{
	// キャラクタの数だけ繰り返し
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		// キャラクター情報の後始末
		Chara_Terminate(&npc[i].charaInfo);
	}
}

/**
* @fn NotPlayer_Process
* @brief プレイヤー以外キャラの処理
*/
void NotPlayer_Process()
{
	VECTOR moveVec;
	bool jumpFlag;

	// キャラクタの数だけ繰り返し
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		// ジャンプフラグを倒しておく
		jumpFlag = false;

		// 一定時間が経過したら移動する方向を変更する
		npc[i].moveTime++;
		if(npc[i].moveTime >= NOTPLAYER_MOVETIME)
		{
			npc[i].moveTime = 0;

			// 新しい方向の決定
			npc[i].moveAngle = GetRand(1000) * DX_PI_F * 2.0f / 1000.0f;

			// 一定確率でジャンプする
			if(GetRand(1000) < NOTPLAYER_JUMPRATIO)
			{
				jumpFlag = true;
			}
		}

		// 新しい方向の角度からベクトルを算出
		moveVec.x = cosf(npc[i].moveAngle) * CHARA_MOVE_SPEED;
		moveVec.y = 0.0f;
		moveVec.z = sinf(npc[i].moveAngle) * CHARA_MOVE_SPEED;

		// プレイヤーとの当たり判定を行う
		Chara_Collision(&npc[i].charaInfo, &moveVec, &player.charaInfo);

		// 自分以外のプレイヤーキャラとの当たり判定を行う
		for(int j=0; j<NOTPLAYER_NUM; j++)
		{
			// 自分との当たり判定はしない
			if(i == j)
			{
				continue;
			}

			Chara_Collision(&npc[i].charaInfo, &moveVec, &npc[j].charaInfo);
		}

		// 移動処理を行う
		Chara_Process(&npc[i].charaInfo, moveVec, jumpFlag);
	}
}

/**
* @fn Stage_Initialize
* @brief ステージの初期化処理
*/
void Stage_Initialize()
{
	// ステージモデルの読み込み
	stage.modelHandle = MV1LoadModel("Resource/ColTestStage.mqo");

	// モデル全体のコリジョン情報のセットアップ
	MV1SetupCollInfo(stage.modelHandle, -1);
}

/**
* @fn Stage_Terminate
* @brief ステージの後始末処理
*/
void Stage_Terminate()
{
	// ステージモデルの後始末
	MV1DeleteModel(stage.modelHandle);
}

/**
* @fn Camera_Initialize
* @brief カメラの初期化処理
*/
void Camera_Initialize()
{
	// カメラの初期水平角度は１８０度
	camera.angleH = DX_PI_F;

	// 垂直角度は０度
	camera.angleV = 0.0f;
}

/**
* @fn Camera_Process
* @brief カメラの処理
*/
void Camera_Process()
{
	// パッドの３ボタンか、シフトキーが押されている場合のみ角度変更操作を行う
	if(CheckHitKey(KEY_INPUT_LSHIFT) || (input.nowInput & PAD_INPUT_C))
	{
		// 「←」ボタンが押されていたら水平角度をマイナスする
		if(input.nowInput & PAD_INPUT_LEFT)
		{
			camera.angleH -= CAMERA_ANGLE_SPEED;

			// －１８０度以下になったら角度値が大きくなりすぎないように３６０度を足す
			if(camera.angleH < -DX_PI_F)
			{
				camera.angleH += DX_TWO_PI_F;
			}
		}

		// 「→」ボタンが押されていたら水平角度をプラスする
		if(input.nowInput & PAD_INPUT_RIGHT)
		{
			camera.angleH += CAMERA_ANGLE_SPEED;

			// １８０度以上になったら角度値が大きくなりすぎないように３６０度を引く
			if(camera.angleH > DX_PI_F)
			{
				camera.angleH -= DX_TWO_PI_F;
			}
		}

		// 「↑」ボタンが押されていたら垂直角度をマイナスする
		if(input.nowInput & PAD_INPUT_UP)
		{
			camera.angleV -= CAMERA_ANGLE_SPEED;

			// ある一定角度以下にはならないようにする
			if(camera.angleV < -DX_PI_F / 2.0f + 0.6f)
			{
				camera.angleV = -DX_PI_F / 2.0f + 0.6f;
			}
		}

		// 「↓」ボタンが押されていたら垂直角度をプラスする
		if(input.nowInput & PAD_INPUT_DOWN)
		{
			camera.angleV += CAMERA_ANGLE_SPEED;

			// ある一定角度以上にはならないようにする
			if(camera.angleV > DX_PI_F / 2.0f - 0.6f)
			{
				camera.angleV = DX_PI_F / 2.0f - 0.6f;
			}
		}
	}

	// カメラの注視点はプレイヤー座標から規定値分高い座標
	camera.target = VAdd(player.charaInfo.position, VGet(0.0f, CAMERA_PLAYER_TARGET_HEIGHT, 0.0f));

	// カメラの座標を決定する
	{
		MATRIX rotZ, rotY;
		float cameraPlayerLength;
		MV1_COLL_RESULT_POLY_DIM hRes;
		int hitNum;

		// 水平方向の回転はＹ軸回転
		rotY = MGetRotY(camera.angleH);

		// 垂直方向の回転はＺ軸回転 )
		rotZ = MGetRotZ(camera.angleV);

		// カメラからプレイヤーまでの初期距離をセット
		cameraPlayerLength = CAMERA_PLAYER_LENGTH;

		// カメラの座標を算出
		// Ｘ軸にカメラとプレイヤーとの距離分だけ伸びたベクトルを
		// 垂直方向回転( Ｚ軸回転 )させたあと水平方向回転( Ｙ軸回転 )して更に
		// 注視点の座標を足したものがカメラの座標
		camera.eye = VAdd(VTransform(VTransform(VGet(-cameraPlayerLength, 0.0f, 0.0f), rotZ), rotY), camera.target);

		// 注視点からカメラの座標までの間にステージのポリゴンがあるか調べる
		hRes = MV1CollCheck_Capsule(stage.modelHandle, -1, camera.target, camera.eye, CAMERA_COLLISION_SIZE);
		hitNum = hRes.HitNum;
		MV1CollResultPolyDimTerminate(hRes);
		if(hitNum != 0)
		{
			float notHitLength;
			float hitLength;
			float testLength;
			VECTOR testPosition;

			// あったら無い位置までプレイヤーに近づく

			// ポリゴンに当たらない距離をセット
			notHitLength = 0.0f;

			// ポリゴンに当たる距離をセット
			hitLength = cameraPlayerLength;
			do
			{
				// 当たるかどうかテストする距離をセット( 当たらない距離と当たる距離の中間 )
				testLength = notHitLength + (hitLength - notHitLength) / 2.0f;

				// テスト用のカメラ座標を算出
				testPosition = VAdd(VTransform(VTransform(VGet(-testLength, 0.0f, 0.0f), rotZ), rotY), camera.target);

				// 新しい座標で壁に当たるかテスト
				hRes = MV1CollCheck_Capsule(stage.modelHandle, -1, camera.target, testPosition, CAMERA_COLLISION_SIZE);
				hitNum = hRes.HitNum;
				MV1CollResultPolyDimTerminate(hRes);
				if(hitNum != 0)
				{
					// 当たったら当たる距離を testLength に変更する
					hitLength = testLength;
				}
				else
				{
					// 当たらなかったら当たらない距離を testLength に変更する
					notHitLength = testLength;
				}

				// hitLength と NoHitLength が十分に近づいていなかったらループ
			} while(hitLength - notHitLength > 0.1f);

			// カメラの座標をセット
			camera.eye = testPosition;
		}
	}

	// カメラの情報をライブラリのカメラに反映させる
	SetCameraPositionAndTarget_UpVecY(camera.eye, camera.target);
}

/**
* @fn Render_Process
* @brief 描画処理
*/
void Render_Process()
{
	// ステージモデルの描画
	MV1DrawModel(stage.modelHandle);

	// プレイヤーモデルの描画
	MV1DrawModel(player.charaInfo.modelHandle);

	// プレイヤー以外キャラモデルの描画
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		MV1DrawModel(npc[i].charaInfo.modelHandle);
	}

	// プレイヤーの影の描画
	Chara_ShadowRender(&player.charaInfo);

	// プレイヤー以外キャラの影の描画
	for(int i=0; i<NOTPLAYER_NUM; i++)
	{
		Chara_ShadowRender(&npc[i].charaInfo);
	}
}

/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウインドウモードで起動
	ChangeWindowMode(true);

	// ライブラリの初期化
	if(DxLib_Init() < 0)
	{
		return -1;
	}

	// キャラクターの共通情報を初期化
	CharaCommon_Initialize();

	// プレイヤーの初期化
	Player_Initialize();

	// プレイヤー以外キャラの初期化
	NotPlayer_Initialize();

	// ステージの初期化
	Stage_Initialize();

	// カメラの初期化
	Camera_Initialize();

	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	// ＥＳＣキーが押されるか、ウインドウが閉じられるまでループ
	while(ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 現在のカウントを取得する
		int time = GetNowCount();

		// 画面をクリア
		ClearDrawScreen();

		// 入力処理
		Input_Process();

		// プレイヤー以外キャラの処理
		NotPlayer_Process();

		// プレイヤーの処理
		Player_Process();

		// カメラの処理
		Camera_Process();

		// 描画処理
		Render_Process();

		// 裏画面の内容を表画面に反映
		ScreenFlip();

		// １７ミリ秒(約秒間６０フレームだった時の１フレームあたりの経過時間)
		// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
		}
	}

	// プレイヤー以外キャラの後始末
	NotPlayer_Terminate();

	// プレイヤーの後始末
	Player_Terminate();

	// キャラクター共通情報の後始末
	CharaCommon_Terminate();

	// ステージの後始末
	Stage_Terminate();

	// ライブラリの後始末
	DxLib_End();

	// ソフト終了
	return 0;
}
