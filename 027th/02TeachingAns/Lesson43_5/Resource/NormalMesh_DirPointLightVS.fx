// 頂点シェーダーの入力
struct VS_INPUT
{
	float4 Position        : POSITION ;     // 座標( ローカル空間 )
	float3 Normal          : NORMAL0 ;      // 法線( ローカル空間 )
	float4 Diffuse         : COLOR0 ;       // ディフューズカラー
	float4 Specular        : COLOR1 ;       // スペキュラカラー
	float4 TexCoords0      : TEXCOORD0 ;	// テクスチャ座標
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : POSITION ;
	float4 Diffuse         : COLOR0 ;
	float4 Specular        : COLOR1 ;
	float2 TexCoords0      : TEXCOORD0 ;
} ;

// マテリアルパラメータ
struct VS_CONST_MATERIAL
{
	float4 Diffuse ;                // マテリアルディフューズカラー
	float4 Specular ;               // マテリアルスペキュラカラー
	float4 Power ;                  // マテリアルスペキュラハイライトの強さ
} ;

// ライトパラメータ
struct VS_CONST_LIGHT
{
	float4 Position ;               // 座標( ビュー空間 )
	float3 Direction ;              // 方向( ビュー空間 )
	float4 Diffuse ;                // ディフューズカラー
	float4 Specular ;               // スペキュラカラー
	float4 Ambient ;                // アンビエントカラーとマテリアルのアンビエントカラーを乗算したもの
	float4 Range_FallOff_AT0_AT1 ;  // x:有効距離  y:スポットライト用FallOff  z:距離による減衰処理用パラメータ０  w:距離による減衰処理用パラメータ１
	float4 AT2_SpotP0_SpotP1 ;      // x:距離による減衰処理用パラメータ２  y:スポットライト用パラメータ０( cos( Phi / 2.0f ) )  z:スポットライト用パラメータ１( 1.0f / ( cos( Theta / 2.0f ) - cos( Phi / 2.0f ) ) )
} ;



// C++ 側で設定する定数の定義
float4              cfAmbient_Emissive        : register( c1  ) ;		// マテリアルエミッシブカラー + マテリアルアンビエントカラー * グローバルアンビエントカラー
float4              cfProjectionMatrix[ 4 ]   : register( c2  ) ;		// ビュー　　→　射影行列
float4              cfViewMatrix[ 3 ]         : register( c6  ) ;		// ワールド　→　ビュー行列
float4              cfTextureMatrix[ 3 ][ 2 ] : register( c88 ) ;		// テクスチャ座標操作用行列
float4              cfLocalWorldMatrix[ 3 ]   : register( c94 ) ;		// ローカル　→　ワールド行列
VS_CONST_MATERIAL   cfMaterial                : register( c11 ) ;		// マテリアルパラメータ
VS_CONST_LIGHT      cfLight[ 2 ]              : register( c14 ) ;		// ディレクショナルライトとポイントライトのパラメータ


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;
	float3 lWorldNrm ;
	float3 lViewNrm ;
	float3 lLightHalfVec ;
	float4 lLightLitParam ;
	float4 lLightLitDest ;
	float3 lLightDir ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightGen ;
	float4 lTotalDiffuse ;
	float4 lTotalSpecular ;


	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ローカル座標をワールド座標に変換
	lWorldPosition.x = dot( VSInput.Position, cfLocalWorldMatrix[ 0 ] ) ;
	lWorldPosition.y = dot( VSInput.Position, cfLocalWorldMatrix[ 1 ] ) ;
	lWorldPosition.z = dot( VSInput.Position, cfLocalWorldMatrix[ 2 ] ) ;
	lWorldPosition.w = 1.0f ;

	// ワールド座標をビュー座標に変換
	lViewPosition.x = dot( lWorldPosition, cfViewMatrix[ 0 ] ) ;
	lViewPosition.y = dot( lWorldPosition, cfViewMatrix[ 1 ] ) ;
	lViewPosition.z = dot( lWorldPosition, cfViewMatrix[ 2 ] ) ;
	lViewPosition.w = 1.0f ;

	// ビュー座標を射影座標に変換
	VSOutput.Position.x = dot( lViewPosition, cfProjectionMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( lViewPosition, cfProjectionMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( lViewPosition, cfProjectionMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( lViewPosition, cfProjectionMatrix[ 3 ] ) ;

	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )



	// ライトの処理 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ディフューズカラーとスペキュラカラーの蓄積値の初期化
	lTotalDiffuse  = float4( 0, 0, 0, 0 ) ;
	lTotalSpecular = float4( 0, 0, 0, 0 ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 開始 )

	// ローカルベクトルをワールドベクトルに変換
	lWorldNrm.x = dot( VSInput.Normal, cfLocalWorldMatrix[ 0 ].xyz ) ;
	lWorldNrm.y = dot( VSInput.Normal, cfLocalWorldMatrix[ 1 ].xyz ) ;
	lWorldNrm.z = dot( VSInput.Normal, cfLocalWorldMatrix[ 2 ].xyz ) ;

	// ワールドベクトルをビューベクトルに変換
	lViewNrm.x = dot( lWorldNrm, cfViewMatrix[ 0 ].xyz ) ;
	lViewNrm.y = dot( lWorldNrm, cfViewMatrix[ 1 ].xyz ) ;
	lViewNrm.z = dot( lWorldNrm, cfViewMatrix[ 2 ].xyz ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 終了 )




	// ディレクショナルライトの処理 *****************************************************( 開始 )

	// ライトの方向セット
	lLightDir = cfLight[ 0 ].Direction ;


	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 開始 )

	// 法線とライトの逆方向ベクトルとの内積を lLightLitParam.x にセット
	lLightLitParam.x = dot( lViewNrm, -lLightDir ) ;

	// ハーフベクトルの計算 norm( ( norm( 頂点位置から視点へのベクトル ) + ライトの方向 ) )
	lLightHalfVec = normalize( normalize( -lViewPosition.xyz ) - lLightDir ) ;

	// 法線とハーフベクトルの内積を lLightLitParam.y にセット
	lLightLitParam.y = dot( lLightHalfVec, lViewNrm ) ;

	// スペキュラ反射率を lLightLitParam.w にセット
	lLightLitParam.w = cfMaterial.Power.x ;

	// ライト計算
	lLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;

	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 終了 )


	// カラー計算 ===========================================================( 開始 )

	// ディフューズライト蓄積値 += ディフューズ角度減衰計算結果 * マテリアルディフューズカラー * ライトのディフューズカラー + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの
	lTotalDiffuse += lLightLitDest.y * cfLight[ 0 ].Diffuse * cfMaterial.Diffuse + cfLight[ 0 ].Ambient ;

	// スペキュラライト蓄積値 += スペキュラ角度減衰計算結果 * ライトのスペキュラカラー
	lTotalSpecular += lLightLitDest.z * cfLight[ 0 ].Specular ;

	// カラー計算 ===========================================================( 終了 )

	// ディレクショナルライトの処理 *****************************************************( 終了 )




	// ポイントライトの処理 *************************************************************( 開始 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( lViewPosition.xyz - cfLight[ 1 ].Position.xyz ) ;


	// 距離減衰値計算 =======================================================( 開始 )

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = lViewPosition.xyz - cfLight[ 1 ].Position.xyz ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( cfLight[ 1 ].Range_FallOff_AT0_AT1.z + cfLight[ 1 ].Range_FallOff_AT0_AT1.w * sqrt( lLightDistancePow2 ) + cfLight[ 1 ].AT2_SpotP0_SpotP1.x * lLightDistancePow2 ) ;

	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, cfLight[ 1 ].Range_FallOff_AT0_AT1.x ) ;

	// 距離減衰値計算 =======================================================( 終了 )


	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 開始 )

	// 法線とライトの逆方向ベクトルとの内積を lLightLitParam.x にセット
	lLightLitParam.x = dot( lViewNrm, -lLightDir ) ;

	// ハーフベクトルの計算 norm( ( norm( 頂点位置から視点へのベクトル ) + ライトの方向 ) )
	lLightHalfVec = normalize( normalize( -lViewPosition.xyz ) - lLightDir ) ;

	// 法線とハーフベクトルの内積を lLightLitParam.y にセット
	lLightLitParam.y = dot( lLightHalfVec, lViewNrm ) ;

	// スペキュラ反射率を lLightLitParam.w にセット
	lLightLitParam.w = cfMaterial.Power.x ;

	// ライト計算
	lLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;

	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 終了 )


	// カラー計算 ===========================================================( 開始 )

	// ディフーズライト蓄積値 += 距離・スポットライト角度減衰値 * ( ディフーズ角度減衰計算結果 * マテリアルディフューズカラー * ライトのディフーズカラー + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの )
	lTotalDiffuse += lLightGen * ( lLightLitDest.y * cfLight[ 1 ].Diffuse * cfMaterial.Diffuse + cfLight[ 1 ].Ambient ) ;

	// スペキュラライト蓄積値 += スペキュラ角度減衰計算結果 * 距離・スポットライト減衰 * ライトのスペキュラカラー
	lTotalSpecular += lLightGen * lLightLitDest.z * cfLight[ 1 ].Specular ;

	// カラー計算 ===========================================================( 終了 )

	// ポイントライトの処理 *************************************************************( 終了 )


	// ライトの処理 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )




	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ディフューズカラー = ディフューズライト蓄積値 + マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの
	VSOutput.Diffuse = lTotalDiffuse + cfAmbient_Emissive ;

	// ディフューズアルファはマテリアルのディフューズカラーのアルファをそのまま使う
	VSOutput.Diffuse.w = cfMaterial.Diffuse.w ;

	// スペキュラカラー = スペキュラライト蓄積値 * マテリアルのスペキュラカラー
	VSOutput.Specular = lTotalSpecular * cfMaterial.Specular ;


	// テクスチャ座標変換行列による変換を行った結果のテクスチャ座標をセット
	VSOutput.TexCoords0.x = dot( VSInput.TexCoords0, cfTextureMatrix[ 0 ][ 0 ] ) ;
	VSOutput.TexCoords0.y = dot( VSInput.TexCoords0, cfTextureMatrix[ 0 ][ 1 ] ) ;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return VSOutput ;
}

