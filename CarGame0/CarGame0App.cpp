#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "CarGame0App.h"
#include "../BaseCodes/Camera.h"
#include "../BaseCodes/GroundObj2.h"
#include "../BaseCodes/InitShader.h"
#include "../BaseCodes/BasicShapeObjs.h"
#include "CarModel.h"
#include "TreeModel0.h"


// Window and User Interface
static bool g_left_button_pushed;
static bool g_right_button_pushed;
static int g_last_mouse_x;
static int g_last_mouse_y;

extern GLuint g_window_w;
extern GLuint g_window_h;

//////////////////////////////////////////////////////////////////////
// Camera 
//////////////////////////////////////////////////////////////////////
static Camera g_camera;
static int g_camera_mode = 0;
static int g_switch = 0;


//light
glm::vec3 pos;
glm::vec3 look;


//////////////////////////////////////////////////////////////////////
//// Define Shader Programs
//////////////////////////////////////////////////////////////////////
GLuint s_program_id;


//////////////////////////////////////////////////////////////////////
//// Animation Parameters
//////////////////////////////////////////////////////////////////////
float g_elaped_time_s = 0.f;	

//////////////////////////////////////////////////////////////////////
//// Car Position, Rotation, Velocity
//// 자동차 제어 변수들.
//////////////////////////////////////////////////////////////////////
glm::vec3 g_car_poisition(0.f, 0.f, 0.f); //위치

float g_car_speed = 0;			          // 속도 (초당 이동 거리)
float g_car_rotation_y = 0;		          // 현재 방향 (y축 회전)
float g_car_angular_speed = 0;	          // 회전 속도 (각속도 - 초당 회전 각)

//dayandnight mode
bool g_dayandnight = false ;
int night = 0;

//smooth
static float fov;
static glm::vec3 position_s;
static glm::vec3 look_s;

/**
InitOpenGL: 프로그램 초기 값 설정을 위해 최초 한 번 호출되는 함수. (main 함수 참고)
OpenGL에 관련한 초기 값과 프로그램에 필요한 다른 초기 값을 설정한다.
예를들어, VAO와 VBO를 여기서 생성할 수 있다.
*/
void InitOpenGL()
{
	s_program_id = CreateFromFiles("../Shaders/v_shader.glsl", "../Shaders/f_shader.glsl");
	glUseProgram(s_program_id);

	glViewport(0, 0, (GLsizei)g_window_w, (GLsizei)g_window_h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Initial State of Camera
	// 카메라 초기 위치 설정한다.
	g_camera.lookAt(glm::vec3(3.f, 2.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	// basic meshes
	InitBasicShapeObjs();

	// Tree
	InitTreeModel();

	// Car
	InitCarModel();

	// 바닥 격자 VAO 생성
	InitGround2();	

	//smooth
	fov = 90.f;
	position_s = glm::vec3(0.1f, 0.6f, -0.1f);
	look_s = glm::vec3(0.1f, 0.5f, 1.f);	
}



/**
ClearOpenGLResource: 프로그램이 끝나기 메모리 해제를 위해 한 번 호출되는 함수. (main 함수 참고)
프로그램에서 사용한 메모리를 여기에서 해제할 수 있다.
예를들어, VAO와 VBO를 여기서 지울 수 있다.
*/
void ClearOpenGLResource()
{
	// Delete (VAO, VBO)
	DeleteBasicShapeObjs();
	DeleteTreeModel();
	DeleteCarModel();
	DeleteGround2();
}


/**
Display: 윈도우 화면이 업데이트 될 필요가 있을 때 호출되는 callback 함수.

윈도우 상에 최종 결과를 렌더링 하는 코드는 이 함수 내에 구현해야한다.
원도우가 처음 열릴 때, 윈도우 크기가 바뀔 때, 다른 윈도우에 의해 화면의 일부
또는 전체가 가려졌다가 다시 나타날 때 등 시스템이 해당 윈도우 내의 그림에 대한
업데이트가 필요하다고 판단하는 경우 자동으로 호출된다.
강제 호출이 필요한 경우에는 glutPostRedisplay() 함수를 호출하면된다.

이 함수는 불시에 빈번하게 호출된다는 것을 명심하고, 윈도우 상태 변화와 무관한
1회성 코드는 가능한한 이 함수 밖에 구현해야한다. 특히 메모리 할당, VAO, VBO 생성
등의 하드웨어 점유를 시도하는 코드는 특별한 이유가 없다면 절대 이 함수에 포함시키면
안된다. 예를 들어, 메시 모델을 정의하고 VAO, VBO를 설정하는 부분은 최초 1회만
실행하면되므로 main() 함수 등 외부에 구현해야한다. 정의된 메시 모델을 프레임 버퍼에
그리도록 지시하는 코드만 이 함수에 구현하면 된다.

만일, 이 함수 내에서 동적 메모리 할당을 해야하는 경우가 있다면 해당 메모리는 반드시
이 함수가 끝나기 전에 해제 해야한다.

ref: https://www.opengl.org/resources/libraries/glut/spec3/node46.html#SECTION00081000000000000000
*/
void Display()
{
	// 전체 화면을 지운다.
	// glClear는 Display 함수 가장 윗 부분에서 한 번만 호출되어야한다.

	//배경화면 색 바꾸기
	if (g_dayandnight == true) {
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	}
	else {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Vertex shader 의 matrix 변수들의 location을 받아온다.
	int m_proj_loc = glGetUniformLocation(s_program_id, "proj_matrix");
	int m_view_loc = glGetUniformLocation(s_program_id, "view_matrix");
	int m_model_loc = glGetUniformLocation(s_program_id, "model_matrix");


	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;

	
	//first-person-view
	if (g_camera_mode == 1) {
		
		projection_matrix = glm::perspective(glm::radians(90.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		// Camera Transform Matrix 설정.

		pos = glm::rotateY(glm::vec3(0.1f, 0.6f, -0.1f), g_car_rotation_y);
		pos += g_car_poisition;
		look = glm::rotateY(glm::vec3(0.1f, 0.5f, 1.f), g_car_rotation_y);
		look += g_car_poisition;

		fov = 90.f;
		position_s = pos;
		look_s = look;

		view_matrix = glm::lookAt(pos, look, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}

	//bird-view
	else if (g_camera_mode == 2) {
		
		projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		pos = glm::rotateY(glm::vec3(0.f, 2.f, -3.f), g_car_rotation_y);
		pos += g_car_poisition;
		look = glm::rotateY(glm::vec3(0.f, 0.f, 0.f), g_car_rotation_y);
		look += g_car_poisition;

		fov = 45.f;
		position_s = pos;
		look_s = look;

		view_matrix = glm::lookAt(pos, look, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}

	//first-person-view
	else if (g_camera_mode == 3) {
		projection_matrix = glm::perspective(glm::radians(fov), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		// Camera Transform Matrix 설정.

		pos = glm::rotateY(position_s, g_car_rotation_y);
		pos += g_car_poisition;
		look = glm::rotateY(look_s, g_car_rotation_y);
		look += g_car_poisition;

		view_matrix = glm::lookAt(pos, look, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}

	//bird-view
	else if (g_camera_mode == 4) {
		projection_matrix = glm::perspective(glm::radians(fov), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		pos = glm::rotateY(position_s, g_car_rotation_y);
		pos += g_car_poisition;
		look = glm::rotateY(look_s, g_car_rotation_y);
		look += g_car_poisition;

		view_matrix = glm::lookAt(pos, look, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}

	else
	{
		//이부분은 건들지 않기
		// Projection Transform Matrix 설정.
		projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Camera Transform Matrix 설정.
		view_matrix = g_camera.GetGLViewMatrix();
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}


	//Light 설정
	{
		
		int num_of_lights_loc = glGetUniformLocation(s_program_id, "num_of_lights");
		glUniform1i(num_of_lights_loc, 5);


		//1번. directional light
		{
				
			if (g_dayandnight == true) {
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
			glUniform1i(type_loc, 0);


			// 빛의 방향 설정.
			glm::vec3 dir(-1.f, -1.f, -1.f);
			dir = glm::normalize(dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// 빛의 세기 설정.
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity");
			glUniform3f(intensity_loc, 0.1f, 0.1f, 0.1f);
			}
			else {
				int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
				glUniform1i(type_loc, 0);

				// 빛의 방향 설정.
				glm::vec3 dir(-1.f, -1.f, -1.f);
				dir = glm::normalize(dir);

				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
				dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

				int dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
				glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

				// 빛의 세기 설정.
				int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity");
				glUniform3f(intensity_loc, 1.f, 1.f, 1.f);
			}
		}


		//2번. pointlight
		{
			if (g_dayandnight == true) // Point Light 설정
			{	

				//반딧불이1
				// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
				int type_loc = glGetUniformLocation(s_program_id, "lights[3].type");
				glUniform1i(type_loc, 1);

				// 빛이 출발하는 위치(광원) 설정.
				// 시간에 따라 위치가 변하도록 함.
				glm::vec3 pos(2.f , 0.4f+0.2f*cos(g_elaped_time_s), 2.f + 0.3f* sin(g_elaped_time_s));

				// Apply Camera Matrices
				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
				pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

				int pos_loc = glGetUniformLocation(s_program_id, "lights[3].position");
				glUniform3f(pos_loc, pos[0], pos[1], pos[2]);


				// 빛의 세기 설정.
				int intensity_loc = glGetUniformLocation(s_program_id, "lights[3].intensity");
				glUniform3f(intensity_loc, 0.f, 1.f, 0.f);


				{
					int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
					glUniform1i(shading_mode_loc, 1);
						
					// 변환 행렬을 설정한다.
					glm::mat4 model_T;
					model_T = glm::translate(glm::vec3(2.f, 0.4f + 0.2f*cos(g_elaped_time_s), 2.f + 0.3f* sin(g_elaped_time_s))) * glm::scale(glm::vec3(0.03f, 0.03f, 0.03f));
					glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));

					// 전체 꼭지점에 적용될 Color 값을 설정한다. 
					glVertexAttrib4f(2, 0.0f, 1.0f, 0.0f, 1.f);

					// 구를 그린다.
					DrawSphere();
					glUniform1i(shading_mode_loc, 0);
				}

				//반딧불이2
				// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
				int type_loc2 = glGetUniformLocation(s_program_id, "lights[4].type");
				glUniform1i(type_loc2, 1);

				// 빛이 출발하는 위치(광원) 설정.
				// 시간에 따라 위치가 변하도록 함.
				glm::vec3 pos2(-2.f + 0.3f* sin(g_elaped_time_s), 0.4f + 0.2f*sin(g_elaped_time_s), -2.f );

				// Apply Camera Matrices
				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
				pos2 = glm::vec3(view_matrix * glm::vec4(pos2, 1.f));

				int pos_loc2 = glGetUniformLocation(s_program_id, "lights[4].position");
				glUniform3f(pos_loc2, pos2[0], pos2[1], pos2[2]);

				// 빛의 세기 설정.
				int intensity_loc2 = glGetUniformLocation(s_program_id, "lights[4].intensity");
				glUniform3f(intensity_loc2, 0.f, 1.f, 0.f);
					

				{
					int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
					glUniform1i(shading_mode_loc, 1);

					// 변환 행렬을 설정한다.
					glm::mat4 model_T2;
					model_T2 = glm::translate(glm::vec3(-2.f + 0.3f* sin(g_elaped_time_s), 0.4f + 0.2f*sin(g_elaped_time_s), -2.f )) * glm::scale(glm::vec3(0.03f, 0.03f, 0.03f));
					glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T2));

					// 전체 꼭지점에 적용될 Color 값을 설정한다. 
					glVertexAttrib4f(2, 0.0f, 1.0f, 0.0f, 1.f);

					// 구를 그린다.
					DrawSphere();
					glUniform1i(shading_mode_loc, 0);
				}
			}
			else {
				int type_loc = glGetUniformLocation(s_program_id, "lights[3].type");
				glUniform1i(type_loc, 3);
				int type_loc2 = glGetUniformLocation(s_program_id, "lights[4].type");
				glUniform1i(type_loc2, 3);
			}
		}

		//3번. spotlight
		{
			if (g_dayandnight == true) {

				//전조등1
				int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
				glUniform1i(type_loc, 2);


				// 빛의 출발 점과, 진행 방향 설정.
				glm::vec3 light_position = glm::rotateY(glm::vec3(0.15f, 0.2f, 0.3f), g_car_rotation_y);
				light_position += g_car_poisition;
				glm::vec3 light_dir = glm::rotateY(glm::vec3(0.0f, -0.3f, 2.f), g_car_rotation_y);
				//light_dir += g_car_poisition;
				light_dir = glm::normalize(light_dir);


				// Apply Camera Matrices
				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  light_position는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
				light_position = glm::vec3(view_matrix * glm::vec4(light_position, 1.f));
				//  light_dir는 방향을 나타내는 벡터이므로 이동(Translation)변환이 적용되지 않도록 한다. (네 번째 요소 0.f으로 셋팅)
				light_dir = glm::vec3(view_matrix * glm::vec4(light_dir, 0.f));


				int light_position_loc = glGetUniformLocation(s_program_id, "lights[1].position");
				glUniform3f(light_position_loc, light_position[0], light_position[1], light_position[2]);

				int light_dir_loc = glGetUniformLocation(s_program_id, "lights[1].dir");
				glUniform3f(light_dir_loc, light_dir[0], light_dir[1], light_dir[2]);


				// Spot Light 변수 설정.
				float light_cos_cutoff = cos(glm::radians(20.f));
				glm::vec3 light_indensity(1.0f, 1.0f, 0.0f);	// yellow Light

				int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[1].cos_cutoff");
				glUniform1f(light_cos_cutoff_loc, light_cos_cutoff);

				int light_intensity_loc = glGetUniformLocation(s_program_id, "lights[1].intensity");
				glUniform3f(light_intensity_loc, light_indensity[0], light_indensity[1], light_indensity[2]);



				//전조등2
				int type_loc2 = glGetUniformLocation(s_program_id, "lights[2].type");
				glUniform1i(type_loc2, 2);

				// 빛의 출발 점과, 진행 방향 설정.
				glm::vec3 light_position2 = glm::rotateY(glm::vec3(-0.15f, 0.2f, 0.3f), g_car_rotation_y);
				light_position2 += g_car_poisition;
				glm::vec3 light_dir2 = glm::rotateY(glm::vec3(0.0f, -0.3f, 2.f), g_car_rotation_y);
				//light_dir2 += g_car_poisition;
				light_dir2 = glm::normalize(light_dir2);


				// Apply Camera Matrices
				////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
				//  light_position는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
				light_position2 = glm::vec3(view_matrix * glm::vec4(light_position2, 1.f));
				//  light_dir는 방향을 나타내는 벡터이므로 이동(Translation)변환이 적용되지 않도록 한다. (네 번째 요소 0.f으로 셋팅)
				light_dir2 = glm::vec3(view_matrix * glm::vec4(light_dir2, 0.f));


				int light_position_loc2 = glGetUniformLocation(s_program_id, "lights[2].position");
				glUniform3f(light_position_loc2, light_position2[0], light_position2[1], light_position2[2]);

				int light_dir_loc2 = glGetUniformLocation(s_program_id, "lights[2].dir");
				glUniform3f(light_dir_loc2, light_dir2[0], light_dir2[1], light_dir2[2]);


				// Spot Light 변수 설정.
				float light_cos_cutoff2 = cos(glm::radians(20.f));
				glm::vec3 light_indensity2(1.0f, 1.0f, 0.0f);	// yellow Light

				int light_cos_cutoff_loc2 = glGetUniformLocation(s_program_id, "lights[2].cos_cutoff");
				glUniform1f(light_cos_cutoff_loc2, light_cos_cutoff2);

				int light_intensity_loc2 = glGetUniformLocation(s_program_id, "lights[2].intensity");
				glUniform3f(light_intensity_loc2, light_indensity2[0], light_indensity2[1], light_indensity2[2]);


				{
					int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
					glUniform1i(shading_mode_loc, 1);

					// Sphere를 위한 Phong Shading 관련 변수 값을 설정한다.
					glm::vec3 l_position = glm::rotateY(glm::vec3(0.15f, 0.35f, 0.4f), g_car_rotation_y);
					l_position += g_car_poisition;

					// 변환 행렬을 설정한다.
					glm::mat4 model_T;
					model_T = glm::translate(l_position) * glm::scale(glm::vec3(0.03f, 0.03f, 0.03f));
					glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));

					// 전체 꼭지점에 적용될 Color 값을 설정한다. 
					glVertexAttrib4f(2, 1.0f, 1.0f, 0.0f, 1.f);
						
					// 구를 그린다.
					DrawSphere();
					glUniform1i(shading_mode_loc, 0);
				}

				{

					// Sphere를 위한 Phong Shading 관련 변수 값을 설정한다.
					int shading_mode_loc = glGetUniformLocation(s_program_id, "shading_mode");
					glUniform1i(shading_mode_loc, 1);

					glm::vec3 l_position = glm::rotateY(glm::vec3(-0.15f, 0.35f, 0.4f), g_car_rotation_y);
					l_position += g_car_poisition;

					// 변환 행렬을 설정한다.
					glm::mat4 model_T;
					model_T = glm::translate(l_position) * glm::scale(glm::vec3(0.03f, 0.03f, 0.03f));
					glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));

					// 전체 꼭지점에 적용될 Color 값을 설정한다. 
					glVertexAttrib4f(2, 1.0f, 1.0f, 0.0f, 1.f);

					// 구를 그린다.
					DrawSphere();
					glUniform1i(shading_mode_loc, 0);
				}
			}
			else {
				int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
				glUniform1i(type_loc, 3);
				int type_loc2 = glGetUniformLocation(s_program_id, "lights[2].type");
				glUniform1i(type_loc2, 3);
			}
		}
	}

	// Ground
	{
		// Ground를 위한 Phong Shading 관련 변수 값을 설정한다.
		int shininess_loc = glGetUniformLocation(s_program_id, "shininess_n");
		glUniform1f(shininess_loc, 50.f);

		int K_s_loc = glGetUniformLocation(s_program_id, "K_s");
		glUniform3f(K_s_loc, 0.3f, 0.3f, 0.3f);

		// 카메라 변환 행렬을 설정한다.
		glm::mat4 model_T(1.f);
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));

		// 그린다.
		DrawGround2();
	}


	// 바닥 격자
	glm::mat4 T0(1.f); // 단위 행렬
	glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(T0));
	DrawGround2();
	

	// Moving Car
	{
		glm::mat4 car_T = glm::translate(g_car_poisition) * glm::rotate(g_car_rotation_y, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE,  glm::value_ptr(car_T));
		DrawCarModel();
	}

	// 나무
	for (int i = 0; i <= 5; i++)
	{
		for (int j = 0; j <= 5; j++)
		{
			glm::mat4 model_T;
			model_T = glm::translate(glm::vec3(i * 2.f - 5.f, 0.f, j * 2.f - 5.f));
			glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));
			DrawTreeModel();
		}
	}


	// flipping the double buffers
	// glutSwapBuffers는 항상 Display 함수 가장 아래 부분에서 한 번만 호출되어야한다.
	glutSwapBuffers();
}


/**
Timer: 지정된 시간 후에 자동으로 호출되는 callback 함수.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node64.html#SECTION000819000000000000000
*/
void Timer(int value)
{
	// Timer 호출 시간 간격을 누적하여, 최초 Timer가 호출된 후부터 현재까지 흘러간 계산한다.
	g_elaped_time_s += value/1000.f;

	// Turn
	g_car_rotation_y += g_car_angular_speed;

	//smooth
	if (g_camera_mode == 3) {
		if (fov != 90.f) {
			fov += 0.4f;
			if (fov > 90.f) {
				fov = 90.f;
			}
		}
		//위치와 방향(벡터니까 거리를 생각 해야함)
		if (position_s != glm::vec3(0.1f, 0.6f, -0.1f)) {
			glm::vec3 pos_s=(glm::vec3(0.1f,0.6f,-0.1f)-position_s)/60.f;
			position_s += pos_s;
		}

		if (look_s != glm::vec3(0.1f, 0.5f, 1.f)) {
			glm::vec3 look_sm = (glm::vec3(0.1f, 0.5f, 1.f) - look_s) / 60.f;
			look_s += look_sm;
		}
	}

	else if (g_camera_mode == 4) {
		if (fov != 45.f) {
			fov -= 0.4f;
			if (fov < 45.f) {
				fov = 45.f;
			}
		}
		//위치와 방향(벡터니까 거리를 생각해야함 - 내가 잘못생각했던 부분(거리 대신 포인트로 생각해버림))
		if (position_s != glm::vec3(0.f, 2.f, -3.f)) {
			glm::vec3 pos_s = (glm::vec3(0.f, 2.f, -3.f) - position_s) / 60.f;
			position_s += pos_s;
		}

		if (look_s != glm::vec3(0.0f, 0.0f, 0.0f)) {
			glm::vec3 look_sm = (glm::vec3(0.0f, 0.0f, 0.0f) - look_s) / 60.f;
			look_s += look_sm;
		}
	}


	//perspective

	// Calculate Velocity
	//자동차가 앞으로 가는지 뒤로 가는지 만들때 xyz에 의존하면 그건 자동차가 움직이는게 아님
	//그래서 자동차가 보고있는 방향에 따라 앞뒤로 움직여야하므로 아래코드 처럼 만듦
	glm::vec3 speed_v = glm::vec3(0.f, 0.f, g_car_speed);
	glm::vec3 velocity = glm::rotateY(speed_v, g_car_rotation_y);	// speed_v 를 y축을 기준으로 g_car_rotation_y 만큼 회전한다.

	// Move
	g_car_poisition += velocity;


	// glutPostRedisplay는 가능한 빠른 시간 안에 전체 그림을 다시 그릴 것을 시스템에 요청한다.
	// 결과적으로 Display() 함수가 호출 된다.
	glutPostRedisplay();

	// 1/60 초 후에 Timer 함수가 다시 호출되로록 한다.
	// Timer 함수 가 동일한 시간 간격으로 반복 호출되게하여,
	// 애니메이션 효과를 표현할 수 있다
	glutTimerFunc((unsigned int)(1000 / 60), Timer, (1000 / 60));
}



/**
Reshape: 윈도우의 크기가 조정될 때마다 자동으로 호출되는 callback 함수.

@param w, h는 각각 조정된 윈도우의 가로 크기와 세로 크기 (픽셀 단위).
ref: https://www.opengl.org/resources/libraries/glut/spec3/node48.html#SECTION00083000000000000000
*/
void Reshape(int w, int h)
{
	//  w : window width   h : window height
	g_window_w = w;
	g_window_h = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glutPostRedisplay();
}

/**
Keyboard: 키보드 입력이 있을 때마다 자동으로 호출되는 함수.
@param key는 눌려진 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/

//눌려진 키 값 / 마우스의 x,y 값
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = -0.01f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.01f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = glm::radians( 1.f );		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = -1 * glm::radians( 1.f );		//  우회전 각속도 설정
		glutPostRedisplay();
		break;

	case '1':
		g_camera_mode = 0;
		glutPostRedisplay();
		break;

	case '2':
		g_camera_mode = 1;
		glutPostRedisplay();
		break;

	case '3':
		g_camera_mode = 2;
		glutPostRedisplay();
		break;

	case '4':
		g_camera_mode = 3;
		glutPostRedisplay();
		break;

	case '5':
		g_camera_mode = 4;
		glutPostRedisplay();
		break;
	
	case 'n':
		night++;
		if (night % 2 == 1) {
			g_dayandnight = true;
		}
		else if (night % 2 == 0) {
			g_dayandnight = false;
		}
		glutPostRedisplay();
		break;
	}

}

/**
KeyboardUp: 눌려졌던 키가 놓여질 때마다 자동으로 호출되는 함수.
@param key는 해당 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = 0.f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = 0.f;		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = 0.f;		//  우회전 각속도 설정
		glutPostRedisplay();
		break;

	}

}



/**
Mouse: 마우스 버튼이 입력될 때마다 자동으로 호출되는 함수.
파라메터의 의미는 다음과 같다.
@param button: 사용된 버튼의 종류
  GLUT_LEFT_BUTTON - 왼쪽 버튼
  GLUT_RIGHT_BUTTON - 오른쪽 버튼
  GLUT_MIDDLE_BUTTON - 가운데 버튼 (휠이 눌러졌을 때)
  3 - 마우스 휠 (휠이 위로 돌아 갔음).
  4 - 마우스 휠 (휠이 아래로 돌아 갔음).
@param state: 조작 상태
  GLUT_DOWN - 눌러 졌음
  GLUT_UP - 놓여졌음
@param x,y: 조작이 일어났을 때, 마우스 포인터의 좌표값.
*/
void Mouse(int button, int state, int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;


	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_left_button_pushed = true;

	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		g_left_button_pushed = false;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		g_right_button_pushed = true;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		g_right_button_pushed = false;
	else if (button == 3)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, -1, 0.01f);
		glutPostRedisplay();
	}
	else if (button == 4)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, 1, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}




/**
MouseMotion: 마우스 포인터가 움직일 때마다 자동으로 호출되는 함수.
@prarm x,y는 현재 마우스 포인터의 좌표값을 나타낸다.
*/
void MouseMotion(int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;

	if (g_left_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_ROTATION_Y_UP, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd);
		glutPostRedisplay();
	}
	else if (g_right_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_TRANS, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}