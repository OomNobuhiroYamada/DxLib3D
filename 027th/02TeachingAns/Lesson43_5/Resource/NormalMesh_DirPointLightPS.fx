// ピクセルシェーダーの入力
struct PS_INPUT
{
	float4 Diffuse         : COLOR0 ;
	float4 Specular        : COLOR1 ;
	float2 TexCoords0      : TEXCOORD0 ;
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : COLOR0 ;
} ;


// C++ 側で設定するテクスチャの定義
sampler  DiffuseMapTexture             : register( s0 ) ;		// ディフューズマップテクスチャ
float4   cfFactorColor                 : register( c5 ) ;		// 不透明度等



// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;

	// テクスチャカラーの読み込み
	TextureDiffuseColor = tex2D( DiffuseMapTexture, PSInput.TexCoords0.xy ) ;

	// 出力カラー = ディフューズカラー * テクスチャカラー + スペキュラカラー
	PSOutput.Color0 = PSInput.Diffuse * TextureDiffuseColor + PSInput.Specular ;

	// 出力アルファ = ディフューズアルファ * テクスチャアルファ * 不透明度
	PSOutput.Color0.a = PSInput.Diffuse.a * TextureDiffuseColor.a * cfFactorColor.a ;

	// 出力パラメータを返す
	return PSOutput ;
}
