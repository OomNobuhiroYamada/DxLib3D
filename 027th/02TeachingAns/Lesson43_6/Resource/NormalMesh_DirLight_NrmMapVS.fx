// 頂点シェーダーの入力
struct VS_INPUT
{
	float4 Position        : POSITION ;     // 座標( ローカル空間 )
	float3 Tan             : TANGENT0 ;     // 接線( ローカル空間 )
	float3 Bin             : BINORMAL0 ;    // 従法線( ローカル空間 )
	float3 Normal          : NORMAL0 ;      // 法線( ローカル空間 )
	float4 Diffuse         : COLOR0 ;       // ディフューズカラー
	float4 Specular        : COLOR1 ;       // スペキュラカラー
	float4 TexCoords0      : TEXCOORD0 ;	// テクスチャ座標
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : POSITION ;     // 座標( プロジェクション空間 )
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VTan            : TEXCOORD2 ;    // 接線( ビュー空間 )
	float3 VBin            : TEXCOORD3 ;    // 従法線( ビュー空間 )
	float3 VNormal         : TEXCOORD4 ;    // 法線( ビュー空間 )
} ;



// C++ 側で設定する定数の定義
float4              cfProjectionMatrix[ 4 ]   : register( c2  ) ;		// ビュー　　→　射影行列
float4              cfViewMatrix[ 3 ]         : register( c6  ) ;		// ワールド　→　ビュー行列
float4              cfTextureMatrix[ 3 ][ 2 ] : register( c88 ) ;		// テクスチャ座標操作用行列
float4              cfLocalWorldMatrix[ 3 ]   : register( c94 ) ;		// ローカル　→　ワールド行列


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;
	float3 lWorldNrm ;
	float3 lWorldTan ;
	float3 lWorldBin ;
	float3 lViewNrm ;
	float3 lViewTan ;
	float3 lViewBin ;


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




	// 法線をビュー空間の角度に変換 =====================================================( 開始 )

	// 従法線、接線、法線をビューベクトルに変換
	lWorldTan.x = dot( VSInput.Tan, cfLocalWorldMatrix[ 0 ].xyz ) ;
	lWorldTan.y = dot( VSInput.Tan, cfLocalWorldMatrix[ 1 ].xyz ) ;
	lWorldTan.z = dot( VSInput.Tan, cfLocalWorldMatrix[ 2 ].xyz ) ;

	lWorldBin.x = dot( VSInput.Bin, cfLocalWorldMatrix[ 0 ].xyz ) ;
	lWorldBin.y = dot( VSInput.Bin, cfLocalWorldMatrix[ 1 ].xyz ) ;
	lWorldBin.z = dot( VSInput.Bin, cfLocalWorldMatrix[ 2 ].xyz ) ;

	lWorldNrm.x = dot( VSInput.Normal, cfLocalWorldMatrix[ 0 ].xyz ) ;
	lWorldNrm.y = dot( VSInput.Normal, cfLocalWorldMatrix[ 1 ].xyz ) ;
	lWorldNrm.z = dot( VSInput.Normal, cfLocalWorldMatrix[ 2 ].xyz ) ;

	lViewTan.x = dot( lWorldTan, cfViewMatrix[ 0 ].xyz ) ;
	lViewTan.y = dot( lWorldTan, cfViewMatrix[ 1 ].xyz ) ;
	lViewTan.z = dot( lWorldTan, cfViewMatrix[ 2 ].xyz ) ;

	lViewBin.x = dot( lWorldBin, cfViewMatrix[ 0 ].xyz ) ;
	lViewBin.y = dot( lWorldBin, cfViewMatrix[ 1 ].xyz ) ;
	lViewBin.z = dot( lWorldBin, cfViewMatrix[ 2 ].xyz ) ;

	lViewNrm.x = dot( lWorldNrm, cfViewMatrix[ 0 ].xyz ) ;
	lViewNrm.y = dot( lWorldNrm, cfViewMatrix[ 1 ].xyz ) ;
	lViewNrm.z = dot( lWorldNrm, cfViewMatrix[ 2 ].xyz ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 終了 )




	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// テクスチャ座標変換行列による変換を行った結果のテクスチャ座標をセット
	VSOutput.TexCoords0.x = dot( VSInput.TexCoords0, cfTextureMatrix[ 0 ][ 0 ] ) ;
	VSOutput.TexCoords0.y = dot( VSInput.TexCoords0, cfTextureMatrix[ 0 ][ 1 ] ) ;

	// 頂点座標を保存
	VSOutput.VPosition = lViewPosition.xyz ;

	// 接線を保存
	VSOutput.VTan = lViewTan ;

	// 従法線を保存
	VSOutput.VBin = lViewBin ;

	// 法線を保存
	VSOutput.VNormal = lViewNrm ;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return VSOutput ;
}

