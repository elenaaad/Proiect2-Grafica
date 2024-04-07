//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul XI - 11_01_umbra.cpp |
// ====================================
// 
//	Program ce deseneaza o casa si care suprinde efectele de umbrire folosindu-se tehnicile MODERN OpenGL; 
//	Elemente de NOUTATE (modelul de iluminare):
//	 - pentru a genera umbra unei surse este utilizata o matrice 4x4;
//	 - in shaderul de varfuri este inclusa si matricea umbrei;
//	 - in shaderul de fragment umbra este colorata separat;
//	 - sursa de lumina este punctuala(varianta de sursa directionala este comentata);
// 
//  
// 
//	Biblioteci
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <math.h>			//	Biblioteca pentru calcule matematice;
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"


//  Identificatorii obiectelor de tip OpenGL; 
GLuint
VaoIdCylinder,
VboIdCylinder,
EboIdCylinder,
VaoIdGround,
VboIdGround,
EboIdGround,
VaoIdSun,
VboIdSun,
EboIdSun,
ColorBufferId,
ProgramId,
myMatrixLocation,
matrUmbraLocation,
viewLocation,
projLocation,
matrRotlLocation,
lightColorLocation,
lightPosLocation,
viewPosLocation,
codColLocation;

int codCol;
float PI = 3.141592;

// matrice utilizate
glm::mat4 myMatrix, matrRot;

// elemente pentru matricea de vizualizare
float Refx = 0.0f, Refy = 0.0f, Refz = 0.0f;
float alpha = PI / 8, beta = 0.0f, dist = 400.0f;
float Obsx, Obsy, Obsz;
float Vx = 0.0, Vy = 0.0, Vz = 1.0;
glm::mat4 view;
float const U_MIN = -PI / 2, U_MAX = PI / 2, V_MIN = 0, V_MAX = 2 * PI;
// elemente pentru matricea de proiectie
float width = 800, height = 600, xwmin = -800.f, xwmax = 800, ywmin = -600, ywmax = 600, znear = 0.1, zfar = 1, fov = 45;
glm::mat4 projection;
int const NR_PARR = 10, NR_MERID = 20;
// sursa de lumina
float xL = 450.f, yL = 100.f, zL = 1000.f;
int index, index_aux;
// matricea umbrei
float matrUmbra[4][4];
float step_u = (U_MAX - U_MIN) / NR_PARR, step_v = (V_MAX - V_MIN) / NR_MERID;
float radius = 50;
void processNormalKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'l':
		Vx -= 0.1;
		break;
	case 'r':
		Vx += 0.1;
		break;
	case '=':
		dist += 5;
		break;
	case '-':
		dist -= 5;
		break;
	}
	if (key == 27)
		exit(0);
}
void processSpecialKeys(int key, int xx, int yy)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += 0.01;
		break;
	case GLUT_KEY_DOWN:
		alpha -= 0.01;
		break;
	}
}

void CreateSun(void)
{
	// SFERA
	// Matricele pentru varfuri, culori, indici
	glm::vec4 Vertices1[(NR_PARR + 1) * NR_MERID];
	glm::vec3 Colors1[(NR_PARR + 1) * NR_MERID];
	glm::vec3 Normals[(NR_PARR + 1) * NR_MERID];
	GLushort Indices1[2 * (NR_PARR + 1) * NR_MERID + 4 * (NR_PARR + 1) * NR_MERID];
	for (int merid = 0; merid < NR_MERID; merid++)
	{
		for (int parr = 0; parr < NR_PARR + 1; parr++)
		{
			// implementarea reprezentarii parametrice 
			float u = U_MIN + parr * step_u; // valori pentru u si v
			float v = V_MIN + merid * step_v;
			float x_vf = radius * cosf(u) * cosf(v) + 400.0f; // coordonatele varfului corespunzator lui (u,v)
			float y_vf = radius * cosf(u) * sinf(v) + 75.0f;
			float z_vf = radius * sinf(u) + 950.0f;

			// identificator ptr varf; coordonate + culoare + indice la parcurgerea meridianelor
			index = merid * (NR_PARR + 1) + parr;
			Vertices1[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Colors1[index] = glm::vec3(1.0f, 1.0f, 0.0f);
			Indices1[index] = index;
			Normals[index] = glm::vec3(x_vf, y_vf, z_vf);
			// indice ptr acelasi varf la parcurgerea paralelelor
			index_aux = parr * (NR_MERID)+merid;
			Indices1[(NR_PARR + 1) * NR_MERID + index_aux] = index;

			// indicii pentru desenarea fetelor, pentru varful curent sunt definite 4 varfuri
			if ((parr + 1) % (NR_PARR + 1) != 0) // varful considerat sa nu fie Polul Nord
			{
				int AUX = 2 * (NR_PARR + 1) * NR_MERID;
				int index1 = index; // varful v considerat
				int index2 = index + (NR_PARR + 1); // dreapta lui v, pe meridianul urmator
				int index3 = index2 + 1;  // dreapta sus fata de v
				int index4 = index + 1;  // deasupra lui v, pe acelasi meridian
				if (merid == NR_MERID - 1)  // la ultimul meridian, trebuie revenit la meridianul initial
				{
					index2 = index2 % (NR_PARR + 1);
					index3 = index3 % (NR_PARR + 1);
				}
				Indices1[AUX + 4 * index] = index1;  // unele valori ale lui Indices, corespunzatoare Polului Nord, au valori neadecvate
				Indices1[AUX + 4 * index + 1] = index2;
				Indices1[AUX + 4 * index + 2] = index3;
				Indices1[AUX + 4 * index + 3] = index4;
			}
		}
	};


	///

	// generare VAO/buffere
	glGenVertexArrays(1, &VaoIdSun);
	glBindVertexArray(VaoIdSun);
	glGenBuffers(1, &VboIdSun); // atribute
	glGenBuffers(1, &EboIdSun); // indici

	// legare+"incarcare" buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboIdSun);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1) + sizeof(Colors1) + sizeof(Normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices1), Vertices1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices1), sizeof(Colors1), Colors1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices1) + sizeof(Colors1), sizeof(Normals), Normals);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdSun);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices1), Indices1, GL_STATIC_DRAW);

	// atributele; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glEnableVertexAttribArray(1); // atributul 1 = culoare
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices1)));
	glEnableVertexAttribArray(2); // atributul 2 = normala
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices1) + sizeof(Colors1)));
}
void CreateGround(void)
{

	// varfurile 
	GLfloat GroundAndSkyVertices[] =
	{
		//	// coordonate                   // culori			// normale
			// varfuri "ground"
			-400500.0f, -400500.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.1f,   0.0f, 1.0f,
			400500.0f, -400500.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 0.1f,   0.0f, 1.0f,
			400500.0f, 400500.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 0.1f,   0.0f, 1.0f,
			-400500.0f, 400500.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 0.1f,   0.0f, 1.0f,


			-400500.0f, -400500.0f, 1000.0f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
			400500.0f, -400500.0f, 1000.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
			400500.0f, 400500.0f, 1000.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
			-400500.0f, 400500.0f, 1000.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,


	};

	//// indicii pentru varfuri
	GLubyte GroundAndSkyIndices[] =
	{
		// fetele "ground"
		 1, 2, 0,   2, 0, 3,
		 5, 6, 4,   6, 4, 7

	};

	glGenVertexArrays(1, &VaoIdGround);
	glGenBuffers(1, &VboIdGround);
	glGenBuffers(1, &EboIdGround);
	glBindVertexArray(VaoIdGround);

	glBindBuffer(GL_ARRAY_BUFFER, VboIdGround);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GroundAndSkyVertices), GroundAndSkyVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdGround);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GroundAndSkyIndices), GroundAndSkyIndices, GL_STATIC_DRAW);

	// atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)0);
	// atributul 1 = culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	// atributul 2 = normale
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
}
void CreateCylinder(void)
{
	GLfloat Vertices[] =
	{
		// varfuri cub 
		 40.0f,  670.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		 90.0f,  670.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		 90.0f,  720.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		 40.0f,  720.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		 40.0f,  670.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		 90.0f,  670.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17 ft
		 90.0f,  720.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18 ft
		 40.0f,  720.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19            7 ARC 4 


		 // varfuri cub 
		  -60.0f,  670.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos //0
		  -10.0f,  670.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //1
		  -10.0f,  720.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata //2
		  -60.0f,  720.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate //3
		  -60.0f,  670.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sp sus //4
		  -10.0f,  670.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //5  fata  spre mn
		  -10.0f,  720.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //6
		  -60.0f,  720.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //sp 7             15      ARC 4 



		  // varfuri cub 
		  -160.0f,  630.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos //4
		  -110.0f,  630.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5
		  -110.0f,  680.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata
		  -160.0f,  680.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate
		  -160.0f,  630.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus
		  -110.0f,  630.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f,
		  -110.0f,  680.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f,
		  -160.0f,  680.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //                 23

		  -265.0f,  565.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  -215.0f,  565.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  -215.0f,  615.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -265.0f,  615.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -265.0f,  565.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		  -215.0f,  565.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
		  -215.0f,  615.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
		  -265.0f,  615.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19                 31  

		  -335.0f,  465.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  -285.0f,  465.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  -285.0f,  515.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -335.0f,  515.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -335.0f,  465.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		  -285.0f,  465.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
		  -285.0f,  515.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
		  -335.0f,  515.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19              39



		  -365.0f,  345.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  -315.0f,  345.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  -315.0f,  395.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -365.0f,  395.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -365.0f,  345.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //st sp sus //8 16
		  -315.0f,  345.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //st ft sus9 17
		  -315.0f,  395.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //dr ft sus10 18
		  -365.0f,  395.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //dr sp sus 11 19              47 arc1

		  -355.0f,  235.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  -305.0f,  235.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  -305.0f,  285.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -355.0f,  285.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -355.0f,  235.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		  -305.0f,  235.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
		  -305.0f,  285.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
		  -355.0f,  285.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19             55   arc1



		  -315.0f,  125.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  -265.0f,  125.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  -265.0f,  175.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -315.0f,  175.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -315.0f,  125.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		  -265.0f,  125.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
		  -265.0f,  175.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
		  -315.0f,  175.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19              63

		  // varfuri cub 
		  -10.0f,  450.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		  40.0f,  450.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		  40.0f,  500.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		  -10.0f,  500.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		  -10.0f,  450.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //st sp sus //8 16
		  40.0f,  450.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //st ft 9 17
		  40.0f,  500.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //dr ft10 18
		  -10.0f,  500.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //dr sp 11 19            71  ARC 2


		  // varfuri cub 
		   -80.0f,  470.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos //0
		   -30.0f,  470.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //1
		   -30.0f,  520.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata //2
		   -80.0f,  520.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate //3
		   -80.0f,  470.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //4
		   -30.0f,  470.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //5
		   -30.0f,  520.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //6
		   -80.0f,  520.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //7             79 ARC 2
			


		   // varfuri cub 
		   95.0f,  385.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
		   145.0f,  385.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
		   145.0f,  435.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
		   95.0f,  435.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
		   95.0f,  385.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
		   145.0f,  385.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
		   145.0f,  435.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
		   95.0f,  435.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19            87

		   // varfuri cub 
	   150.0f,  315.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
	   200.0f,  315.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
	   200.0f,  365.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
	   150.0f,  365.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
	   150.0f,  315.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
	   200.0f,  315.0f,  200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
	   200.0f,  365.0f,  200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
	   150.0f,  365.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19            95  


	  190.0f, 205.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
	  240.0f, 205.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
	  240.0f, 155.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
	  190.0f, 155.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
	  190.0f, 205.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
	  240.0f, 205.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17
	  240.0f, 155.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18
	  190.0f, 155.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19 


	  // varfuri cub 
	  40.0f, -35.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
	  90.0f, -35.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
	  90.0f, -85.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
	  40.0f, -85.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
	  40.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
	  90.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //fata9 17
	  90.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, // fata10 18                 
	  40.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19            7  ARC 3


	  // varfuri cub 
	  -60.0f, -35.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos //0
	  -10.0f, -35.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //1
	  -10.0f, -85.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata //2
	  -60.0f, -85.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate //3
	  -60.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //4
	  -10.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //5
	  -10.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //6                    
	  -60.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //7             15  ARC 3


305.0f,  465.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
355.0f,  465.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
355.0f,  515.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
305.0f,  515.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
305.0f,  465.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
355.0f,  465.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
355.0f,  515.0f, 200.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
305.0f,  515.0f, 200.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19              39 



310.0f, 565.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
380.0f, 565.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
380.0f, 615.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
310.0f, 615.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
310.0f, 565.0f, 40.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
380.0f, 565.0f, 40.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17
380.0f, 615.0f, 40.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18
310.0f, 615.0f, 40.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19                 31                 


// varfuri cub 
  80.0f, 100.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12  144
  130.0f, 100.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
  130.0f, 50.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
  80.0f, 50.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
  80.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
  130.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17
  130.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18
  80.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19            7   ARC 5


  // varfuri cub 
  -20.0f, 100.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12  144
  30.0f, 100.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
  30.0f, 50.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
  -20.0f, 50.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
  -20.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
  30.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17  
  30.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18     spre mine stg
  -20.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19            ARC 5





  // varfuri cub 
	140.0f,  630.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos //4
	190.0f,  630.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5
	190.0f,  680.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata
	140.0f,  680.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate
	140.0f,  630.0f, 70.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus
	190.0f,  630.0f,  70.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f,
	190.0f,  680.0f,  70.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f,
	140.0f,  680.0f, 70.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //                 23

	245.0f,  565.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
	295.0f,  565.0f,  0.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, -1.0f, -1.0f, //stg fata //5 13
	295.0f,  615.0f,  0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  1.0f, 1.0f, -1.0f, //dr fata // 6 14
	245.0f,  615.0f, 0.0f, 1.0f,    0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, -1.0f, //dr spate // 7 15
	245.0f,  565.0f, 40.0f, 1.0f,  0.5f, 0.5f, 0.5f,  -1.0f, -1.0f, 1.0f, //sus //8 16
	295.0f,  565.0f,  40.0f, 1.0f, 0.5f, 0.5f, 0.5f,  1.0f, -1.0f, 1.0f, //9 17
	295.0f,  615.0f,  40.0f, 1.0f,  0.5f, 0.5f, 0.5f,  1.0f, 1.0f, 1.0f, //10 18
	245.0f,  615.0f, 40.0f, 1.0f,   0.5f, 0.5f, 0.5f,  -1.0f, 1.0f, 1.0f, //11 19                 31

	345.0f, 345.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, -1.0f, //stg spate jos // 4 12
	370.0f, 345.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, -1.0f, //stg fata //5 13
	370.0f, 395.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, //dr fata // 6 14
	345.0f, 395.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, -1.0f, //dr spate // 7 15
	345.0f, 345.0f, 100.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //8 16
	370.0f, 345.0f, 100.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17
	370.0f, 395.0f, 100.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18
	345.0f, 395.0f, 100.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //11 19              47

	//arc 1
		  -315.0f, 395.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 
		  -365.0f, 395.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, 
		  -355.0f, 235.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
		  -305.0f, 235.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f,
		  -315.0f, 395.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
		  -365.0f, 395.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f,
		  -355.0f, 235.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
		  -305.0f, 235.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f,

		  // ARC 2
		  40.0f, 450.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //ft
		  40.0f, 500.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //ft
		  -80.0f, 470.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //4
		  -80.0f, 520.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //7        
		  40.0f, 450.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //ft
		  40.0f, 500.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //ft
		  -80.0f, 470.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sus //4
		  -80.0f, 520.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //7        

		  //ARC 3 

		  -60.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
		  90.0f, -35.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f,
		  90.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
		  -60.0f, -85.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f,
		  -60.0f, -35.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
		  90.0f, -35.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, 
		  90.0f, -85.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
		  -60.0f, -85.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f,

		  //ARC 4
	-60.0f, 670.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sp 
	90.0f, 670.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17 ft
	90.0f, 720.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18 ft
	-60.0f, 720.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //sp 7    
	-60.0f, 670.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f, //sp 
	90.0f, 670.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f, //9 17 ft
	90.0f, 720.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, //10 18 ft
	-60.0f, 720.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f, //sp 7    

	//ARC 5 spate 2 , fata 1	
	-20.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
	130.0f, 100.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f,
	130.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
	-20.0f, 50.0f, 200.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f,
	-20.0f, 100.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, -1.0f, 1.0f,
	130.0f, 100.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, -1.0f, 1.0f,
	130.0f, 50.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
	-20.0f, 50.0f, 250.0f, 1.0f, 0.5f, 0.5f, 0.5f, -1.0f, 1.0f, 1.0f,

	};


	// indicii pentru varfuri
	GLubyte Indices[] =
	{
		// fetele cubului
	   1, 2, 0,  2, 0, 3,
	   2, 3, 6,  6, 3, 7,
	   7, 3, 4,  4, 3, 0,
	   4, 0, 5,  5, 0, 1,
	   1, 2, 5,  5, 2, 6,
	   5, 6, 4,  4, 6, 7,


	   9, 10, 8,  10, 8, 11,
	   10, 11, 14,  14, 11, 15,
	   15, 11, 12,  12, 11, 8,
	   12, 8, 13,  13, 8, 9,
	   9, 10, 13,  13, 10, 14,
	   13, 14, 12,  12, 14, 14,


	   17, 18, 16,  18, 16, 19,
	   18, 19, 22,  22, 19, 23,
	   23, 19, 20,  20, 19, 16,
	   20, 16, 21,  21, 16, 17,
	   17, 18, 21,  21, 18, 22,
	   21, 22, 20,  20, 22, 22,

	   25, 26, 24,  26, 24, 27,
	   26, 27, 30,  30, 27, 31,
	   31, 27, 28,  28, 27, 24,
	   28, 24, 29,  29, 24, 25,
	   25, 26, 29,  29, 26, 30,
	   29, 30, 28,  28, 30, 31,

	   33, 34, 32,  34, 32, 35,
	   34, 35, 38,  38, 35, 39,
	   39, 35, 36,  36, 35, 32,
	   36, 32, 37,  37, 32, 33,
	   33, 34, 37,  37, 34, 38,
	   37, 38, 36,  36, 38, 39,

	   41, 42, 40,  42, 40, 43,
	   42, 43, 46,  46, 43, 47,
	   47, 43, 44,  44, 43, 40,
	   44, 40, 45,  45, 40, 41,
	   41, 42, 45,  45, 42, 46,
	   45, 46, 44,  44, 46, 47,

	   48, 49, 50,  50, 51, 48,
	   51, 50, 54,  54, 55, 51,
	   51, 55, 52,  52, 48, 51,
	   48, 49, 53,  53, 52, 48,
	   49, 50, 54,  54, 53, 49,
	   52, 53, 54,  54, 55, 52,

	   56, 57, 58,  58, 59, 56,
	   59, 58, 62,  62, 63, 59,
	   56, 59, 63,  63, 60, 56,
	   56, 57, 61,  61, 60, 56,
	   57, 58, 62,  62, 61, 57,
	   60, 61, 62,  62, 63, 60,

	   64, 65, 66,  66, 67, 64,
	   67, 66, 70,  70, 71, 67,
	   64, 67, 71,  71, 68, 64,
	   64, 65, 69,  69, 68, 64,
	   65, 66, 70,  70, 69, 65,
	   68, 69, 70,  70, 71, 68,

	   72, 73, 74,  74, 75, 72,
	   75, 74, 78,  78, 79, 75,
	   72, 75, 79,  79, 76, 72,
	   72, 73, 77,  77, 76, 72,
	   73, 74, 78,  78, 77, 73,
	   76, 77, 78,  78, 79, 76,

	   80, 81, 82,  82, 83, 80,
	   83, 82, 86,  86, 87, 83,
	   80, 83, 87,  87, 84, 80,
	   80, 81, 85,  85, 84, 80,
	   81, 82, 86,  86, 85, 81,
	   84, 85, 86,  86, 87, 84,

	   88, 89, 90,  90, 91, 88,
	   91, 90, 94,  94, 95, 91,
	   88, 91, 95,  95, 92, 88,
	   88, 89, 93,  93, 92, 88,
	   89, 90, 94,  94, 93, 89,
	   92, 93, 94,  94, 95, 92,

	   96, 97, 98, 98, 99, 96,
	   99, 98, 102, 102, 103, 99,
	   96, 99, 103, 103, 100, 96,
	   96, 97, 101, 101, 100, 96,
	   97, 98, 102, 102, 101, 97,
	   100, 101, 102, 102, 103, 100,


	   104, 105, 106, 106, 107, 104,
	   107, 106, 110, 110, 111, 107,
	   104, 107, 111, 111, 108, 104,
	   104, 105, 109, 109, 108, 104,
	   105, 106, 110, 110, 109, 105,
	   108, 109, 110, 110, 111, 108,


	   112, 113, 114, 114, 115, 112,
	   115, 114, 118, 118, 119, 115,
	   112, 115, 119, 119, 116, 112,
	   112, 113, 117, 117, 116, 112,
	   113, 114, 118, 118, 117, 113,
	   116, 117, 118, 118, 119, 116,



	   120, 121, 122, 122, 123, 120,
	   123, 122, 126, 126, 127, 123,
	   120, 123, 127, 127, 124, 120,
	   120, 121, 125, 125, 124, 120,
	   121, 122, 126, 126, 125, 121,
	   124, 125, 126, 126, 127, 124,

	   128, 129, 130, 130, 131, 128,
	   131, 130, 134, 134, 135, 131,
	   128, 131, 135, 135, 132, 128,
	   128, 129, 133, 133, 132, 128,
	   129, 130, 134, 134, 133, 129,
	   132, 133, 134, 134, 135, 132,

	   136, 137, 138, 138, 139, 136,
	   139, 138, 142, 142, 143, 139,
	   136, 139, 143, 143, 140, 136,
	   136, 137, 141, 141, 140, 136,
	   137, 138, 142, 142, 141, 137,
	   140, 141, 142, 142, 143, 140,

	   144, 145, 146,  146, 147, 144,
	   147, 146, 150,  150, 151, 147,
	   144, 147, 151,  151, 148, 144,
	   144, 145, 149,  149, 148, 144,
	   145, 146, 150,  150, 149, 145,
	   148, 149, 150,  150, 151, 148,

	   152, 153, 154,  154, 155, 152,
	   155, 154, 158,  158, 159, 155,
	   152, 155, 159,  159, 156, 152,
	   152, 153, 157,  157, 156, 152,
	   153, 154, 158,  158, 157, 153,
	   156, 157, 158,  158, 159, 156,

	   160, 161, 162,  162, 163, 160,
	   163, 162, 166,  166, 167, 163,
	   160, 163, 167,  167, 164, 160,
	   160, 161, 165,  165, 164, 160,
	   161, 162, 166,  166, 165, 161,
	   164, 165, 166,  166, 167, 164,

	   168, 169, 170,  170, 171, 168,
	   171, 170, 174,  174, 175, 171,
	   168, 171, 175,  175, 172, 168,
	   168, 169, 173,  173, 172, 168,
	   169, 170, 174,  174, 173, 169,
	   172, 173, 174,  174, 175, 172,

	   //fete arc1 corect 

		176, 177, 178, 178, 179, 176,
		179, 178, 182, 182, 183, 179,
		176, 179, 183, 183, 180, 176,
		176, 177, 181, 181, 180, 176,
		177, 178, 182, 182, 181, 177,
		180, 181, 182, 182, 183, 180,

		//FETE ARC 2

		184, 185, 186, 186, 187, 184,
		187, 186, 190, 190, 191, 187,
		184, 187, 191, 191, 188, 184,
		184, 185, 189, 189, 188, 184,
		185, 186, 190, 190, 189, 185,
		188, 189, 190, 190, 191, 188,

		//FETE ARC 3

	   192, 193, 194, 194, 195, 192,
	   195, 194, 198, 198, 199, 195,
	   192, 195, 199, 199, 196, 192,
	   192, 193, 197, 197, 196, 192,
	   193, 194, 198, 198, 197, 193,
	   196, 197, 198, 198, 199, 196,

	   //FETE ARC 4

		   200, 201, 202, 202, 203, 200,
		   203, 202, 206, 206, 207, 203,
		   200, 203, 207, 207, 204, 200,
		   200, 201, 205, 205, 204, 200,
		   201, 202, 206, 206, 205, 201,
		   204, 205, 206, 206, 207, 204,

		   //FETE ARC 5

		   208, 209, 210, 210, 211, 208,
		   211, 210, 214, 214, 215, 211,
		   208, 211, 215, 215, 212, 208,
		   208, 209, 213, 213, 212, 208,
		   209, 210, 214, 214, 213, 209,
		   212, 213, 214, 214, 215, 212





	};

	glGenVertexArrays(1, &VaoIdCylinder);
	glGenBuffers(1, &VboIdCylinder);
	glGenBuffers(1, &EboIdCylinder);
	glBindVertexArray(VaoIdCylinder);

	glBindBuffer(GL_ARRAY_BUFFER, VboIdCylinder);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdCylinder);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)0);
	// atributul 1 = culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	// atributul 2 = normale
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
	
}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboIdCylinder);
	glDeleteBuffers(1, &EboIdCylinder);
	glDeleteBuffers(1, &VboIdGround);
	glDeleteBuffers(1, &EboIdGround);
	glDeleteBuffers(1, &VboIdSun);
	glDeleteBuffers(1, &EboIdSun);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoIdCylinder);
	glDeleteVertexArrays(1, &VaoIdGround);
	glDeleteVertexArrays(1, &VaoIdSun);

}

void CreateShaders(void)
{
	ProgramId = LoadShaders("11_01_Shader.vert", "11_01_Shader.frag");
	glUseProgram(ProgramId);
}

void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	myMatrix = glm::mat4(1.0f);
	matrRot = glm::rotate(glm::mat4(1.0f), PI / 8, glm::vec3(0.0, 0.0, 1.0));
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	CreateCylinder();
	CreateGround();
	CreateShaders();
	CreateSun();
	// locatii pentru shader-e
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	matrUmbraLocation = glGetUniformLocation(ProgramId, "matrUmbra");
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	lightColorLocation = glGetUniformLocation(ProgramId, "lightColor");
	lightPosLocation = glGetUniformLocation(ProgramId, "lightPos");
	viewPosLocation = glGetUniformLocation(ProgramId, "viewPos");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
}
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//pozitia observatorului
	Obsx = Refx + dist * cos(alpha) * cos(beta);
	Obsy = Refy + dist * cos(alpha) * sin(beta);
	Obsz = Refz + dist * sin(alpha);

	// matrice de vizualizare + proiectie
	glm::vec3 Obs = glm::vec3(Obsx, Obsy, Obsz);   // se schimba pozitia observatorului	
	glm::vec3 PctRef = glm::vec3(Refx, Refy, Refz); // pozitia punctului de referinta
	glm::vec3 Vert = glm::vec3(Vx, Vy, Vz); // verticala din planul de vizualizare 
	view = glm::lookAt(Obs, PctRef, Vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	// matricea pentru umbra
	float D = -0.5f;
	matrUmbra[0][0] = zL + D; matrUmbra[0][1] = 0; matrUmbra[0][2] = 0; matrUmbra[0][3] = 0;
	matrUmbra[1][0] = 0; matrUmbra[1][1] = zL + D; matrUmbra[1][2] = 0; matrUmbra[1][3] = 0;
	matrUmbra[2][0] = -xL; matrUmbra[2][1] = -yL; matrUmbra[2][2] = D; matrUmbra[2][3] = -1;
	matrUmbra[3][0] = -D * xL; matrUmbra[3][1] = -D * yL; matrUmbra[3][2] = -D * zL; matrUmbra[3][3] = zL;
	glUniformMatrix4fv(matrUmbraLocation, 1, GL_FALSE, &matrUmbra[0][0]);

	// Variabile uniforme pentru iluminare
	glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLocation, xL, yL, zL);
	glUniform3f(viewPosLocation, Obsx, Obsy, Obsz);

	glBindVertexArray(VaoIdSun);
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++)
	{
		if ((patr + 1) % (NR_PARR + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort)));
	}


	glBindVertexArray(VaoIdGround);
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	///
	glBindBuffer(GL_ARRAY_BUFFER, VboIdGround);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdGround);

	///
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)6);

	//cilindru
	glBindVertexArray(VaoIdCylinder);
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(0));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(36));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(72));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(108));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(144));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(180));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(216));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(252));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(288));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(324));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(360));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(396));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(432));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(468));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(504));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(540));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(576));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(612));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(648));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(684));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(720));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(756));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(792));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(828));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(864));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(900));


	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(936));



	//UMBRE*****************************************************

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(0));

	// desenare umbra cub 2
	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(36));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(72));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(108));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(144));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(180));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(216));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(252));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(288));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(324));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(360));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(396));


	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(432));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(468));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(504));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(540));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(576));


	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(612));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(648));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(684));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(720));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(756));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(792));


	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(828));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(864));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(900));

	codCol = 1;
	glUniform1i(codColLocation, codCol);
	myMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)(936));

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_BYTE, (void*)(42));


	glutSwapBuffers();
	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}
///
void AssociateAttributePointers()
{
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
}
///
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 900);
	glutCreateWindow("Iluminare - Umbre - OpenGL <<nou>>");
	glewInit();
	Initialize();
	glutIdleFunc(RenderFunction);
	glutDisplayFunc(RenderFunction);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}