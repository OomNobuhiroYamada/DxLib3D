#include "DxLib.h"
#include "Input.h"
#include "Player.h"
#include "Stage.h"
#include "Camera.h"
#include <math.h>
/**
* @file
* @brief Lesson38
* @author N.Yamada
* @date 2023/01/09
*
* @details 3Dアクション基本2（追加コリジョンモデル）
* @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

void Render_Process();			//!< 描画処理

PADINPUT input;					//!< 入力情報の実体宣言
PLAYER player;					//!< プレイヤー情報の実体宣言
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
* @fn Player_Initialize
* @brief プレイヤーの初期化
*/
void Player_Initialize()
{
	// 初期座標は原点
	player.position = VGet(0.0f, 0.0f, 0.0f);

	// 回転値は０
	player.angle = 0.0f;

	// ジャンプ力は初期状態では０
	player.jumpPower = 0.0f;

	// モデルの読み込み
	player.modelHandle = MV1LoadModel("Resource/DxChara.x");

	// 影描画用の画像の読み込み
	player.shadowHandle = LoadGraph("Resource/Shadow.tga");

	// 初期状態では「立ち止り」状態
	player.state = AnimeState::Neutral;
	Player_PlayAnim(player.state);

	// 初期状態でプレイヤーが向くべき方向はＸ軸方向
	player.targetMoveDirection = VGet(1.0f, 0.0f, 0.0f);

	// アニメーションのブレンド率を初期化
	player.animBlendRate = 1.0f;

	// 初期状態ではアニメーションはアタッチされていないにする
	player.playAnim1 = -1;
	player.playAnim2 = -1;
}

/**
* @fn Player_Terminate
* @brief プレイヤーの後始末
*/
void Player_Terminate()
{
	// モデルの削除
	MV1DeleteModel(player.modelHandle);

	// 影用画像の削除
	DeleteGraph(player.shadowHandle);
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
	bool moveFlag;		// 移動したかどうかのフラグ( true:移動した  false:移動していない )

	// ルートフレームのＺ軸方向の移動パラメータを無効にする
	{
		MATRIX localMatrix;

		// ユーザー行列を解除する
		MV1ResetFrameUserLocalMatrix(player.modelHandle, 2);

		// 現在のルートフレームの行列を取得する
		localMatrix = MV1GetFrameLocalMatrix(player.modelHandle, 2);

		// Ｚ軸方向の平行移動成分を無効にする
		localMatrix.m[3][2] = 0.0f;

		// ユーザー行列として平行移動成分を無効にした行列をルートフレームにセットする
		MV1SetFrameUserLocalMatrix(player.modelHandle, 2, localMatrix);
	}

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

	// 移動したかどうかのフラグを初期状態では「移動していない」を表すfalseにする
	moveFlag = false;

	// パッドの３ボタンと左シフトがどちらも押されていなかったらプレイヤーの移動処理
	if(CheckHitKey(KEY_INPUT_LSHIFT) == 0 && (input.nowInput & PAD_INPUT_C) == 0)
	{
		// 方向ボタン「←」が入力されたらカメラの見ている方向から見て左方向に移動する
		if(input.nowInput & PAD_INPUT_LEFT)
		{
			// 移動ベクトルに「←」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, leftMoveVec);

			// 移動したかどうかのフラグを「移動した」にする
			moveFlag = true;
		}
		else
		{
			// 方向ボタン「→」が入力されたらカメラの見ている方向から見て右方向に移動する
			if(input.nowInput & PAD_INPUT_RIGHT)
			{
				// 移動ベクトルに「←」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(leftMoveVec, -1.0f));

				// 移動したかどうかのフラグを「移動した」にする
				moveFlag = true;
			}
		}

		// 方向ボタン「↑」が入力されたらカメラの見ている方向に移動する
		if(input.nowInput & PAD_INPUT_UP)
		{
			// 移動ベクトルに「↑」が入力された時の移動ベクトルを加算する
			moveVec = VAdd(moveVec, upMoveVec);

			// 移動したかどうかのフラグを「移動した」にする
			moveFlag = true;
		}
		else
		{
			// 方向ボタン「↓」が入力されたらカメラの方向に移動する
			if(input.nowInput & PAD_INPUT_DOWN)
			{
				// 移動ベクトルに「↑」が入力された時の移動ベクトルを反転したものを加算する
				moveVec = VAdd(moveVec, VScale(upMoveVec, -1.0f));

				// 移動したかどうかのフラグを「移動した」にする
				moveFlag = true;
			}
		}

		// プレイヤーの状態が「ジャンプ」ではなく、且つボタン１が押されていたらジャンプする
		if(player.state != AnimeState::Jump && (input.edgeInput & PAD_INPUT_A))
		{
			// 状態を「ジャンプ」にする
			player.state = AnimeState::Jump;

			// Ｙ軸方向の速度をセット
			player.jumpPower = CHARA_JUMP_POWER;

			// ジャンプアニメーションの再生
			Player_PlayAnim(player.state);
		}
	}

	// 移動ボタンが押されたかどうかで処理を分岐
	if(moveFlag)
	{
		// 移動ベクトルを正規化したものをプレイヤーが向くべき方向として保存
		player.targetMoveDirection = VNorm(moveVec);

		// プレイヤーが向くべき方向ベクトルをプレイヤーのスピード倍したものを移動ベクトルとする
		moveVec = VScale(player.targetMoveDirection, CHARA_MOVE_SPEED);

		// もし今まで「立ち止まり」状態だったら
		if(player.state == AnimeState::Neutral)
		{
			// 状態を「走り」にする
			player.state = AnimeState::Run;

			// 走りアニメーションを再生する
			Player_PlayAnim(player.state);
		}
	}
	else
	{
		// このフレームで移動していなくて、且つ状態が「走り」だったら
		if(player.state == AnimeState::Run)
		{
			// 状態を「立ち止り」にする
			player.state = AnimeState::Neutral;

			// 立ち止りアニメーションを再生する
			Player_PlayAnim(player.state);
		}
	}

	// 状態が「ジャンプ」の場合は
	if(player.state == AnimeState::Jump)
	{
		// Ｙ軸方向の速度を重力分減算する
		player.jumpPower -= CHARA_GRAVITY;

		// 移動ベクトルのＹ成分をＹ軸方向の速度にする
		moveVec.y = player.jumpPower;
	}

	// プレイヤーの移動方向にモデルの方向を近づける
	Player_AngleProcess();

	// 移動ベクトルを元にコリジョンを考慮しつつプレイヤーを移動
	Player_Move(moveVec);

	// アニメーション処理
	Player_AnimProcess();
}

/**
* @fn Player_Move
* @brief プレイヤーの移動処理
* @param[in] VECTOR moveVector
*/
void Player_Move(VECTOR moveVector)
{
	bool moveFlag;												// 水平方向に移動したかどうかのフラグ( false:移動していない  true:移動した )
	bool hitFlag;												// ポリゴンに当たったかどうかを記憶しておくのに使う変数( false:当たっていない  true:当たった )
	MV1_COLL_RESULT_POLY_DIM hitDim[STAGECOLLOBJ_MAXNUM + 1];	// プレイヤーの周囲にあるポリゴンを検出した結果が代入される当たり判定結果構造体
	int hitDimNum;												// hitDim の有効な配列要素数
	int kabeNum;												// 壁ポリゴンと判断されたポリゴンの数
	int yukaNum;												// 床ポリゴンと判断されたポリゴンの数
	MV1_COLL_RESULT_POLY *kabe[CHARA_MAX_HITCOLL];				// 壁ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *yuka[CHARA_MAX_HITCOLL];				// 床ポリゴンと判断されたポリゴンの構造体のアドレスを保存しておくためのポインタ配列
	MV1_COLL_RESULT_POLY *poly;									// ポリゴンの構造体にアクセスするために使用するポインタ( 使わなくても済ませられますがプログラムが長くなるので・・・ )
	HITRESULT_LINE lineRes;										// 線分とポリゴンとの当たり判定の結果を代入する構造体
	VECTOR oldPos;												// 移動前の座標	
	VECTOR nowPos;												// 移動後の座標

	// 移動前の座標を保存
	oldPos = player.position;

	// 移動後の座標を算出
	nowPos = VAdd(player.position, moveVector);

	// プレイヤーの周囲にあるステージポリゴンを取得する
	// ( 検出する範囲は移動距離も考慮する )
	hitDim[0] = MV1CollCheck_Sphere(stage.modelHandle, -1, player.position, CHARA_ENUM_DEFAULT_SIZE + VSize(moveVector));

	// プレイヤーの周囲にあるコリジョンオブジェクトのポリゴンも取得する
	for(int i=0; i<stage.collObjNum; i++)
	{
		hitDim[i + 1] = MV1CollCheck_Sphere(stage.collObjModelHandle[i], -1, player.position, CHARA_ENUM_DEFAULT_SIZE + VSize(moveVector));
	}

	// hitDim の有効な数はコリジョンオブジェクトの数とステージ自体のコリジョン
	hitDimNum = stage.collObjNum + 1;

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
		for(int j=0; j<hitDimNum; j++)
		{
			for(int i=0; i<hitDim[j].HitNum; i++)
			{
				// ＸＺ平面に垂直かどうかはポリゴンの法線のＹ成分が０に限りなく近いかどうかで判断する
				if(hitDim[j].Dim[i].Normal.y < 0.000001f && hitDim[j].Dim[i].Normal.y > -0.000001f)
				{
					// 壁ポリゴンと判断された場合でも、プレイヤーのＹ座標＋１．０ｆより高いポリゴンのみ当たり判定を行う
					if(hitDim[j].Dim[i].Position[0].y > player.position.y + 1.0f ||
						hitDim[j].Dim[i].Position[1].y > player.position.y + 1.0f ||
						hitDim[j].Dim[i].Position[2].y > player.position.y + 1.0f)
					{
						// ポリゴンの数が列挙できる限界数に達していなかったらポリゴンを配列に追加
						if(kabeNum < CHARA_MAX_HITCOLL)
						{
							// ポリゴンの構造体のアドレスを壁ポリゴンポインタ配列に保存する
							kabe[kabeNum] = &hitDim[j].Dim[i];

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
						yuka[yukaNum] = &hitDim[j].Dim[i];

						// 床ポリゴンの数を加算する
						yukaNum++;
					}
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

				// ポリゴンとプレイヤーが当たっていなかったら次のカウントへ
				if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
				{
					continue;
				}

				// ここにきたらポリゴンとプレイヤーが当たっているということなので、ポリゴンに当たったフラグを立てる
				hitFlag = true;

				// 壁に当たったら壁に遮られない移動成分分だけ移動する
				{
					VECTOR slideVec;	// プレイヤーをスライドさせるベクトル

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
					hitFlag = 1;
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

					// プレイヤーと当たっているかを判定
					if(HitCheck_Capsule_Triangle(nowPos, VAdd(nowPos, VGet(0.0f, CHARA_HIT_HEIGHT, 0.0f)), CHARA_HIT_WIDTH, poly->Position[0], poly->Position[1], poly->Position[2]) == false)
					{
						continue;
					}

					// 当たっていたら規定距離分プレイヤーを壁の法線方向に移動させる
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
		if(player.state == AnimeState::Jump && player.jumpPower > 0.0f)
		{
			float MinY;

			// 天井に頭をぶつける処理を行う

			// 一番低い天井にぶつける為の判定用変数を初期化
			MinY = 0.0f;

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
				if(hitFlag == true && MinY < lineRes.Position.y)
				{
					continue;
				}

				// ポリゴンに当たったフラグを立てる
				hitFlag = true;

				// 接触したＹ座標を保存する
				MinY = lineRes.Position.y;
			}

			// 接触したポリゴンがあったかどうかで処理を分岐
			if(hitFlag)
			{
				// 接触した場合はプレイヤーのＹ座標を接触座標を元に更新
				nowPos.y = MinY - CHARA_HIT_HEIGHT;

				// Ｙ軸方向の速度は反転
				player.jumpPower = -player.jumpPower;
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
				if(player.state == AnimeState::Jump)
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

				// 接触したポリゴンで一番高いＹ座標をプレイヤーのＹ座標にする
				nowPos.y = MaxY;

				// Ｙ軸方向の移動速度は０に
				player.jumpPower = 0.0f;

				// もしジャンプ中だった場合は着地状態にする
				if(player.state == AnimeState::Jump)
				{
					// 移動していたかどうかで着地後の状態と再生するアニメーションを分岐する
					if(moveFlag)
					{
						// 移動している場合は走り状態に
						player.state = AnimeState::Run;
						Player_PlayAnim(player.state);
					}
					else
					{
						// 移動していない場合は立ち止り状態に
						player.state = AnimeState::Neutral;
						Player_PlayAnim(player.state);
					}

					// 着地時はアニメーションのブレンドは行わない
					player.animBlendRate = 1.0f;
				}
			}
			else
			{
				// 床コリジョンに当たっていなくて且つジャンプ状態ではなかった場合は
				if(player.state != AnimeState::Jump)
				{
					// ジャンプ中にする
					player.state = AnimeState::Jump;

					// ちょっとだけジャンプする
					player.jumpPower = CHARA_FALL_UP_POWER;

					// アニメーションを再生する
					Player_PlayAnim(player.state);
				}
			}
		}
	}

	// 新しい座標を保存する
	player.position = nowPos;

	// プレイヤーのモデルの座標を更新する
	MV1SetPosition(player.modelHandle, player.position);

	// 検出したプレイヤーの周囲のポリゴン情報を開放する
	for(int i=0; i<hitDimNum; i++)
	{
		MV1CollResultPolyDimTerminate(hitDim[i]);
	}
}

/**
* @fn Player_AngleProcess
* @brief プレイヤーの向きを変える処理
*/
void Player_AngleProcess()
{
	float targetAngle;			// 目標角度
	float saAngle;				// 目標角度と現在の角度との差

	// 目標の方向ベクトルから角度値を算出する
	targetAngle = atan2f(player.targetMoveDirection.x, player.targetMoveDirection.z);

	// 目標の角度と現在の角度との差を割り出す
	{
		// 最初は単純に引き算
		saAngle = targetAngle - player.angle;

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
	player.angle = targetAngle - saAngle;
	MV1SetRotationXYZ(player.modelHandle, VGet(0.0f, player.angle + DX_PI_F, 0.0f));
}

/**
* @fn Player_PlayAnim
* @brief プレイヤーに新たなアニメーションを再生する
* @param[in] int animIndex
*/
void Player_PlayAnim(int animIndex)
{
	// 再生中のモーション２が有効だったらデタッチする
	if(player.playAnim2 != -1)
	{
		MV1DetachAnim(player.modelHandle, player.playAnim2);
		player.playAnim2 = -1;
	}

	// 今まで再生中のモーション１だったものの情報を２に移動する
	player.playAnim2 = player.playAnim1;
	player.animPlayCount2 = player.animPlayCount1;

	// 新たに指定のモーションをモデルにアタッチして、アタッチ番号を保存する
	player.playAnim1 = MV1AttachAnim(player.modelHandle, animIndex);
	player.animPlayCount1 = 0.0f;

	// ブレンド率は再生中のモーション２が有効ではない場合は１．０ｆ( 再生中のモーション１が１００％の状態 )にする
	player.animBlendRate = player.playAnim2 == -1 ? 1.0f : 0.0f;
}

/**
* @fn Player_AnimProcess
* @brief プレイヤーのアニメーション処理
*/
void Player_AnimProcess()
{
	float animTotalTime;		// 再生しているアニメーションの総時間

	// ブレンド率が１以下の場合は１に近づける
	if(player.animBlendRate < 1.0f)
	{
		player.animBlendRate += CHARA_ANIM_BLEND_SPEED;
		if(player.animBlendRate > 1.0f)
		{
			player.animBlendRate = 1.0f;
		}
	}

	// 再生しているアニメーション１の処理
	if(player.playAnim1 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(player.modelHandle, player.playAnim1);

		// 再生時間を進める
		player.animPlayCount1 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if(player.animPlayCount1 >= animTotalTime)
		{
			player.animPlayCount1 = fmodf(player.animPlayCount1, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(player.modelHandle, player.playAnim1, player.animPlayCount1);

		// アニメーション１のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(player.modelHandle, player.playAnim1, player.animBlendRate);
	}

	// 再生しているアニメーション２の処理
	if(player.playAnim2 != -1)
	{
		// アニメーションの総時間を取得
		animTotalTime = MV1GetAttachAnimTotalTime(player.modelHandle, player.playAnim2);

		// 再生時間を進める
		player.animPlayCount2 += CHARA_PLAY_ANIM_SPEED;

		// 再生時間が総時間に到達していたら再生時間をループさせる
		if(player.animPlayCount2 > animTotalTime)
		{
			player.animPlayCount2 = fmodf(player.animPlayCount2, animTotalTime);
		}

		// 変更した再生時間をモデルに反映させる
		MV1SetAttachAnimTime(player.modelHandle, player.playAnim2, player.animPlayCount2);

		// アニメーション２のモデルに対する反映率をセット
		MV1SetAttachAnimBlendRate(player.modelHandle, player.playAnim2, 1.0f - player.animBlendRate);
	}
}

/**
* @fn Player_ShadowRender
* @brief プレイヤーの影を描画
*/
void Player_ShadowRender()
{
	MV1_COLL_RESULT_POLY_DIM hitResDim;
	MV1_COLL_RESULT_POLY *hitRes;
	VERTEX3D vertex[3];
	VECTOR slideVec;
	int modelHandle;

	// ライティングを無効にする
	SetUseLighting(false);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(true);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 影を落とすモデルの数だけ繰り返し
	for(int j=0; j<stage.collObjNum+1; j++)
	{
		// チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル
		if(j == 0)
		{
			modelHandle = stage.modelHandle;
		}
		else
		{
			modelHandle = stage.collObjModelHandle[j - 1];
		}

		// プレイヤーの直下に存在する地面のポリゴンを取得
		hitResDim = MV1CollCheck_Capsule(modelHandle, -1, player.position, VAdd(player.position, VGet(0.0f, -CHARA_SHADOW_HEIGHT, 0.0f)), CHARA_SHADOW_SIZE);

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
			if(hitRes->Position[0].y > player.position.y - CHARA_SHADOW_HEIGHT)
			{
				vertex[0].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[0].y - player.position.y) / CHARA_SHADOW_HEIGHT));
			}

			if(hitRes->Position[1].y > player.position.y - CHARA_SHADOW_HEIGHT)
			{
				vertex[1].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[1].y - player.position.y) / CHARA_SHADOW_HEIGHT));
			}

			if(hitRes->Position[2].y > player.position.y - CHARA_SHADOW_HEIGHT)
			{
				vertex[2].dif.a = (BYTE)(128 * (1.0f - fabs(hitRes->Position[2].y - player.position.y) / CHARA_SHADOW_HEIGHT));
			}

			// ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			vertex[0].u = (hitRes->Position[0].x - player.position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
			vertex[0].v = (hitRes->Position[0].z - player.position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
			vertex[1].u = (hitRes->Position[1].x - player.position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
			vertex[1].v = (hitRes->Position[1].z - player.position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
			vertex[2].u = (hitRes->Position[2].x - player.position.x) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;
			vertex[2].v = (hitRes->Position[2].z - player.position.z) / (CHARA_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(vertex, 1, player.shadowHandle, true);
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hitResDim);
	}

	// ライティングを有効にする
	SetUseLighting(true);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(false);
}

/**
* @fn Stage_Initialize
* @brief ステージの初期化処理
*/
void Stage_Initialize()
{
	// ステージモデルの読み込み
	stage.modelHandle = MV1LoadModel("Resource/ColTestStage.mqo");

	// コリジョンモデルの派生元ハンドルの読み込み
	stage.collObjBaseModelHandle = MV1LoadModel("Resource/ColTestObj.mqo");

	// ステージに配置しているコリジョンモデルの数を０にする
	stage.collObjNum = 0;

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

	// ステージに配置したコリジョンモデルの後始末
	for(int i=0; i<stage.collObjNum; i++)
	{
		MV1DeleteModel(stage.collObjModelHandle[i]);
	}
}

/**
* @fn Stage_AddCollObj
* @brief コリジョンオブジェクトをステージに追加する
* @param[in] VECTOR position
*/
void Stage_AddCollObj(VECTOR position)
{
	int newObj;

	// 追加するコリジョンオブジェクトが使用する配列の番号をセット
	newObj = stage.collObjNum;

	// 追加するコリジョンモデルを派生元ハンドルから生成させる
	stage.collObjModelHandle[newObj] = MV1DuplicateModel(stage.collObjBaseModelHandle);

	// 座標をセット
	MV1SetPosition(stage.collObjModelHandle[newObj], position);

	// コリジョンオブジェクトの数を増やす
	stage.collObjNum++;
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
	camera.target = VAdd(player.position, VGet(0.0f, CAMERA_PLAYER_TARGET_HEIGHT, 0.0f));

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
		hitNum = 0;

		// 最初はステージ自体と判定
		hRes = MV1CollCheck_Capsule(stage.modelHandle, -1, camera.target, camera.eye, CAMERA_COLLISION_SIZE);
		hitNum += hRes.HitNum;
		MV1CollResultPolyDimTerminate(hRes);

		// ステージのポリゴンは周囲に無かったら今度はコリジョンオブジェクトのポリゴンが周囲にあるか調べる
		if(hitNum == 0)
		{
			for(int i=0; i<stage.collObjNum; i++)
			{
				hRes = MV1CollCheck_Capsule(stage.collObjModelHandle[i], -1, camera.target, camera.eye, CAMERA_COLLISION_SIZE);
				hitNum += hRes.HitNum;
				MV1CollResultPolyDimTerminate(hRes);

				// 周囲にあったらその時点でループから抜ける
				if(hitNum != 0)
				{
					break;
				}
			}
		}

		// ポリゴンが周囲にあったら当たり判定処理
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

				// 最初はステージ自体と
				hRes = MV1CollCheck_Capsule(stage.modelHandle, -1, camera.target, testPosition, CAMERA_COLLISION_SIZE);
				hitNum = hRes.HitNum;
				MV1CollResultPolyDimTerminate(hRes);

				// ステージ自体と当たっていなかったら今度はコリジョンオブジェクトと
				if(hitNum == 0)
				{
					for(int i=0; i<stage.collObjNum; i++)
					{
						hRes = MV1CollCheck_Capsule(stage.collObjModelHandle[i], -1, camera.target, testPosition, CAMERA_COLLISION_SIZE);
						hitNum += hRes.HitNum;
						MV1CollResultPolyDimTerminate(hRes);

						// 当たっていたらその時点でループから抜ける
						if(hitNum != 0)
						{
							break;
						}
					}
				}

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

	// コリジョンモデルの描画
	for(int i=0; i<stage.collObjNum; i++)
	{
		MV1DrawModel(stage.collObjModelHandle[i]);
	}

	// プレイヤーモデルの描画
	MV1DrawModel(player.modelHandle);

	// プレイヤーの影の描画
	Player_ShadowRender();
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

	// プレイヤーの初期化
	Player_Initialize();

	// ステージの初期化
	Stage_Initialize();

	// カメラの初期化
	Camera_Initialize();

	// ステージにコリジョンモデルを幾つか追加する
	Stage_AddCollObj(VGet(2900.0f, 0.0f, -120.0f));
	Stage_AddCollObj(VGet(-2900.0f, 0.0f, -120.0f));
	Stage_AddCollObj(VGet(1600.0f, 2200.0f, 2700.0f));
	Stage_AddCollObj(VGet(-2600.0f, 1600.0f, 2400.0f));

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

	// プレイヤーの後始末
	Player_Terminate();

	// ステージの後始末
	Stage_Terminate();

	// ライブラリの後始末
	DxLib_End();

	// ソフト終了
	return 0;
}
