#include "main.h"
#include <fstream>

// #include <assimp/Importer.hpp>      // C++ importer interface
// #include <assimp/scene.h>           // Output data structure
// #include <assimp/postprocess.h>     // Post processing flags

#include <locale>
#include <codecvt>
#include <string>
#include <array>
#include <locale> 

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

bool LoadModel(char* filename, std::vector<VertexPositionNormalTexture> & _vertices, std::vector<uint16_t> & _indices)
{
	int m_vertexCount, m_indexCount;

	std::ifstream fin;
	char input;
	int i;


	// Open the model file.  If it could not open the file then exit.
	fin.open(filename);
	if (fin.fail())
	{
		return false;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i<m_vertexCount; i++)
	{
		VertexPositionNormalTexture v;
		fin >> v.position.x >> v.position.y >> v.position.z;
		fin >> v.textureCoordinate.x >> v.textureCoordinate.y;
		fin >> v.normal.x >> v.normal.y >> v.normal.z;

		_vertices.push_back(v);

		_indices.push_back(i);
	}

	// Close the model file.
	fin.close();

	return true;
}


template<typename T>
void CreateBuffer(_In_ ID3D11Device* device, T const& data, D3D11_BIND_FLAG bindFlags, _Outptr_ ID3D11Buffer** pBuffer)
{
	assert(pBuffer != 0);

	D3D11_BUFFER_DESC bufferDesc = { 0 };

	bufferDesc.ByteWidth = (UINT)data.size() * sizeof(T::value_type);
	bufferDesc.BindFlags = bindFlags;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA dataDesc = { 0 };

	dataDesc.pSysMem = data.data();

	device->CreateBuffer(&bufferDesc, &dataDesc, pBuffer);

	//SetDebugObjectName(*pBuffer, "DirectXTK:GeometricPrimitive");
}

std::unique_ptr<DirectX::ModelMeshPart> CreateModelMeshPart(ID3D11Device* device, std::function<void(std::vector<VertexPositionNormalTexture> & _vertices, std::vector<uint16_t> & _indices)> createGeometry){
	std::vector<VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	createGeometry(vertices, indices);

	size_t nVerts = vertices.size();

	std::unique_ptr<DirectX::ModelMeshPart> modelMeshPArt(new DirectX::ModelMeshPart());

	modelMeshPArt->indexCount = indices.size();
	modelMeshPArt->startIndex = 0;
	modelMeshPArt->vertexOffset = 0;
	modelMeshPArt->vertexStride = sizeof(VertexPositionNormalTexture);
	modelMeshPArt->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	modelMeshPArt->indexFormat = DXGI_FORMAT_R16_UINT;
	modelMeshPArt->vbDecl = std::shared_ptr<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
		new std::vector<D3D11_INPUT_ELEMENT_DESC>(
		&VertexPositionNormalTexture::InputElements[0],
		&VertexPositionNormalTexture::InputElements[VertexPositionNormalTexture::InputElementCount]
		)
		);

	CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, modelMeshPArt->vertexBuffer.ReleaseAndGetAddressOf());

	CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, modelMeshPArt->indexBuffer.ReleaseAndGetAddressOf());

	return modelMeshPArt;
}

void CreateSinglePointBuffer(ID3D11Buffer ** vertexBuffer,  ID3D11Device* device, DirectX::IEffect * effect, ID3D11InputLayout ** layout){
	HRESULT hr;
	void const* shaderByteCode;
	size_t byteCodeLength;

	std::vector<XMFLOAT3> vertices;

	vertices.push_back(XMFLOAT3(0,0,0));

	CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, vertexBuffer);

	effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	const D3D11_INPUT_ELEMENT_DESC vbDecl[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(&vbDecl[0],1,shaderByteCode, byteCodeLength,layout);
}

std::unique_ptr<DirectX::ModelMeshPart> CreateQuadModelMeshPart(ID3D11Device* device){
	std::vector<XMFLOAT3> vertices;
	std::vector<uint16_t> indices;

	vertices.push_back(XMFLOAT3( -1, -1, 0.5));
	vertices.push_back(XMFLOAT3( -1,  1, 0.5));
	vertices.push_back(XMFLOAT3(  1, -1, 0.5));
	vertices.push_back(XMFLOAT3(  1,  1, 0.5));

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	std::unique_ptr<DirectX::ModelMeshPart> modelMeshPArt(new DirectX::ModelMeshPart());

	const D3D11_INPUT_ELEMENT_DESC vbDecl[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	modelMeshPArt->indexCount = indices.size();
	modelMeshPArt->startIndex = 0;
	modelMeshPArt->vertexOffset = 0;
	modelMeshPArt->vertexStride = sizeof(XMFLOAT3);
	modelMeshPArt->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	modelMeshPArt->indexFormat = DXGI_FORMAT_R16_UINT;
	modelMeshPArt->vbDecl = std::shared_ptr<std::vector<D3D11_INPUT_ELEMENT_DESC>>(
		new std::vector<D3D11_INPUT_ELEMENT_DESC>(
		&vbDecl[0],
		&vbDecl[1]
		)
		);

	CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, modelMeshPArt->vertexBuffer.ReleaseAndGetAddressOf());

	CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, modelMeshPArt->indexBuffer.ReleaseAndGetAddressOf());

	return modelMeshPArt;
}

SceneNode::~SceneNode(){
	for (int i = 0; i < children.size(); i++)
		delete children[i];
	for (int i = 0; i < mesh.size(); i++)
		delete mesh[i];
}

////
void modelMeshPartDraw(ID3D11DeviceContext* deviceContext, ModelMeshPart* mmp, ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4X4 transformation, IEffect* ieffect, ID3D11InputLayout* iinputLayout, std::function<void(ID3D11ShaderResourceView * texture, DirectX::XMFLOAT4X4 transformation)> setCustomState)
{
	deviceContext->IASetInputLayout(iinputLayout);

	auto vb = mmp->vertexBuffer.Get();
	UINT vbStride = mmp->vertexStride;
	UINT vbOffset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);

	// Note that if indexFormat is DXGI_FORMAT_R32_UINT, this model mesh part requires a Feature Level 9.2 or greater device
	deviceContext->IASetIndexBuffer(mmp->indexBuffer.Get(), mmp->indexFormat, 0);

	assert(ieffect != 0);
	ieffect->Apply(deviceContext);

	// Hook lets the caller replace our shaders or state settings with whatever else they see fit.
	setCustomState(texture, transformation);

	// Draw the primitive.
	deviceContext->IASetPrimitiveTopology(mmp->primitiveType);

	deviceContext->DrawIndexed(mmp->indexCount, mmp->startIndex, mmp->vertexOffset);
}
////

void SceneNode::draw(_In_ ID3D11DeviceContext* deviceContext, _In_ IEffect* ieffect, _In_ ID3D11InputLayout* iinputLayout,
	_In_opt_ std::function<void(ID3D11ShaderResourceView * texture, DirectX::XMFLOAT4X4 transformation)> setCustomState){
	for (int i = 0; i < children.size(); i++)
		children[i]->draw(deviceContext, ieffect, iinputLayout, setCustomState);
	for (int i = 0; i < mesh.size(); i++)
		//mesh[i]->Draw(deviceContext, ieffect, iinputLayout, setCustomState);
		modelMeshPartDraw(deviceContext, mesh[i], texture[i].Get(), transformation, ieffect, iinputLayout, setCustomState);
}
XMFLOAT4X4& assign(XMFLOAT4X4& output, const aiMatrix4x4& aiMe){
	output._11 = aiMe.a1;
	output._12 = aiMe.a2;
	output._13 = aiMe.a3;
	output._14 = aiMe.a4;

	output._21 = aiMe.b1;
	output._22 = aiMe.b2;
	output._23 = aiMe.b3;
	output._24 = aiMe.b4;

	output._31 = aiMe.c1;
	output._32 = aiMe.c2;
	output._33 = aiMe.c3;
	output._34 = aiMe.c4;

	output._41 = aiMe.d1;
	output._42 = aiMe.d2;
	output._43 = aiMe.d3;
	output._44 = aiMe.d4;

	return output;
}
void collectMeshes(const aiScene* scene, int level, aiNode * node, std::vector<VertexPositionNormalTexture> * _vertices, std::vector<uint16_t> * _indices, DirectX::XMFLOAT4X4 parentTransformation){
	
	SimpleMath::Matrix transformation = SimpleMath::Matrix(assign(XMFLOAT4X4(), node->mTransformation));

	transformation = transformation.Transpose();

	auto nodeTransformation = transformation * parentTransformation;

	char buffer[1024];
	sprintf(buffer, "%d %s \n", level, node->mName.C_Str());// , delta.y);
	OutputDebugStringA(buffer);

	if (std::string("former") == std::string(node->mName.C_Str())){
		unsigned int vertex_size = 0;
		for (int i = 0; i < node->mNumMeshes; i++){
			auto nodeMesh = scene->mMeshes[node->mMeshes[i]];

			for (int k = 0; k < nodeMesh->mNumVertices; k++){
				VertexPositionNormalTexture v;

				auto _f = SimpleMath::Vector4::Transform(SimpleMath::Vector4(nodeMesh->mVertices[k].x, nodeMesh->mVertices[k].y, nodeMesh->mVertices[k].z, 1.0f), nodeTransformation);

				v.position.x = _f.x;
				v.position.y = _f.y;
				v.position.z = _f.z;

				_vertices->push_back(v);
			}

			for (int m = 0; m < nodeMesh->mNumFaces; m++){
				if (nodeMesh->mFaces[m].mNumIndices != 3) throw "";
				for (int n = 0; n < 3; n++){
					_indices->push_back(vertex_size + nodeMesh->mFaces[m].mIndices[n]);
				};
			}

			vertex_size += nodeMesh->mNumVertices;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++){

		collectMeshes(scene, ++level, node->mChildren[i], _vertices, _indices, nodeTransformation);
	}
}

float degToRad(float d){
	return (d * 3.141592654f) / 180.0f;
}

void loadJet(ID3D11Device* device, std::vector<VertexPositionNormalTexture> & _vertices, std::vector<uint16_t> & _indices){

	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll 
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile("models\\jet.fbx", aiProcess_Triangulate | aiProcess_MakeLeftHanded);// | aiProcess_MakeLeftHanded);

	auto node = scene->mRootNode;

	SimpleMath::Matrix transformation = SimpleMath::Matrix::CreateScale(5, 5, 5) * SimpleMath::Matrix::CreateRotationZ(degToRad(90.0f)) * SimpleMath::Matrix::CreateRotationY(degToRad(-90.0f));

	collectMeshes(scene, 0, node, &_vertices, &_indices, transformation);
}