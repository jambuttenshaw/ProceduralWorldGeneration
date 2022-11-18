/**
* \class Writable Texture
*
* \brief Provides an interface to set pixels of a texture at runtime
*
* 
* Not totally robust, but provides an example of the most important parts.
* 
* TODO:
*	Add support for different texture formats
*	Add support for different buffer formats & mapping
*
* \author Gaz Robinson
*/
#pragma once
#include <vector>
#include <wrl/client.h>
#include <DirectXMath.h>

class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Texture2D;
class ID3D11ShaderResourceView;

using std::vector;
struct UCHAR_Color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class WritableTexture
{
public:
	WritableTexture(int width, int height, ID3D11Device* device);
	~WritableTexture();

	inline ID3D11ShaderResourceView* GetTextureView() const {
		return m_TexView.Get();
	}
	/// <summary>
	/// Set pixel (X,Y) to a 0..1.0f RGBA value
	/// </summary>
	void SetPixel(int x, int y, float r, float g, float b, float a);
	void SetPixel(int x, int y, DirectX::XMFLOAT4 color);
	void Update(ID3D11DeviceContext* context) const;

private:
	const int m_Height, m_Width;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TexView;

	UCHAR_Color* m_PPixels;
};

