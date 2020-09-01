/*

	ofxWinMenu basic example - ofApp.cpp

	Example of using ofxWinMenu addon to create a menu for a Microsoft Windows application.
	
	Copyright (C) 2016-2017 Lynn Jarvis.

	https://github.com/leadedge

	http://www.spout.zeal.co

    =========================================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    =========================================================================

	03.11.16 - minor comment cleanup
	21.02.17 - rebuild for OF 0.9.8

*/

/*
(미로 생성: recursive bactracking 알고리즘을 이용, graph 자료구조만 이용하여 미로 정보(경로) 저장
 미로 경로 탐색: A* 알고리즘 사용)
 * 미로게임 구현
 1. 게임 모드에 따라 미로의 크기가 정해지며 미로는 게임을 시작할 때마다 임의로 생성된다.
    (easy mode: 30x30, normal mode: 50x50, hard mode: 70x70)
	게임이 시작되면 플레이어는 방향키를 이동하여 미로를 이동하며, 중간 경로에 있는 별들을 먹어 star bonus 점수를 얻는다.
	도착지점에 도착하면 게임이 끝나고, easy mode는 2분, normal mode는 5분, hard mode는 7분의 시간에서 게임 플레이시간을 뺀 time bonus 점수를 얻는다.
	게임이 끝난 후 플레이어는 이름을 랭킹에 저장할 수 있다.
2. 가장 최근에 플레이한 게임의 미로의 경로(정답)를 Show Answer 메뉴에서 확인할 수 있다.
3. Show Ranking 메뉴에서 게임 랭킹을 확인할 수 있다.(게임 랭킹 정보는 rank.txt에 저장/ setup 시에 기존 rank.txt 불러옴)
4. exit할 때, 모든 정보의 메모리가 해제되며, 종료 시 랭킹이 rank.txt에 저장됨
*/

#include "ofApp.h"
#include <iostream>
#include <utility>
#include <stack>
#include <queue>
#include <random>
#include <time.h>
#include <string.h>
using namespace std;
//--------------------------------------------------------------
void ofApp::setup() {

	ofSetWindowTitle("Maze Game"); // Set the app name on the title bar
	ofSetFrameRate(30); // 초당 프레임
	ofBackground(255, 255, 255);
	// Get the window size for image loading
	windowWidth = ofGetWidth();
	windowHeight = ofGetHeight();
	// Set the size of window, Centre on the screen
	ofSetWindowShape(1300, 800);
	ofSetWindowPosition((ofGetScreenWidth()-windowWidth)/3, (ofGetScreenHeight()-windowHeight)/3);

	// Load a font rather than the default
	myFont.loadFont("verdana.ttf", 17, true, true);

	// Window handle used for topmost function
	hWnd = WindowFromDC(wglGetCurrentDC());

	// Disable escape key exit so we can exit fullscreen with Escape (see keyPressed)
	ofSetEscapeQuitsApp(false);

	//
	// Create a menu using ofxWinMenu
	//

	// A new menu object with a pointer to this class
	menu = new ofxWinMenu(this, hWnd);

	// Register an ofApp function that is called when a menu item is selected.
	// The function can be called anything but must exist. 
	// See the example "appMenuFunction".
	menu->CreateMenuFunction(&ofApp::appMenuFunction);

	// Create a window menu
	HMENU hMenu = menu->CreateWindowMenu();

	//
	// Create a "Start" popup menu -> 게임 시작
	//
	HMENU hPopup = menu->AddPopupMenu(hMenu, "Start");

	//
	// Add popup items to the File menu
	//

	// easy, normal, hard mode 게임모드 선택 메뉴
	menu->AddPopupItem(hPopup, "Easy", false, false); // Not checked and not auto-checked
	menu->AddPopupItem(hPopup, "Normal", false, false); // Not checked and not auto-checked
	menu->AddPopupItem(hPopup, "Hard", false, false); // Not checked and not auto-checked
	
	// Final File popup menu item is "Exit" - add a separator before it
	menu->AddPopupSeparator(hPopup);
	menu->AddPopupItem(hPopup, "Exit", false, false);

	//
	// View popup menu
	//
	hPopup = menu->AddPopupMenu(hMenu, "View");

	// 정답 확인 메뉴
	menu->AddPopupItem(hPopup, "Show Answer",false,false); // Checked
	// 랭킹 확인 메뉴
	menu->AddPopupItem(hPopup, "Show Ranking", false, false); // Checked
	bTopmost = false; // app is topmost
	bFullscreen = false; // not fullscreen yet
	// 전체화면 메뉴
	menu->AddPopupItem(hPopup, "Full screen", false, false); // Not checked and not auto-check

	// Set the menu to the window
	menu->SetWindowMenu();

	// 초기화면 플래그 1로 설정
	isFirstpage = 1;

	// 트리 초기화
	ranktree = NULL;
	readFile(); // 기존 랭킹 정보 파일 있다면 랭킹 불러오기

} // end Setup


//
// Menu function
//
// This function is called by ofxWinMenu when an item is selected.
// The the title and state can be checked for required action.
// 
void ofApp::appMenuFunction(string title, bool bChecked) {

	ofFileDialogResult result;
	string filePath;
	size_t pos;

	//
	// Game menu - easy, normal, hard mode + exit func
	//
	if(title == "Easy") {
		if (isMaze) {
			freeMemory();
		}
		isFirstpage = 0; // 첫 페이지 아님
		isPath = 0; // 답 보여주지 x
		isRanking = 0; // ranking 보여주지 x
		// easy mode 기본 설정
		WIDTH = HEIGHT = 15;
		target_x = target_y = 14;
		WALL = 30; STARS = 5; TIME = 120;
		mazeGenerate(); // easy mode 미로 생성
		isGame = 1; // 게임 시작
		time(&USER.startTime); // 현재 플레이어 게임 시작 시간 set
	}
	if (title == "Normal") {
		if (isMaze) {
			freeMemory();
		}
		isFirstpage = 0; // 첫 페이지 아님
		isPath = 0; // 답 보여주지 x
		isRanking = 0; // ranking 보여주지 x
		// normal mode 기본 설정
		WIDTH = HEIGHT = 20;
		target_x = target_y = 19;
		WALL = 25; STARS = 10; TIME = 300;
		mazeGenerate(); // normal mode 미로 생성
		isGame = 1; // 게임 시작
		time(&USER.startTime); // 현재 플레이어 게임 시작 시간 set
	}
	if (title == "Hard") {
		if (isMaze) {
			freeMemory();
		}
		isFirstpage = 0; // 첫 페이지 아님
		isPath = 0; // 답 보여주지 x
		isRanking = 0; // ranking 보여주지 x
		// hard mode 기본 설정
		WIDTH = HEIGHT = 25;
		target_x = target_y = 24;
		WALL = 20; STARS = 20; TIME = 420;
		mazeGenerate(); // hard mode 미로 생성
		isGame = 1; // 게임 시작
		time(&USER.startTime); // 현재 플레이어 게임 시작 시간 set
	}
	if(title == "Exit") {
		make_rankfile();
		isExit = 1;
		freeMemory();
		ofExit(); // Quit the application
	}

	//
	// view menu -> show answer, show ranking, full screen menu
	//
	if(title == "Show Answer") {
		isGame = 0; // 게임 중 아님
		isRanking = 0; // ranking 보여주지 x
		if (isMaze)
		{	
			isFirstpage = 0; // 첫 페이지 아님
			isPath = findPath(); // 미로 경로 탐색
		}
		else
			cout << "you must open file first" << endl;
		
	}

	if (title == "Show Ranking") {
		isGame = 0; // 게임 중 아님
		isFirstpage = 0; // 첫 페이지 아님
		isPath = 0; // 답 보여주는 것 아님
		isRanking = 1; // 랭킹 표시
	}

	if(title == "Full screen") {
		bFullscreen = !bFullscreen; // Not auto-checked and also used in the keyPressed function
		doFullScreen(bFullscreen); // But als take action immediately
	}

} // end appMenuFunction


//--------------------------------------------------------------
void ofApp::update() {

}


//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(100);
	ofSetLineWidth(5);
	
	// 첫 페이지를 보여줌 -> game description 포함
	if (isFirstpage) {
		drawFirstpage();
	}

	// 게임 중이거나 정답을 보여줄 때는 미로를 그림
	if (isGame || isPath) {
		mazedraw();
	}

	// 정답 경로를 그림
	if (isPath)
	{
		ofSetColor(200);
		ofSetLineWidth(5);
		if (isMaze)
			pathdraw();
		else
			cout << "You must open file first" << endl;
	}

	// 게임 중 게임 캐릭터, 플레이어의 경로를 그림
	if (isGame) {
		drawcharacter();
	}

	// 랭킹 보여줌
	if (isRanking) {
		drawranking();
	}
	
} // end Draw

/*
  func name: drawFirstpage
  처음 게임을 실행했을 때와, 모든 게임 기능이 종료된 후 표시되는 화면을 그리는 함수로,
  maze game 프로그램에 대한 간략한 설명을 보여준다.
  실행조건: isFirstpage = 1
*/
void ofApp::drawFirstpage()
{
	char str[150];
	char str2[150];
	char str3[100];

	ofSetColor(0, 0, 0);
	sprintf(str, "1. If you want to start a game,\n please select one of the game modes you want - easy, normal, hard - from Start menu.");
	sprintf(str2, "2. If you want to check the answer of the recent maze you selected,\n please select Show Answer menu.");
	sprintf(str3, "3. Once you finish the game, check your score and input your name to record the score.");
	myFont.drawString(str, 10, 50);
	myFont.drawString(str2, 10, 140);
	myFont.drawString(str3, 10, 230);
}

/*
  func name: drawStar
  획득하면 star bonus 점수를 얻는 star 개체를 미로의 위치 정보를 받아 알맞게 별을 그려주는 함수
  mazedraw 함수에서 사용됨
*/
void ofApp::drawStar(int x, int y)
{	
	int ox, oy;
	ox = (WALL - 18.5) / 2; oy = (WALL - 17) / 2;
	ofSetHexColor(0xb5de10);
	ofSetPolyMode(OF_POLY_WINDING_NONZERO);
	ofBeginShape();
	ofVertex(ox+x*WALL+18.5, oy+y*WALL+11);
	ofVertex(ox+x*WALL, oy+y*WALL +11);
	ofVertex(ox+x*WALL+15, oy+y*WALL);
	ofVertex(ox+x*WALL+9, oy+y*WALL+17.5);
	ofVertex(ox+x*WALL+3.5, oy+y*WALL);
	ofEndShape();
}

// oF 윈도우 창을 최대화면으로 보여줌
void ofApp::doFullScreen(bool bFull)
{
	// Enter full screen
	if(bFull) {
		// Remove the menu but don't destroy it
		menu->RemoveWindowMenu();
		// hide the cursor
		ofHideCursor();
		// Set full screen
		ofSetFullscreen(true);
	}
	else { 
		// return from full screen
		ofSetFullscreen(false);
		// Restore the menu
		menu->SetWindowMenu();
		// Restore the window size allowing for the menu
		ofSetWindowShape(windowWidth, windowHeight + GetSystemMetrics(SM_CYMENU)); 
		// Centre on the screen
		ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
		// Show the cursor again
		ofShowCursor();
		// Restore topmost state
		if(bTopmost) doTopmost(true);
	}

} // end doFullScreen

// 윈도우 창을 맨 위로 보여 줌
void ofApp::doTopmost(bool bTop)
{
	if(bTop) {
		// get the current top window for return
		hWndForeground = GetForegroundWindow();
		// Set this window topmost
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		ShowWindow(hWnd, SW_SHOW);
	}
	else {
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(hWnd, SW_SHOW);
		// Reset the window that was topmost before
		if(GetWindowLong(hWndForeground, GWL_EXSTYLE) & WS_EX_TOPMOST)
			SetWindowPos(hWndForeground, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		else
			SetWindowPos(hWndForeground, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
	}
} // end doTopmost

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	
	// Escape key exit has been disabled but it can be checked here
	if(key == VK_ESCAPE) {
		// Disable fullscreen set, otherwise quit the application as usual
		if(bFullscreen) {
			bFullscreen = false;
			doFullScreen(false);
		}
		else {
			ofExit();
		}
	}

	if(key == 'f') {
		bFullscreen = !bFullscreen;	
		doFullScreen(bFullscreen);
		// Do not check this menu item
		// If there is no menu when you call the SetPopupItem function it will crash
	}

} // end keyPressed

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	// isGame = 1이면 게임이 실행된다고 판단, 이 때에는 방향키 입력을 받음
	if (isGame) {
		playgame(key);
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
/* set up시 실행되며, 만약 기존에 rank.txt 파일이 bin/data 디렉토리에 있다면 랭킹을 bst로 불러온다. */
bool ofApp::readFile()
{	
	ofFile file("rank.txt");
	// 랭킹 파일이 존재한다면
	if(file.exists()) {
		ofBuffer buffer(file);
		char name[21]; int scores;

		// 점수 받아서 bst에 넣기
		for (auto line : buffer.getLines()) {
			char* str = new char[line.size() + 1];
			copy(line.begin(), line.end(), str);
			str[line.size()] = '\0'; 
			sscanf(str, "%s %d", name, &scores);
			insert_bst(name, scores);
			delete[] str;
		}
	}
	return true;
}

//-----------------------------------------user defined--------------------------------------------------

/*
  func name: freeMemory
  프로그램 실행 중 할당한 메모리들을 모두 해제시킴
  만약 isExit가 set된 경우 (즉 exit 메뉴가 선택된 경우) 랭킹 정보 담는 binary search tree까지 메모리 해제
  (exit 메뉴 아닐 시 maze, user 정보만 초기화 및 메모리 해제)
*/
void ofApp::freeMemory() {
	// maze 정보 초기화
	vector<node*>().swap(path_g);
	vector<pair<int,int>>().swap(maze_path);
	vector<pair<int, int>>().swap(stars_pos); // 별들 위치 저장
	this->HEIGHT = 0;
	this->WIDTH = 0;
	// USER 정보 초기화
	USER.pos.first = 0; USER.pos.second = 0;
	USER.timescore = 0;
	USER.starscore = 0;
	USER.totalscore = 0;
	USER.startTime = 0;
	USER.name.clear();
	// isExit일경우에 bst 메모리 해제
	if (isExit) {
		stack<bstnode*> s;
		bstnode *current, *del;
		current = ranktree->lchild;
		for (;;) {
			for (; current; current = current->lchild)
				s.push(current);
			if (s.empty()) break;
			current = s.top(); s.pop();
			del = current;
			current = current->rchild;
			delete del;
		}
		ranktree = NULL;
	}
}

// ------------------------------------------ 미로 생성 기능 --------------------------------------------

/*
  func name: mazeGenerate
  미로를 생성해주는 함수로, 게임 모드에 따라 크기가 다르다. 게임을 시작하는 모든 경우에 임의로 생성된다.
  recursive backtracking 알고리즘을 사용하며, 미로의 위치 정보로 미로의 경로가 그래프에 저장된다.
  추가적으로 게임에 사용될 별들의 위치를 랜덤으로 정한다.
*/
void ofApp::mazeGenerate()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> ranw(0, WIDTH - 1);
	uniform_int_distribution<int> ranh(0, HEIGHT - 1);
	uniform_int_distribution<int> rand(0, 3);

	int x, y, nx, ny, d, dcheck; //현재 위치, 다음 위치, 현재 방향
	vector<vector<bool>> selected(HEIGHT, vector<bool>(WIDTH, false));
	bool dvalid[4]; // 방향 유효성 나타내는 bool 배열
	stack <pair<int, int>> s; // 함수 local stack -> recursion 피함
	pair <int, int> temp;

	create_graph(); // 빈 그래프 생성

	x = ranw(gen); y = ranh(gen);
	temp.first = x; temp.second = y; s.push(temp);
	selected[y][x] = true; // 랜덤으로 처음 위치 구함 -> 스택에 push
	while (s.empty() == false) {
		temp = s.top(); s.pop(); // 스택에서 제일 처음거 꺼냄
		x = temp.first; y = temp.second; // x, y 정보 갱신
		d = 0; dcheck = 0;// 방향 정보 초기화
		for (int dir = 0; dir < 4; dir++) dvalid[dir] = true;
		while (dcheck < 4) {
			d = rand(gen);
			if (dvalid[d] == false) continue; // 이 방향 탐색했으면 continue
			else {
				dvalid[d] = false; dcheck++;
			}
			if (!check_move(x, y, d)) continue; // 이동할 수 없는 위치이면 다음 방향으로
			nx = process_d(x, y, d).first; ny = process_d(x, y, d).second;
			if (selected[ny][nx] == false) {
				selected[ny][nx] = true;
				add_edge(x, y, d);
				temp.first = x; temp.second = y;
				s.push(temp);
				x = nx; y = ny; dcheck = 0; // 다음 위치로 이동
				for (int dir = 0; dir < 4; dir++) dvalid[dir] = true;
			}

		}
	}
	// 각 게임 모드에 맞는 star의 개수만큼 랜덤으로 star 배치
	for (int i = 0; i < STARS; i++) {
		temp.first = ranw(gen);
		temp.second = ranh(gen);
		stars_pos.push_back(temp);
	}
	isMaze = 1;
}

/*
  func name: check_move
  미로 생성 함수 generateMaze의 내부 함수로, 현재 위치와 방향 d를 전달하면
  현재 위치에서 d 방향으로 움직이는 게 가능한지 여부를 반환한다.
*/
bool ofApp::check_move(int x, int y, int d)
{
	if (x <= 0 && d == 3) return false; // 왼쪽일때
	else if (x >= WIDTH - 1 && d == 1) return false; //오른쪽일때
	else if (y <= 0 && d == 0) return false; // 위일때
	else if (y >= HEIGHT - 1 && d == 2) return false; // 아래일때
	else return true;
}

/* 
  func name: process_d
  미로 생성 함수 generateMaze의 내부 함수로, 현재 위치와 방향 d를 전달하면
  현재 위치에서 d 방향으로 움직였을 때의 위치를 pair 구조체로 반환한다.
*/
pair<int, int> ofApp::process_d(int x, int y, int d)
{
	pair<int, int> temp;
	switch (d) {
	case 0: temp.first = x; temp.second = y - 1; // 위
		break;
	case 1: temp.first = x + 1; temp.second = y; // 오른쪽
		break;
	case 2: temp.first = x; temp.second = y + 1; // 아래
		break;
	case 3: temp.first = x - 1; temp.second = y; // 왼쪽
		break;
	}
	return temp;
}

/*
  func name: create_graph
  미로 생성 함수 generateMaze의 내부 함수로, 미로의 크기에 맞게 빈 그래프를 생성한다.
  이후에 간선이 추가될 이 그래프는 미로의 가능한 모든 경로를 저장한다.
  그래프는 1차원이며 row major 순서로 미로의 각 위치가 배열의 인덱스마다 한 칸씩 저장되어 있다.
*/
void ofApp::create_graph()
{
	// 그래프 정보 초기화
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			node* tempn = new node;  tempn->x = j; tempn->y = i; tempn->link = NULL;
			this->path_g.push_back(tempn);
		}
	}
}

/*
  func name: add_edge
  미로 생성 함수 generateMaze의 내부 함수로, 미로의 위치와 방향을 전달하면
  그 위치로부터 해당 방향으로 한 칸 나아가는 미로의 경로를 간선으로 표현하여 그래프에 저장한다.
*/
void ofApp::add_edge(int x, int y, int d)
{
	node* newnode = new node;
	node* newnode2 = new node;
	// 도착지점 위치 찾기, 출발지점에 새 노드 추가
	newnode->x = process_d(x, y, d).first;
	newnode->y = process_d(x, y, d).second;
	newnode->link = path_g[y*WIDTH + x]->link;
	path_g[y*WIDTH + x]->link = newnode;
	// 도착지점에도 새 노드 추가
	newnode2->x = x;
	newnode2->y = y;
	newnode2->link = path_g[process_d(x, y, d).second*WIDTH + process_d(x, y, d).first]->link;
	path_g[process_d(x, y, d).second*WIDTH + process_d(x, y, d).first]->link = newnode2;
}

/*
  func name: mazedraw
  미로 생성 함수 generateMaze의 결과로 path_g에 저장된 미로 경로 정보를 바탕으로 미로를 그린다.
*/
void ofApp::mazedraw()
{
	bool d[4]; // 4 방향으로 갈 수 있는지 저장하는 배열(0:위, 1:오른쪽, 2:아래, 3:왼쪽) 
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			node *temp, *temp2;
			temp = temp2 = path_g[i*WIDTH + j];
			// 방향 정보 초기화
			for (int k = 0; k < 4; k++) d[k] = false;
			for (temp2 = temp2->link; temp2; temp2 = temp2->link) {
				if (temp2->x - temp->x == -1) d[3] = true;
				else if (temp2->x - temp->x == 1) d[1] = true;
				else if (temp2->y - temp->y == -1) d[0] = true;
				else if (temp2->y - temp->y == 1) d[2] = true;
			}
			// 해당 방향 길 없거나, 테두리일 때 벽 그림
			if (d[0] == false || i == 0) ofDrawLine(j * WALL, i * WALL, (j + 1) * WALL, i * WALL);
			if (d[1] == false || j == WIDTH - 1) ofDrawLine((j + 1) * WALL, i * WALL, (j + 1) * WALL, (i + 1) * WALL);
			if (d[2] == false || i == HEIGHT - 1) ofDrawLine(j * WALL, (i + 1) * WALL, (j + 1) * WALL, (i + 1) * WALL);
			if (d[3] == false || j == 0) ofDrawLine(j * WALL, i * WALL, j * WALL, (i + 1) * WALL);
		}
	}
}

//--------------------------------------------------미로 경로 찾기 함수----------------------------------------------

/*
  func name: findPath
  미로를 탐색하여 시작 위치부터 미로의 출구까지의 경로를 보여주는 함수이다. 미로탐색 알고리즘으로는 A* 알고리즘이 사용
  dfs방식으로 미로 경로 탐색하되 도착지점에서 가까운 방향이로 이동
*/
bool ofApp::findPath()
{
	stack<pair<int, int>> S;
	vector<vector<int>> visited(HEIGHT, vector<int>(WIDTH, 0));
	int x, y, mindis;
	pair <int, int> mind; // 각 방향으로 갔을 때 도착지점과의 거리
	pair <int, int> temp;
	node* tn;
	x = 0; y = 0;
	temp.first = x; temp.second = y;
	S.push(temp);
	visited[0][0] = 1; // 시작 위치 스택에 저장
	while (S.empty() == false) {
		mind.first = -1; mind.second = -1;
		mindis = -1; // 최소 거리, 방향 초기화
		tn = path_g[y*WIDTH + x]->link;
		for (; tn; tn = tn->link) {
			if (visited[tn->y][tn->x] == 0) {
				if (tn->x == target_x && tn->y == target_y) {
					// 미로 도착지점 찾음
					mindis = 0;
					mind.first = tn->x; mind.second = tn->y;
					S.push(mind);
					break;
				}
				else if (mindis == -1 || (target_y - tn->y + target_x - tn->x < mindis)) {
					// 도착지점에 제일 가까운 방향 찾기
					mindis = target_y - tn->y + target_x - tn->x;
					mind.first = tn->x; mind.second = tn->y;
				}
			}
		}
		if (mindis == 0) {
			// 길 찾았음 -> 현재 스택에 저장된 길들 vector에 저장
			while (S.empty() == false) {
				temp.first = S.top().first; temp.second = S.top().second;
				maze_path.push_back(temp);
				S.pop();
			}
			break; // while loop 탈출
		}
		else if (mindis == -1) {
			// 갈 곳 없음 -> 스택 pop
			S.pop();
			x = S.top().first; y = S.top().second; // 위치 갱신
		}
		else {
			// 다음 위치로 가기
			x = mind.first; y = mind.second; // 위치 갱신
			visited[y][x] = 1;
			temp.first = x; temp.second = y;
			S.push(temp);
		}
	}
	return true;
}

/*
  func name: pathdaraw
  isPath 플래그가 1일 때 수행되며,
  미로 탐색 함수 findpath의 결과로 저장된 maze_path안의 미로 좌표들을 바탕으로 미로 경로, 즉 정답을 그린다.
  실행 조건: isPath
*/
void ofApp::pathdraw()
{
	ofSetColor(255, 0, 0);
	int ox = WALL/2, oy = WALL/2; // 원래 위치
	int x1, y1, x2, y2;
	for (int i = 0; i < maze_path.size() - 1; i++) {
		x1 = maze_path[i].first; y1 = maze_path[i].second;
		x2 = maze_path[i + 1].first; y2 = maze_path[i + 1].second;
		ofDrawLine(ox + x1 * WALL, oy + y1 * WALL, ox + x2 * WALL, oy + y2 * WALL);
	}
}

//----------------------- 게임 실행 함수 -----------------------------------

/*
  func name: playgame
  플레이어가 현재 만들어진 미로를 가지고 미로 찾기 게임을 할 때 실행되는 함수
  keyReleased에서 실행되며 방향키 입력을 받아 플레이어 위치 이동시킴, 실제적인 게임 기능 구현하는 함수
  initial state: User 정보는 초기화 되었다고 가정
  실행조건: isGame = 1
*/
void ofApp::playgame(int key)
{	
	char str[200];
	char tempname[21];

	if (key == OF_KEY_LEFT) {
		if(play_check(USER.pos.first, USER.pos.second, USER.pos.first-1, USER.pos.second))
		// 왼쪽으로 한 칸 이동
		USER.pos.first--;
	}

	if (key == OF_KEY_RIGHT) {
		if (play_check(USER.pos.first, USER.pos.second, USER.pos.first+1, USER.pos.second))
		// 오른쪽으로 한 칸 이동
		USER.pos.first++;
	}

	if (key == OF_KEY_UP) {
		if (play_check(USER.pos.first, USER.pos.second, USER.pos.first, USER.pos.second - 1))
		// 위로 한 칸 이동
		USER.pos.second--;
	}

	if (key == OF_KEY_DOWN) {
		if (play_check(USER.pos.first, USER.pos.second, USER.pos.first, USER.pos.second + 1))
		// 아래로 한 칸 이동
		USER.pos.second++;
	}

	// 경로에 추가
	pair<int, int> temp;
	temp.first = USER.pos.first; temp.second = USER.pos.second;

	// star 먹었을 경우
	for (int i = 0; i < STARS; i++) {
		if (USER.pos.first == stars_pos[i].first && USER.pos.second == stars_pos[i].second) {
			USER.starscore += 100;
			stars_pos[i].first = -1;
		}
	}

	// 만약 탈출했다면
	if (USER.pos.first == target_x && USER.pos.second == target_y) {
		// 점수 계산 및 출력
		calc_score();
		sprintf(str, "Your Score: %d(Time Bonus Score: %d + Star Bonus Score: %d)\nInput your name to record score.", USER.totalscore, USER.timescore, USER.starscore);
		ofSystemAlertDialog(str);
		// 플레이어 이름 입력 (default: Player)
		USER.name = ofSystemTextBoxDialog("Input your name", "Player");
		// 트리에 저장
		char* str = new char[USER.name.size() + 1];
		copy(USER.name.begin(), USER.name.end(), str);
		str[USER.name.size()] = '\0';
		insert_bst(str, USER.totalscore);
		// 다시 초기 화면으로
		isGame = 0;
		isFirstpage = 1;
	}
}

/*
  func name: play_check
  x1, y1로부터 x2, y2로의 이동이 현재 미로안에서 가능한지 알려주는 함수
*/
bool ofApp::play_check(int x1, int y1, int x2, int y2)
{	
	node* cur = path_g[y1*WIDTH + x1];
	for (cur = cur->link; cur; cur = cur->link) {
		if (cur->x == x2 && cur->y == y2) return true;
	}
	return false;
}

/*
  func name: drawcharacter
  현재 플레이어 정보를 가지고 캐릭터 아이콘과 이전까지의 경로를 보여줌, stars들도 그려줌
  실행 조건: isGame = 1
*/
void ofApp::drawcharacter()
{	
	// 캐릭터 그리기
	int ox = WALL / 2, oy = WALL / 2;
	ofSetHexColor(0xe4bbfe);
	ofDrawCircle(ox+USER.pos.first*WALL, oy+USER.pos.second*WALL, WALL/3);

	// 별그리기
	for (int i = 0; i < STARS; i++) {
		if (stars_pos[i].first != -1) {
			drawStar(stars_pos[i].first, stars_pos[i].second);
		}
	}

	// 지금까지 경로 그리기
	
}

/*
  func name: calc_score
  현재 플레이어의 star bonus 점수 정보와, 게임 실행 시간을 가지고 계산한 time bonus 점수를 합산한 플레이어의 total score를 구한다.
  time bonus score = 각 게임모드에 따른 제한시간(초) – 플레이 시간(초) * 100
  star bonus score = 획득한 star 개수 *100
  total score = time bonus score + star bonus score
  실행 조건: isGame = 1 & 게임이 완료됨
*/
void ofApp::calc_score()
{	
	time_t cur_t; time(&cur_t);
	int dur = cur_t - USER.startTime;
	USER.timescore = (TIME - dur) * 100;
	USER.totalscore = USER.timescore + USER.starscore;
}

// -------------------------------------- 랭킹 기능 관련 함수 ---------------------------------

/*
  func name: insert_bst
  랭킹 정보를 담는 binary search tree ranktree에 주어진 정보를 갖는 랭킹 노드를 생성하여 삽입한다.
*/
void ofApp::insert_bst(char* n, int s)
{
	bstnode *temp, *cur;
	temp = cur = ranktree;
	// cur에 삽입할 위치 찾기
	while (temp) {
		if (s == temp->score) {
			cur = temp; break;
		}
		else if (s < temp->score) {
			cur = temp;
			temp = temp->lchild;
		}
		else {
			cur = temp;
			temp = temp->rchild;
		}
	}
	// 해당 위치에 삽입
	temp = new bstnode;
	strcpy(temp->name, n); temp->score = s;
	temp->lchild = temp->rchild = NULL;
	if (cur) {
		if (s < cur->score) cur->lchild = temp;
		else cur->rchild = temp;
	}
	else ranktree = temp;
}

/*
  func name: make_rankfile
  exit 메뉴 선택시 프로그램이 종료되기 전에 현재 랭킹 binary search tree의 랭킹 정보를 rank.txt에 저장한다.
  실행 조건: exit 메뉴 실행시
*/
void ofApp::make_rankfile()
{
	rankfile.open("rank.txt", ofFile::WriteOnly);
	
	// 내림차순의 순서로 bst 파일에 출력
	stack<bstnode*> S;
	bstnode* current = ranktree;
	for (;;) {
		for (; current; current = current->rchild)
			S.push(current);
		if (S.empty()) break;
		current = S.top(); S.pop();
		rankfile << current->name << " " << current->score << endl;
		current = current->lchild;
	}
}

/*
  func name: drawranking
  Show Ranking 메뉴를 선택했을 때 현재 랭킹 binary search tree에 담긴 랭킹 정보를 내림차순으로 윈도우 창에 출력한다.
  실행조건: isRanking = 1
*/
void ofApp::drawranking()
{
	vector<char*> ranks;
	// 내림차순의 순서로 bst 화면에 출력
	stack<bstnode*> S;
	bstnode* current = ranktree;
	char str[100];
	int i = 0;
	ofSetColor(0, 0, 0); // 검은색으로 글씨 색상 설정
	while(1) {
		for (; current; current = current->rchild)
			S.push(current);
		if (S.empty()) break;
		current = S.top(); S.pop();
		ranks.push_back(str);
		sprintf(ranks[i], "%d. name: %s/ score: %d", i + 1, current->name, current->score);
		myFont.drawString(ranks[i], 10, 40 * (i + 1));
		current = current->lchild;
		i++;
	}
	//벡터 삭제
	vector<char*>().swap(ranks);
}



