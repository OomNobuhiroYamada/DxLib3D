// ピクセルシェーダーの入力
struct PS_INPUT
{
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VTan            : TEXCOORD2 ;    // 接線( ビュー空間 )
	float3 VBin            : TEXCOORD3 ;    // 従法線( ビュー空間 )
	float3 VNormal         : TEXCOORD4 ;    // 法線( ビュー空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : COLOR0 ;
} ;

// マテリアルパラメータ
struct MATERIAL
{
	float4 Diffuse ;      // ディフューズカラー
	float4 Specular ;     // スペキュラカラー
	float4 Power ;        // スペキュラの強さ
} ;

// ライトパラメータ
struct LIGHT
{
	float4 Position ;               // 座標( ビュー空間 )
	float3 Direction ;              // 方向( ビュー空間 )
	float4 Diffuse ;                // ディフューズカラー
	float4 Specular ;               // スペキュラカラー
	float4 Ambient ;                // アンビエントカラーとマテリアルのアンビエントカラーを乗算したもの
	float4 Range_FallOff_AT0_AT1 ;  // x:有効距離  y:スポットライト用FallOff  z:距離による減衰処理用パラメータ０  w:距離による減衰処理用パラメータ１
	float4 AT2_SpotP0_SpotP1 ;      // x:距離による減衰処理用パラメータ２  y:スポットライト用パラメータ０( cos( Phi / 2.0f ) )  z:スポットライト用パラメータ１( 1.0f / ( cos( Theta / 2.0f ) - cos( Phi / 2.0f ) ) )
} ;


// C++ 側で設定するテクスチャや定数の定義
sampler  DiffuseMapTexture             : register( s0 ) ;		// ディフューズマップテクスチャ
sampler  NormalMapTexture              : register( s1 ) ;		// 法線マップテクスチャ
float4   cfAmbient_Emissive            : register( c1 ) ;		// エミッシブカラー + マテリアルアンビエントカラー * グローバルアンビエントカラー
MATERIAL cfMaterial                    : register( c2 ) ;		// マテリアルパラメータ
float4   cfFactorColor                 : register( c5 ) ;		// 不透明度等
LIGHT    cfLight[ 3 ]                  : register( c32 ) ;		// ライトパラメータ


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float3 V_to_Eye ;
	float3 Normal ;
	float3 VNrm ;
	float3 VTan ;
	float3 VBin ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float4 SpecularColor ;
	float3 TempF3 ;
	float Temp ;
	float3 lLightTemp ;
	float3 lLightDir ;
	float DiffuseAngleGen ;

	// 接線・従法線・法線を正規化
	VNrm = normalize( PSInput.VNormal );
	VTan = normalize( PSInput.VTan );
	VBin = normalize( PSInput.VBin );

	// 頂点座標から視点へのベクトルを接底空間に投影した後正規化して保存
	TempF3.x = dot( VTan, -PSInput.VPosition.xyz ) ;
	TempF3.y = dot( VBin, -PSInput.VPosition.xyz ) ;
	TempF3.z = dot( VNrm, -PSInput.VPosition.xyz ) ;
	V_to_Eye = normalize( TempF3 ) ;

	// 法線の 0～1 の値を -1.0～1.0 に変換する
	Normal = ( tex2D( NormalMapTexture, PSInput.TexCoords0.xy ).rgb - float3( 0.5f, 0.5f, 0.5f ) ) * 2.0f ;

	// ディフューズカラーとスペキュラカラーの蓄積値を初期化
	TotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	TotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;




	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	TempF3 = cfLight[ 0 ].Direction ;

	// ライトのベクトルを接地空間に変換
	lLightDir.x = dot( VTan, TempF3 ) ;
	lLightDir.y = dot( VBin, TempF3 ) ;
	lLightDir.z = dot( VNrm, TempF3 ) ;

	// ディフューズ色計算

	// DiffuseAngleGen = ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ライトのディフューズカラー * マテリアルのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
	TotalDiffuse += cfLight[ 0 ].Diffuse * cfMaterial.Diffuse * DiffuseAngleGen + cfLight[ 0 ].Ambient ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), cfMaterial.Power.x )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), cfMaterial.Power.x ) ;

	// スペキュラカラー蓄積値 += Temp * ライトのスペキュラカラー
	TotalSpecular += Temp * cfLight[ 0 ].Specular ;

	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )




	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
	TotalDiffuse += cfAmbient_Emissive ;

	// SpecularColor = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	SpecularColor = TotalSpecular * cfMaterial.Specular ;

	// 出力カラー = TotalDiffuse * テクスチャカラー + SpecularColor
	TextureDiffuseColor = tex2D( DiffuseMapTexture, PSInput.TexCoords0.xy ) ;
	PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;

	// アルファ値 = テクスチャアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * cfMaterial.Diffuse.a * cfFactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}


