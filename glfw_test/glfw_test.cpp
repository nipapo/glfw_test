// glfw_test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Window.h"
#include "Shape.h"

// 矩形の頂点の位置
constexpr Object::Vertex rectangleVertex[] =
{
	{ -0.5f, -0.5f },
	{  0.5f, -0.5f },
	{  0.5f,  0.5f },
	{ -0.5f,  0.5f }
};

GLboolean printShaderInfoLog(GLuint shader, const char* str)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) std::cerr << "Compile Error in " << str << std::endl;

	GLsizei bufSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1)
	{
		std::vector<GLchar> infoLog(bufSize);
		GLsizei length;
		glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
		std::cerr << &infoLog[0] << std::endl;
	}
	return static_cast<GLboolean>(status);
}

GLboolean printProgramInfoLog(GLuint program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) std::cerr << "Link Error." << std::endl;

	GLsizei bufSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1)
	{
		std::vector<GLchar> infoLog(bufSize);
		GLsizei length;
		glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
		std::cerr << &infoLog[0] << std::endl;
	}
	return static_cast<GLboolean>(status);
}





// プログラムオブジェクトを作成する
//vsrc: バーテックスシェーダのソースプログラムの文字列
//pv: バーテックスシェーダのソースプログラム中の in 変数名の文字列
//fsrc: フラグメントシェーダのソースプログラムの文字列
//fc: フラグメントシェーダのソースプログラム中の out 変数名の文字列
GLuint createProgram(const char * vsrc, const char * pv,const char * fsrc, const char * fc) 
{
	// 空のプログラムオブジェクトを作成する
	const GLuint program(glCreateProgram());

	if (vsrc != NULL) 
	{
		// バーテックスシェーダのシェーダオブジェクトを作成する
		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsrc, NULL);
		glCompileShader(vobj);
		
		// バーテックスシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
		if (printShaderInfoLog(vobj,"vertex shader"))
			glAttachShader(program, vobj);
		glDeleteShader(vobj);
	}

	if (fsrc != NULL)
	{
		// フラグメントシェーダのシェーダオブジェクトを作成する
		const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fobj, 1, &fsrc, NULL);
		glCompileShader(fobj);

		// フラグメントシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
		if (printShaderInfoLog(fobj,"fragment shader"))
			glAttachShader(program, fobj);
		glDeleteShader(fobj);
	}

	// プログラムオブジェクトをリンクする
	glBindAttribLocation(program, 0, pv);
	glBindFragDataLocation(program, 0, fc);
	glLinkProgram(program);

	// 作成したプログラムオブジェクトを返す
	if (printProgramInfoLog(program))
		return program;
	glDeleteProgram(program);
	return 0;
}


GLchar* readShaderSource(const char* name) 
{
	// ファイル名が NULL なら NULL を返す
	if (name == NULL) return NULL;

	// ソースファイルを開く
	std::ifstream file(name, std::ios::binary);
	if (file.fail())
	{
		// 開けなかった
		std::cerr << "Error: Can't open source file: " << name << std::endl;
		return NULL;
	}
	// ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
	file.seekg(0L, std::ios::end);
	const GLsizei length(static_cast<GLsizei>(file.tellg()));

	// ファイルサイズ＋文字列の終端文字 (‘¥0’) 分のメモリを確保
	GLchar *buffer(new(std::nothrow) GLchar[length + 1]);
	if (buffer == NULL)
	{
		// メモリが足らなかった
		std::cerr << "Error: Too large file: " << name << std::endl;
		file.close();
		return NULL;
	}
	// ファイルを先頭から読み込む
	file.seekg(0L, std::ios::beg);
	file.read(buffer, length);
	buffer[length] = 0;
//	buffer[length] = '¥0';

	if (file.bad()) {
		// うまく読み込めなかった
		std::cerr << "Error: Could not read souce file: " << name << std::endl;
		delete[] buffer;
		buffer = NULL;
	}
	file.close();

	//ソースプログラムを読み込んだメモリのポインタを返す
	return buffer;
}

GLuint loadProgram(const char* vert, const char* pv,const char* frag, const char* fc) 
{
	// シェーダのソースファイルを読み込む
	const GLchar* vsrc(readShaderSource(vert));
	const GLchar* fsrc(readShaderSource(frag));

	// プログラムオブジェクトを作成する
	const GLuint program(createProgram(vsrc, pv, fsrc, fc));

	// 読み込みに使ったメモリを解放する
	delete vsrc;
	delete fsrc;

	// 作成したプログラムオブジェクトを返す
	return program;
}

int main()
{
	if (glfwInit() == GL_FALSE)
	{
		std::cerr << "Can't initilize GLFW" << std::endl;
		return 1;
	}
	// プログラム終了時の処理を登録する
	atexit(glfwTerminate);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ウィンドウを作成する
	Window window;

	// 背景色を指定する
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	
	// ビューポートを設定する
//	glViewport(100, 50, 300, 300);

	// プログラムオブジェクトを作成する
	const GLuint program(loadProgram("point.vert", "pv", "point.frag", "fc"));

	// uniform 変数の場所を取得する
	const GLint sizeLoc(glGetUniformLocation(program, "size"));
	const GLint scaleLoc(glGetUniformLocation(program, "scale"));
	const GLint locationLoc(glGetUniformLocation(program, "location"));

	// 図形データを作成する
	std::unique_ptr<const Shape> shape(new Shape(2, 4, rectangleVertex));

	while (window)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		// シェーダプログラムの使用開始
		glUseProgram(program);

		// uniform 変数に値を設定する
		glUniform2fv(sizeLoc, 1, window.getSize());
		glUniform1f(scaleLoc, window.getScale());
		glUniform2fv(locationLoc, 1, window.getLocation());

		// 図形を描画する
		shape->draw();

		// カラーバッファを入れ替える
		window.swapBuffers();
	}
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
