/*

        Copyright 2010 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Tutorial 01 - Create a window
*/

#include <GL/freeglut.h>
#include <stdio.h>

/*
	修改
	1. 平台工具集 Visual Studio 2017 (v141)   默认是v143
	2. Windows SDK 版本 10.0.17763.0  否则 stdio.h 找不到
	3. OpenCV32 下载安装 得到头文件和库 安装/解压目录在 D:\OpenCV32
	    https://sourceforge.net/projects/opencvlibrary/files/opencv-win/
		增加库搜索目录 D:\OpenCV32\build\x64\vc14\lib
		(头文件搜索目录 已经默认是 D:\OpenCV32\build\include 不用添加)

*/


static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);

    int width = 1920;
    int height = 1080;
    glutInitWindowSize(width, height);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Tutorial 01");
    printf("window id: %d\n", win);

    GLclampf Red = 0.0f, Green = 0.5f, Blue = 0.5f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glutDisplayFunc(RenderSceneCB);

    glutMainLoop();

    return 0;
}
