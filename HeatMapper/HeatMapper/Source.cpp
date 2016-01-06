#include "Header.h"

using namespace std;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 ourcolor;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"ourcolor = color;\n"
"}\n\0";

/* ----- GLOBAL VARIBALES ----- */



/* --- END GLOBAL VARIBALES --- */

void loadPlayerData() //Get position data from file and store in an array
{
	string filename;
	

	cout << "Enter file to load: (\"positions.txt\", \"positions2.txt\", \"positions3.txt\")" << endl;
	cin >> filename;
	cout << endl;

	ifstream posFile(filename);

	if (!posFile)
	{
		cout << "ERROR LOADING POSITION DATA - Ensure that the file is stored in the Heatmap subdirectory" << endl;
		return;
	}

	cout << "How many players were present in this game?" << endl;
	cin >> maxPlayers;
	cout << endl;
	
	//Set some defaults if invalid data was entered
	if (maxPlayers > 8)
		maxPlayers = 8;
	else if (maxPlayers < 1)
		maxPlayers = 1;

	int lineCounter;
	int clientNum;

	float xPos;
	float yPos;
	float zPos;

	cout << "Loading position data from file..." << endl;

	while (true)
	{
		lineCounter = 0;

		if (posFile.eof()) break;

		for (string line; getline(posFile, line);)
		{
			istringstream in(line);
			string tag;
			in >> tag;

			if (tag == "Player")
			{

				in >> clientNum;
			}

			if (tag == "Position")
			{
				in >> xPos;
				in >> yPos;
				in >> zPos;

				//Normalisation of vectors
				/*xPos = xPos / sqrt((xPos*xPos) + (yPos*yPos) + (zPos*zPos));
				yPos = yPos / sqrt((xPos*xPos) + (yPos*yPos) + (zPos*zPos));
				zPos = zPos / sqrt((xPos*xPos) + (yPos*yPos) + (zPos*zPos));*/

				//Normalize coordinates
				xPos = xPos / 1250 - 0.4f;
				yPos = yPos / 1250 - 0.6f;
				zPos = zPos / sqrt((xPos*xPos) + (yPos*yPos) + (zPos*zPos));

				playerPositions[clientNum][lineCounter] = glm::vec3(xPos, yPos, zPos);

				if (clientNum == (maxPlayers - 1))
				{
					lineCounter++; //Change lineCounter after each frame (i.e. all player positions logged)
					
				}
			}

			if (posFile.eof())
			{
				cout << "File loaded." << endl;

				posFile.close();
			}
		}
	}
}

void generateGrid()
{
	int squareNum = 0;
	
	cout << "Generating visualisation..." << endl;

	for (int row = 0; row < SqPer; row++)
	{
		for (int column = 0; column < SqPer; column++)
		{
			GLfloat vertices[] =
			{
				-0.99f + ((float)row * 0.01f), -0.99f + ((float)column * 0.01f), 0.0f,  // Top Right
				-0.99f + ((float)row * 0.01f), -1.0f + ((float)column * 0.01f), 0.0f,  // Bottom Right
				-1.0f + ((float)row * 0.01f), -1.0f + ((float)column * 0.01f), 0.0f,  // Bottom Left
				-1.0f + ((float)row * 0.01f), -0.99f + ((float)column * 0.01f), 0.0f   // Top Left 
			};

			// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
			/* Heatmap */
			glBindVertexArray(VAO[squareNum]);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[squareNum]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[squareNum]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
			/* Heatmap */

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

			float tempXSmallest = 0.0f;
			float tempYSmallest = 0.0f;
			float tempXLargest = 0.0f;
			float tempYLargest = 0.0f;
			int vertNum = 0;

			for (int player = 0; player < playerNum; player++)
			{
				for (int frame = 0; frame < 27980; frame++)
				{
					tempXLargest = vertices[vertNum];
					tempYLargest = vertices[vertNum + 1];
					tempXSmallest = vertices[vertNum + 6];
					tempYSmallest = vertices[vertNum + 7];

					if (playerPositions[player][frame].x > tempXSmallest && playerPositions[player][frame].x < tempXLargest)
					{
						if (playerPositions[player][frame].y > tempYSmallest && playerPositions[player][frame].y < tempYLargest)
						{
							//Heatmap
							hitCount[row][column] += 1;
							
							//Trajectory
							entered[player][squareNum] = true;
						}
					}
				}
			}

			//Cleanup
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			squareNum++;
		}
	}

	cout << "Heatmap and trajectories generated." << endl << endl;
	cout << "To switch mode, press: " << endl << "\"h\" for Heatmap" << endl << "\"t\" for Trajectories" << endl << "\"b\" for Both" << endl;
	cout << "\"p\" cycles the players when visualising trajectories" << endl;
}

void runHeatmap()
{
	int squareNum = 0;

	//Loop through each square and check its 'hit' value
	for (int row = 0; row < SqPer; row++)
	{
		for (int column = 0; column < SqPer; column++)
		{
			if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
			else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
			else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
			else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
			else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
			else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
			else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
			else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White

			glBindVertexArray(VAO[squareNum]);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			squareNum++;
		}
	}
}

void runTrajectory()
{
	int squareNum = 0;

	if(p == 1)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[0][squareNum] == true))
				{
						glUniform4f(vertexColorLocation, 0.4f, 0.3f, 0.7f, 1.0f); //Player 1 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 2)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[1][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.8f, 0.5f, 0.5f, 1.0f); //Player 2 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 3)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[2][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.5f, 0.5f, 0.5f, 1.0f); //Player 3 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 4)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[3][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.6f, 0.6f, 1.0f, 1.0f); //Player 4 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 5)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[4][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.6f, 1.0f); //Player 5 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 6)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[5][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.6f, 0.7f, 1.0f, 1.0f); //Player 6 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 7)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[6][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.4f, 0.6f, 0.6f, 1.0f); //Player 7 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 8)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[7][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.5f, 1.0f, 0.5f, 1.0f); //Player 8 Colour
				}
				else
					glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 0.0f); //Black

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}
}

void runBoth()
{
	int squareNum = 0;

	if (p == 1)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[0][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.4f, 0.3f, 0.7f, 1.0f); //Player 1 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}
				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 2)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[1][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.8f, 0.5f, 0.5f, 1.0f); //Player 2 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 3)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[2][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.5f, 0.5f, 0.5f, 1.0f); //Player 3 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 4)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[3][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.6f, 0.6f, 1.0f, 1.0f); //Player 4 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 5)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[4][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.6f, 1.0f); //Player 5 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 6)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[5][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.6f, 0.7f, 1.0f, 1.0f); //Player 6 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 7)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[6][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.4f, 0.6f, 0.6f, 1.0f); //Player 7 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}

	if (p == 8)
	{
		for (int row = 0; row < SqPer; row++)
		{
			for (int column = 0; column < SqPer; column++)
			{
				if ((entered[7][squareNum] == true))
				{
					glUniform4f(vertexColorLocation, 0.5f, 1.0f, 0.5f, 1.0f); //Player 8 Colour
				}
				else
				{
					if ((hitCount[row][column]) == 0) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f); //Black
					else if (hitCount[row][column] == 1) glUniform4f(vertexColorLocation, 0.0f, 0.0f, 1.0f, 1.0f); //Blue
					else if (hitCount[row][column] > 1 && hitCount[row][column] < 5) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f); //Cyan
					else if (hitCount[row][column] > 4 && hitCount[row][column] < 10) glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); //Green
					else if (hitCount[row][column] > 9 && hitCount[row][column] < 16) glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); //Yellow
					else if (hitCount[row][column] > 15 && hitCount[row][column] < 23) glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.0f, 1.0f); //Orange
					else if (hitCount[row][column] > 22 && hitCount[row][column] < 31) glUniform4f(vertexColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); //Red
					else glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f); //White
				}

				glBindVertexArray(VAO[squareNum]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				squareNum++;
			}
		}
	}
}

int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Heatmap", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);


	// Build and compile our shader program
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Check for compile time errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	loadPlayerData();

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glGenVertexArrays(MaxBoxes, VAO);
	glGenBuffers(MaxBoxes, VBO);
	glGenBuffers(MaxBoxes, EBO);

	generateGrid();

	while (!glfwWindowShouldClose(window)) //Game loop
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);

		vertexColorLocation = glGetUniformLocation(shaderProgram, "color");

		if (viewMode == 0)
			runHeatmap();
		else if (viewMode == 1)
			runTrajectory();
		else
			runBoth();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(MaxBoxes, VAO);
	glDeleteBuffers(MaxBoxes, VBO);
	glDeleteBuffers(MaxBoxes, EBO);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	/* Camera */
	if (key == GLFW_KEY_UP)
		glViewport(camX, camY -= 6.0f, camWidth, camHeight);

	if (key == GLFW_KEY_DOWN)
		glViewport(camX, camY += 6.0f, camWidth, camHeight);

	if (key == GLFW_KEY_LEFT)
		glViewport(camX += 8.0f, camY, camWidth, camHeight);

	if (key == GLFW_KEY_RIGHT)
		glViewport(camX -= 8.0f, camY, camWidth, camHeight);

	//Zoom
	if (key == GLFW_KEY_Z)
		glViewport(camX -= 8.0f, camY -= 6.0f, camWidth += 16, camHeight += 12);
	
	if (key == GLFW_KEY_X)
	{
		if(camWidth > 1600 && camHeight > 1200) //User is unable to zoom out futher than the origin
			glViewport(camX += 8.0f, camY += 6.0f, camWidth -= 16, camHeight -= 12);
	}

	//Centre
	if (key == GLFW_KEY_C)
	{
		camX = 0.0f;
		camY = 0.0f;
		camWidth = WIDTH;
		camHeight = HEIGHT;

		glViewport(camX, camY, camWidth, camHeight);
	}

	/* End Camera */

	//Change mode
	//Zoom
	if (key == GLFW_KEY_H)
	{
		viewMode = 0; //Heatmap
	}

	if (key == GLFW_KEY_T)
	{
		viewMode = 1; //Trajectories
	}

	if (key == GLFW_KEY_B)
	{
		viewMode = 2; //Both
	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS && (viewMode == 1 || viewMode == 2))
	{
		if (p < maxPlayers)
			p++;
		else
			p = 1;
	}

	//Exit
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
