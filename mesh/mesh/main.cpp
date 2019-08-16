#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/mesh.h>

#include <vector>

int main()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("  ", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	return 0;
}