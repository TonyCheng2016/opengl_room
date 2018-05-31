#ifndef COBJ_H
#define COBJ_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define SOLIDCUBE_NUM 75000  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CObj : public CShape
{
private:
	vec4 m_vertices2[5000];
	vec4 m_vertices[5000];
	vec3 m_Normals2[6000];
	vec3 m_Normals[6000];
	vec3 m_Face[22000];
	int  m_iIndex;

	//void Quad(int a, int b, int c, int d);
public:
	CObj(char *filename);
	~CObj(){};

	void Update(float dt, point4 vLightPos, color4 vLightI);
	void Update(float dt, const LightSource &Lights);
	void Update(float dt); // 不計算光源的照明

	void RenderWithFlatShading(point4 vLightPos, color4 vLightI);
	void RenderWithGouraudShading(point4 vLightPos, color4 vLightI);
	void RenderWithFlatShading(const LightSource &Lights);
	void RenderWithGouraudShading(const LightSource &Lights);
	void Draw();
	void DrawW();

	//loadobj
	GLuint elephant;
	float elephantrot;
	char ch = '1';
	void loadObj(char *fname);//obj reader	
};

#endif