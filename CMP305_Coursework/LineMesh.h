#pragma once
#include "BaseMesh.h"
#include <vector>

class LineSegment {
public:
	LineSegment() {
		start = XMVectorSet(0, 0, 0, 1); end = XMVectorSet(0, 1, 0, 1);
	}
	LineSegment(XMVECTOR s, XMVECTOR e) {
		start = s; end = e;
	}
	
	XMVECTOR start;
	XMVECTOR end;
};

class LineMesh :
	public BaseMesh
{
public:
	
	LineMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~LineMesh();

	void	sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	void	BuildLine(ID3D11DeviceContext* deviceContext, ID3D11Device* device);

	inline int		GetLineCount() const { return m_LineList.size(); }

	//Line manipulation
	inline void		AddLine(XMVECTOR start, XMVECTOR end) { m_LineList.push_back(LineSegment(start, end)); }
	inline void		AddSegment() { m_LineList.push_back(LineSegment()); }
	inline void		Clear() { m_LineList.clear(); }

private:
	std::vector<VertexType>		vertices;
	std::vector<UINT>			indices;
	std::vector<LineSegment>	m_LineList;

	void	initBuffers(ID3D11Device* device);
};

