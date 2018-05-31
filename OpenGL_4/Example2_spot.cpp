// Gouraud shading with a single light source at (4, 4, 0);
// Part 1 : Per vertex lighting is computed by CPU
// Part 2 : The control of the light source (position and intensity (RGB) )
//			Light's position could be moved automatically with Key A/a 
//			Light's intensity could be changed with Key R/r G/g B/b
// Part 3 : Geometry objects' materials
//
// 執行前的準備工作
// 開啟 CShape.h 中的 #define LIGHTING_WITHCPU 
// 關閉 CShape.h 中的 #define LIGHTING_WITHGPU 
// 關閉 CShape.h 中的 #define PERVERTEX_LIGHTING 
//
// 開啟 CShape.h 中的 #define SPOT_LIGHT 才能看到 SPOT_LIGHT 的效果
//
// 開起 #define SETTING_MATERIALS  可以看到材質
//
//
//

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common\CLineSegment.h"
#include "Common/CCamera.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

#define SETTING_MATERIALS 

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CChecker      *g_pChecker;
CSolidCube    *g_pCube;
CSolidSphere  *g_pSphere;

// For View Point
GLfloat g_fRadius = 10.0;
GLfloat g_fTheta = 60.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 3;
float g_fLightTheta = 0;

float g_fLightR = 0.95f;
float g_fLightG = 0.95f;
float g_fLightB = 0.95f;

CWireSphere *g_pLight;
vec4 g_vLight(3.0f, 5.0f, 0.0f, 1.0f); // x = r cos(theta) = 3, z = r sin(theta) = 0
vec4 g_fLightI( g_fLightR, g_fLightG, g_fLightB, 1.0f); 

LightSource g_Light1 = {
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	point4(3.0f, 5.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection，需重新計算
	1.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	60.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.5f,	// spotCosCutoff = cos(spotCutoff) ; spot 的照明範圍取 cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

CLineSegment *g_LightLine;
bool g_bUpdateLight = false;

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();

void init( void )
{
	mat4 mxT;
	vec4 vT, vColor;
	// 產生所需之 Model View 與 Projection Matrix

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// 產生物件的實體
	g_pChecker = new CChecker(GRID_SIZE);
	g_pChecker->SetShadingMode(GOURAUD_SHADING);
	g_pChecker->SetShader();
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pChecker->SetMaterials(vec4(0), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pChecker->	SetKaKdKsShini(0, 0.8f, 0.5f, 1);
#endif

	g_pCube = new CSolidCube;
	g_pCube->SetShader();
	// 設定 Cube
	vT.x = 1.5; vT.y = 0.5; vT.z = -1.5;
	mxT = Translate(vT);
	g_pCube->SetTRSMatrix(mxT);
	g_pCube->SetShadingMode(GOURAUD_SHADING);
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pCube->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pCube->SetKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
#endif

	g_pSphere = new CSolidSphere(1, 16, 16);
	g_pSphere->SetShader();
	// 設定 Sphere
	vT.x = -1.5; vT.y = 1.0; vT.z = 1.5;
	mxT = Translate(vT);
	g_pSphere->SetTRSMatrix(mxT);
	g_pSphere->SetShadingMode(GOURAUD_SHADING);
	// Part 3 : materials
#ifdef SETTING_MATERIALS
	g_pSphere->SetMaterials(vec4(0), vec4(0, 0, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSphere->SetKaKdKsShini(0.15f, 0.53f, 0.78f, 4);
#endif
	
	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetShader();
	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_fLightI);

	// 計算 SpotDirection Vector 同時正規化成單位向量
	g_Light1.UpdateDirection();

	g_LightLine = new CLineSegment(g_Light1.position, g_Light1.spotTarget, vec4(1, 0, 0, 1));
	g_LightLine->SetShader();

	// 因為本範例不會動到 Projection Matrix 所以在這裡設定一次即可
	// 就不寫在 OnFrameMove 中每次都 Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->SetProjectionMatrix(mpx);
	g_pCube->SetProjectionMatrix(mpx);
	g_pSphere->SetProjectionMatrix(mpx);
	g_pLight->SetProjectionMatrix(mpx);
	g_LightLine->SetProjectionMatrix(mpx);
}


//----------------------------------------------------------------------------
void GL_Display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	g_pChecker->Draw();
	g_pSphere->Draw();
	g_pCube->Draw();
	g_pLight->Draw();
	g_LightLine->Draw();
	glutSwapBuffers();	// 交換 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// 每秒繞 Y 軸轉 90 度
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime*(float)M_PI_2;
	if( g_fLightTheta >= (float)M_PI*2.0f ) {
		g_fLightTheta -= (float)M_PI*2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_vLight.x = g_fLightRadius * cosf(g_fLightTheta);
	g_vLight.z = g_fLightRadius * sinf(g_fLightTheta);

	g_bUpdateLight = true;

	mxT = Translate(g_vLight);
	g_pLight->SetTRSMatrix(mxT);
	g_LightLine->UpdatePosition(g_Light1.position, g_Light1.spotTarget);
}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{
	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view 與 projection matrix 是否需要更新給物件
	auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pChecker->SetViewMatrix(mvx);
		g_pCube->SetViewMatrix(mvx);
		g_pSphere->SetViewMatrix(mvx);
		g_pLight->SetViewMatrix(mvx);
		g_LightLine->SetViewMatrix(mvx);
	}

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		UpdateLightPosition(delta);
	}

	if ( g_bUpdateLight ) {
		g_Light1.position.x = g_vLight.x;
		g_Light1.position.z = g_vLight.z;
		g_Light1.spotDirection.x = g_Light1.spotTarget.x - g_Light1.position.x;
		g_Light1.spotDirection.y = g_Light1.spotTarget.y - g_Light1.position.y;
		g_Light1.spotDirection.z = g_Light1.spotTarget.z - g_Light1.position.z;
		g_Light1.spotDirection = normalize(g_Light1.spotDirection);
		g_LightLine->UpdatePosition(g_Light1.position, g_Light1.spotTarget);
		g_bUpdateLight = false;
	}
	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pChecker->Update(delta, g_Light1);
	g_pCube->Update(delta, g_Light1);
	g_pSphere->Update(delta, g_Light1);
	g_pLight->Update(delta);
	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case  SPACE_KEY:

		break;
//----------------------------------------------------------------------------
// Part 2 : for single light source
	case 65: // A key
	case 97: // a key
		g_bAutoRotating = !g_bAutoRotating;
		break;
	case 82: // R key
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_fLightI.x = g_fLightR;
		g_Light1.diffuse.x = g_fLightI.x;
		g_pLight->SetColor(g_fLightI);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_fLightI.x = g_fLightR;
		g_Light1.diffuse.x = g_fLightI.x;
		g_pLight->SetColor(g_fLightI);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_fLightI.y = g_fLightG;
		g_Light1.diffuse.y = g_fLightI.y;
		g_pLight->SetColor(g_fLightI);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_fLightI.y = g_fLightG;
		g_Light1.diffuse.y = g_fLightI.y;
		g_pLight->SetColor(g_fLightI);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_fLightI.z = g_fLightB;
		g_Light1.diffuse.z = g_fLightI.z;
		g_pLight->SetColor(g_fLightI);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_fLightI.z = g_fLightB;
		g_Light1.diffuse.z = g_fLightI.z;
		g_pLight->SetColor(g_fLightI);
		break;
//----------------------------------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		delete g_pCube;
		delete g_pSphere;
		delete g_pChecker;
		delete g_pLight;
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // 目前按下的是滑鼠左鍵
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // 目前按下的是滑鼠中鍵 ，換成 Y 軸
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // 目前按下的是滑鼠右鍵
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {
	// 從這裡控制 SpotLight target 的位置
	// 左右為移動 target 的 X 軸座標，上下為為移動 target 的 Z 軸座標
	switch(key) {
		case GLUT_KEY_LEFT:		// 目前按下的是向左方向鍵
			g_Light1.spotTarget.x -= 0.1f;
			g_bUpdateLight = true;
			break;
		case GLUT_KEY_RIGHT:	// 目前按下的是向右方向鍵
			g_Light1.spotTarget.x += 0.1f;
			g_bUpdateLight = true;
			break;
		case GLUT_KEY_UP:	// 目前按下的是向右方向鍵
			g_Light1.spotTarget.z -= 0.1f;
			g_bUpdateLight = true;
			break;
		case GLUT_KEY_DOWN:	// 目前按下的是向右方向鍵
			g_Light1.spotTarget.z += 0.1f;
			g_bUpdateLight = true;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {

	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {

	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE);  // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Shading Example 2 Spot");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// 處理 ASCI 按鍵如 A、a、ESC 鍵...等等
	glutSpecialFunc( Win_SpecialKeyboard);	// 處理 NON-ASCI 按鍵如 F1、Home、方向鍵...等等
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
    glutMainLoop();
    return 0;
}
